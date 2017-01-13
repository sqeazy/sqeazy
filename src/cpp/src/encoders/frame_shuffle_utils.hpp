#ifndef _FRAME_SHUFFLE_UTILS_H_
#define _FRAME_SHUFFLE_UTILS_H_

#include <cstdint>
#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "traits.hpp"

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
							const shape_container_t& _shape)  {

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
		  return encode_with_remainder(_begin,_end,_out,_shape);
		else{
		  return encode_full(_begin,_end,_out,_shape);
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
								 const shape_container_t& _shape) {

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

		const shape_value_t n_chunks = std::accumulate(n_full_frames.begin(), n_full_frames.end(),1,std::multiplies<shape_value_t>());

		// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
		// median plus stddev around median or take 75% quanframe directly

		std::vector<in_value_t> metric(n_chunks,0.);
		auto voxel_itr = _begin;
		for(std::size_t i = 0;i<n_chunks;++i){
		  median_acc_t acc;

		  for(std::size_t p = 0;p<n_elements_per_frame_chunk;++p){
			acc(*voxel_itr++);
		  }

		  metric[i] = std::round(bacc::median(acc));
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(n_chunks);

		auto sorted_metric = metric;
		std::sort(sorted_metric.begin(), sorted_metric.end());

		auto dst = _out;
		for(shape_value_t i =0;i<metric.size();++i){
		  auto original_index = std::find(metric.begin(), metric.end(), sorted_metric[i]) - metric.begin();
		  decode_map[i] = original_index;
		  auto src_start = _begin + original_index*n_elements_per_frame_chunk;
		  dst = std::copy(src_start,
						  src_start+n_elements_per_frame_chunk,
						  dst);
		}


		return dst;

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
										   const shape_container_t& _shape) {

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

		std::vector<in_value_t> metric(n_chunks-1,0.);
		auto voxel_itr = _begin;
		for(std::size_t i = 0;i<metric.size();++i){
		  median_acc_t acc;

		  for(std::size_t p = 0;p<n_elements_per_frame_chunk;++p){
			acc(*voxel_itr++);
		  }

		  metric[i] = std::round(bacc::median(acc));
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(metric.size());

		auto sorted_metric = metric;
		std::sort(sorted_metric.begin(), sorted_metric.end());

		auto dst = _out;
		for(shape_value_t i =0;i<metric.size();++i){
		  auto original_index = std::find(metric.begin(), metric.end(), sorted_metric[i]) - metric.begin();
		  decode_map[i] = original_index;
		  auto src_start = _begin + original_index*n_elements_per_frame_chunk;
		  dst = std::copy(src_start,
						  src_start+n_elements_per_frame_chunk,
						  dst);
		}

		dst = std::copy(_begin+metric.size()*n_elements_per_frame_chunk,
						_end,
						dst);
		return dst;

	  }

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t decode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape) const {

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
										   const shape_container_t& _shape) const {


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

		auto dst = _out;
		auto src = _begin;

		for(shape_value_t i =0;i<decode_map.size();++i){

		  dst = _out + (decode_map[i])*n_elements_per_frame_chunk;
		  dst = std::copy(src,
						  src+n_elements_per_frame_chunk,
						  dst);

		  src += n_elements_per_frame_chunk;
		}

		if(has_remainder){
		  dst = std::copy(src,_end,
						  _out+decode_map.size()*n_elements_per_frame_chunk);
		  src = _end;
		}

		if(src == _end)
		  return _out+n_elements;
		else
		  return dst;
	  };
	};

  };

};

#endif /* _FRAME_SHUFFLE_UTILS_H_ */
