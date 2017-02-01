#ifndef _FRAME_SHUFFLE_UTILS_H_
#define _FRAME_SHUFFLE_UTILS_H_

#include <cstdint>
#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "traits.hpp"



#ifdef _OPENMP
#include "omp.h"
typedef typename std::make_signed<std::size_t>::type omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#else
typedef std::size_t omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#endif

namespace sqeazy {

  namespace detail {

    namespace bacc = boost::accumulators;

	struct frame_shuffle {

	  std::size_t frame_chunk_size;
      std::vector<std::size_t> decode_map;

	  frame_shuffle(std::size_t _fsize = 1,
					std::vector<std::size_t> _map = std::vector<std::size_t>()):
		frame_chunk_size(_fsize),
		decode_map(_map)
      {

      }

      template <typename value_t>
      std::vector<value_t> remainder(const std::vector<value_t>& _shape) const {
		std::vector<value_t> rem = _shape;
		std::fill(rem.begin(),rem.end(),0);
		rem[sqeazy::row_major::z] = _shape[sqeazy::row_major::z] % frame_chunk_size;
		return rem;
      }

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t encode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int nthreads = 1)  {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		// typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		// typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::frame_shuffle::encode] frame_shuffle received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::frame_shuffle::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = std::distance(_begin,_end);
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::frame_shuffle::encode] input range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::accumulate(rem.begin(), rem.end(),0) > 0;

		if(has_remainder)
		  return encode_with_remainder(_begin,_end,_out,_shape,nthreads);
		else{
		  return encode_full(_begin,_end,_out,_shape,nthreads);
		}

      }

      /**
		 \brief implementation where the input stack is assumed to yield only full frames

		 \param[in]

		 \return
		 \retval

      */
      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t encode_full(in_iterator_t _begin,
								 in_iterator_t _end,
								 out_iterator_t _out,
								 const shape_container_t& _shape,
								 int nthreads=  1) {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;
		typedef typename bacc::accumulator_set<in_value_t,
											   bacc::stats<bacc::tag::median>
											   > median_acc_t ;
		// 75% quanframe?
		// typedef typename boost::accumulators::accumulator_set<double, stats<boost::accumulators::tag::pot_quanframe<boost::right>(.75)> > quanframe_acc_t;

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_frame = _shape[row_major::y]*_shape[row_major::x];
		const std::size_t n_elements_per_frame_chunk = n_elements_per_frame*frame_chunk_size;

		shape_container_t n_full_frames = _shape;
		std::fill(n_full_frames.begin(), n_full_frames.end(),1);
		n_full_frames[row_major::z] = _shape[row_major::z] / frame_chunk_size;

		const omp_size_type n_chunks = std::accumulate(n_full_frames.begin(), n_full_frames.end(),1,std::multiplies<shape_value_t>());

		// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
		// median plus stddev around median or take 75% quanframe directly

		std::vector<float> metric(n_chunks,0.f);
		auto metric_itr = metric.data();

#pragma omp parallel for    \
  shared(metric_itr)		\
  firstprivate(_begin)		\
  num_threads(nthreads)
		for(omp_size_type i = 0;i<n_chunks;++i){

		  median_acc_t acc;
		  auto voxel_itr = _begin + i*n_elements_per_frame_chunk;

		  for(std::size_t p = 0;p<n_elements_per_frame_chunk;++p){
			acc(*voxel_itr++);
		  }

		  *(metric_itr + i) = bacc::median(acc);
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(n_chunks);

		auto sorted_metric = metric;

		//TODO: we need a parallel sort here!
		//TODO: what if there are frame_chunks with the same value? (highly unlikely with float32_t)
		std::sort(sorted_metric.begin(), sorted_metric.end());

		auto decode_map_itr = decode_map.data();
		auto metric_begin = metric.data();
		auto metric_end = metric_begin + metric.size();
		auto sorted_metric_begin = sorted_metric.data();
		const omp_size_type loop_count = metric.size();

#pragma omp parallel for								\
  shared(_out)							\
  firstprivate(decode_map_itr,metric_begin,metric_end,sorted_metric_begin,_begin) \
  num_threads(nthreads)
		for(omp_size_type i =0;i<loop_count;++i){
		  auto dst = _out + i*n_elements_per_frame_chunk;

		  auto item = *(sorted_metric_begin + i);
		  auto unsorted_itr = std::find(metric_begin, metric_end,
										item);
		  std::size_t original_index = std::distance(metric_begin,unsorted_itr);
		  *(decode_map_itr + i) = original_index;
		  auto src_start = _begin + original_index*n_elements_per_frame_chunk;

		  std::copy(src_start,
					src_start+n_elements_per_frame_chunk,
					dst);
		}


		return _out + metric.size()*n_elements_per_frame_chunk;

      }


      /**
		 \brief encode with the stack dimensions not fitting the frame shape in any way

		 \param[in]

		 \return
		 \retval

      */
	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int nthreads = 1) {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;
		typedef typename bacc::accumulator_set<in_value_t,
											   bacc::stats<bacc::tag::median>
											   > median_acc_t ;
		// 75% quanframe?
		// typedef typename boost::accumulators::accumulator_set<double, stats<boost::accumulators::tag::pot_quanframe<boost::right>(.75)> > quanframe_acc_t;

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_frame = _shape[row_major::y]*_shape[row_major::x];
		const std::size_t n_elements_per_frame_chunk = n_elements_per_frame*frame_chunk_size;

		shape_container_t n_full_frames = _shape;
		std::fill(n_full_frames.begin(), n_full_frames.end(),1);
		n_full_frames[row_major::z] = _shape[row_major::z] / frame_chunk_size;

		const shape_value_t n_chunks = (_shape[row_major::z] + frame_chunk_size - 1)/frame_chunk_size;
		// const auto len = std::distance(_begin,_end);

		// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
		// median plus stddev around median or take 75% quanframe directly

		std::vector<float> metric(n_chunks-1,0.);
		auto metric_itr = metric.data();
		const omp_size_type loop_count = metric.size();

#pragma omp parallel for    \
  shared(metric_itr)		\
  firstprivate(_begin)		\
  num_threads(nthreads)
		for(omp_size_type i = 0;i<loop_count;++i){
		  median_acc_t acc;
		  auto voxel_itr = _begin + i*n_elements_per_frame_chunk;

		  for(std::size_t p = 0;p<n_elements_per_frame_chunk;++p){
			acc(*voxel_itr++);
		  }

		  *(metric_itr + i) = bacc::median(acc);
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(metric.size());

		auto sorted_metric = metric;

//TODO: we need a parallel sort here!
		//TODO: what if there are frame_chunks with the same value? (highly unlikely with float32_t)
		std::sort(sorted_metric.begin(), sorted_metric.end());

		auto decode_map_itr = decode_map.data();
		auto metric_begin = metric.data();
		auto metric_end = metric_begin + metric.size();
		auto sorted_metric_begin = sorted_metric.data();


		#pragma omp parallel for															\
		  shared(_out)																		\
		  firstprivate(decode_map_itr,metric_begin,metric_end,sorted_metric_begin,_begin)	\
		  num_threads(nthreads)
		for(omp_size_type i =0;i<loop_count;++i){

		  auto item = *(sorted_metric_begin + i);
		  auto unsorted_itr = std::find(metric_begin, metric_end,
										item);
		  std::size_t original_index = std::distance(metric_begin,unsorted_itr);
		  *(decode_map_itr + i) = original_index;
		  auto src_start = _begin + original_index*n_elements_per_frame_chunk;

		  auto dst = _out + i*n_elements_per_frame_chunk;
		  std::copy(src_start,
					src_start+n_elements_per_frame_chunk,
					dst);
		}

		auto dst = std::copy(_begin+metric.size()*n_elements_per_frame_chunk,
							 _end,
							 _out + metric.size()*n_elements_per_frame_chunk);
		return dst;

	  }

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t decode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int nthreads = 1) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		// typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		// typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::frame_shuffle::encode] frame_shuffle received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::frame_shuffle::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		const std::size_t n_elements = std::distance(_begin,_end);
		const std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::frame_shuffle::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		return decode_with_remainder(_begin, _end, _out,_shape);

      }

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t decode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int nthreads = 1) const {


		// typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		// typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::accumulate(rem.begin(), rem.end(),0) > 0;

		const std::size_t n_elements_per_frame = _shape[row_major::y]*_shape[row_major::x];
		const std::size_t n_elements = std::distance(_begin,_end);

		const std::size_t n_elements_per_frame_chunk = n_elements_per_frame*frame_chunk_size;

		shape_container_t n_full_frames = _shape;
		std::fill(n_full_frames.begin(), n_full_frames.end(),1);
		n_full_frames[row_major::z] = _shape[row_major::z] / frame_chunk_size;

		const shape_value_t n_chunks = (_shape[row_major::z] + frame_chunk_size - 1)/frame_chunk_size;
		// const auto len = std::distance(_begin,_end);

		const omp_size_type loop_count = decode_map.size();
		auto decode_map_itr = decode_map.data();

#pragma omp parallel for												\
		  shared(_out)																		\
  firstprivate(decode_map_itr,_begin) \
		  num_threads(nthreads)
		for(omp_size_type i =0;i<loop_count;++i){
		  auto src = _begin + i*n_elements_per_frame_chunk;
		  auto offset = *(decode_map_itr+i);
		  auto dst = _out + (offset*n_elements_per_frame_chunk);
		  std::copy(src,
					src+n_elements_per_frame_chunk,
					dst);
		}

		auto dst = _out;

		if(has_remainder){
		  dst = std::copy(_begin+decode_map.size()*n_elements_per_frame_chunk,
						  _end,
						  _out+decode_map.size()*n_elements_per_frame_chunk);
		}
		else{
		  dst += decode_map.size()*n_elements_per_frame_chunk;
		}

		return dst;
	  };
	};

  };

};

#endif /* _FRAME_SHUFFLE_UTILS_H_ */
