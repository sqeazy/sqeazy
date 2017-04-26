#ifndef _TILE_SHUFFLE_UTILS_H_
#define _TILE_SHUFFLE_UTILS_H_

#include <cstdint>
#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "traits.hpp"
#include "sqeazy_algorithms.hpp"

namespace sqeazy {

  namespace detail {

	namespace bacc = boost::accumulators;

	struct tile_shuffle {

	  std::size_t tile_size;
	  std::vector<std::size_t> decode_map;

	  tile_shuffle(std::size_t _tsize,
				   std::vector<std::size_t> _map = std::vector<std::size_t>()):
		tile_size(_tsize),
		decode_map(_map)
		{

		}

	  template <typename value_t>
	  std::vector<value_t> remainder(const std::vector<value_t>& _shape) const {
		std::vector<value_t> rem = _shape;
		for(value_t & el : rem)
		  el = el % tile_size;

		return rem;
	  }

	  /**
	   *  \brief perform a tile shuffle on the input
	   *
	   *  the input stack is partitioned in equisized boxes of dimension tile_size^3. for each tile,
	   *  the median intensity value is computed; all medians are sorted; following the order of the medians,
	   *  the input is rearranged so that small-median tiles are at the beginning of the output array
	   *  large-median tiles are at the end of the output array
	   *
	   *  TODO: parallelize the sort if the compression ratio is enhanced
	   *
	   *  \param param
	   *  \return return type
	   */
	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int _nthreads = 1 )  {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::tile_shuffle::encode] tile_shuffle received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::tile_shuffle::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::tile_shuffle::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		if(has_remainder)
		  return encode_with_remainder(_begin,_end,_out,_shape,_nthreads);
		else{
		  return encode_full(_begin,_end,_out,_shape,_nthreads);
		}

	  }

	  /**
		 \brief implementation where the input stack is assumed to yield only full tiles

		 \param[in]

		 \return
		 \retval

	  */
	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_full(in_iterator_t _begin,
								 in_iterator_t _end,
								 out_iterator_t _out,
								 const shape_container_t& _shape,
								 int _nthreads = 1 ) {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;
		typedef typename bacc::accumulator_set<in_value_t,
											   bacc::stats<bacc::tag::median>
											   > median_acc_t ;

		// 75% quantile?
		// typedef typename boost::accumulators::accumulator_set<double, stats<boost::accumulators::tag::pot_quantile<boost::right>(.75)> > quantile_acc_t;

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements          = std::distance(_begin,_end);
		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
		const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);

		shape_container_t n_full_tiles = _shape;

		for(shape_value_t & n_tiles : n_full_tiles )
		  n_tiles = n_tiles / tile_size;

		const shape_value_t len_tiles = std::accumulate(n_full_tiles.begin(), n_full_tiles.end(),1,std::multiplies<shape_value_t>());

		// COLLECT TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////

		std::vector< std::vector<in_value_t> > tiles(len_tiles, std::vector<in_value_t>(n_elements_per_tile,0) );
		auto ptiles = tiles.data();
		auto pshape = _shape.data();
		auto pn_full_tiles = n_full_tiles.data();

#pragma omp parallel for												\
  shared( _begin, ptiles)												\
  firstprivate(pshape, pn_full_tiles)													\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<pshape[row_major::z];++z){
		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile = z % tile_size;

		  for(shape_value_t y = 0;y<pshape[row_major::y];++y){
			std::size_t ytile = y / tile_size;
			std::size_t y_intile = y % tile_size;

			std::size_t linear_tile_id = ztile*pn_full_tiles[sqeazy::row_major::y]*pn_full_tiles[sqeazy::row_major::x]
			  + ytile*pn_full_tiles[sqeazy::row_major::x]
			  ;

			auto acc_iter = _begin + z*pshape[row_major::y]*pshape[row_major::x] + y*pshape[row_major::x];

			for(shape_value_t x = 0;x<pshape[row_major::x];x+=tile_size,++linear_tile_id,acc_iter+=tile_size){
			  // xtile = x / tile_size;

			  auto tiles_iter = tiles[linear_tile_id].begin();

			  std::copy(acc_iter, acc_iter + tile_size,
						tiles_iter + (z_intile*n_elements_per_tile_frame) + y_intile*tile_size);


			}
		  }
		}

		// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
		// median plus stddev around median or take 75% quantile directly

		std::vector<in_value_t> metric(len_tiles,0.);
		auto pmetric = metric.data();

#pragma omp parallel for												\
  shared( pmetric)														\
  firstprivate(ptiles)													\
  num_threads(_nthreads)
		for(std::size_t i = 0;i<len_tiles;++i){
		  median_acc_t acc;

		  for(std::size_t p = 0;p<n_elements_per_tile;++p){
			acc(ptiles[i][p]);
		  }

		  pmetric[i] = std::round(bacc::median(acc));
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(len_tiles);

		auto sorted_metric = metric;
		std::sort(sorted_metric.begin(), sorted_metric.end());

		// WRITE TILES 2 OUTPUT ////////////////////////////////////////////////////////////////////////////////////////////////
		// auto dst = _out;
		auto pdecode_map = decode_map.data();
		auto psorted_metric = sorted_metric.data();
#pragma omp parallel for						\
  shared( pdecode_map, _out)					\
  firstprivate(ptiles,pmetric, psorted_metric)					\
  num_threads(_nthreads)
		for(shape_value_t i =0;i<metric.size();++i){
		  auto original_index = std::find(pmetric, pmetric + len_tiles, psorted_metric[i]) - pmetric;

		  pdecode_map[i] = original_index;
		  std::copy(ptiles[original_index].begin(), ptiles[original_index].end(),
					_out + i*n_elements_per_tile);
		}


		return _out + n_elements;

	  }


	  /**
		 \brief encode with the stack dimensions not fitting the tile shape in any way

		 \param[in]

		 \return
		 \retval

	  */
	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int _nthreads = 1)  {


		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;
		typedef typename bacc::accumulator_set<in_value_t,
											   bacc::stats<bacc::tag::median>
											   > median_acc_t ;
		// 75% quantile?
		// typedef typename boost::accumulators::accumulator_set<double, stats<boost::accumulators::tag::pot_quantile<boost::right>(.75)> > quantile_acc_t;

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
		// const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);

		shape_container_t n_full_tiles = _shape;

		for(shape_value_t & tile_dim : n_full_tiles )
		  tile_dim = tile_dim / tile_size;

		shape_container_t n_tiles = n_full_tiles;
		for(int i =0;i<(int)rem.size();++i)
		  n_tiles[i] += rem[i] ? 1 : 0;

		const shape_value_t len_tiles = std::accumulate(n_tiles.begin(), n_tiles.end(),1,std::multiplies<shape_value_t>());

		// SETUP TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////

		std::vector< std::vector<in_value_t> > tiles(len_tiles, std::vector<in_value_t>(n_elements_per_tile,0));
		auto ptiles = tiles.data();
		auto pshape = _shape.data();
		auto pn_tiles = n_tiles.data();

		// COLLECT TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma omp parallel for												\
  shared( ptiles )														\
  firstprivate(pshape, _begin,pn_tiles)									\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<pshape[row_major::z];++z){
		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile = z % tile_size;
		  std::size_t ztile_shape = (ztile+1)*tile_size > pshape[row_major::z] ? (pshape[row_major::z]-(ztile*tile_size)) : tile_size;

		  for(shape_value_t y = 0;y<pshape[row_major::y];++y){
			std::size_t ytile = y / tile_size;
			std::size_t y_intile = y % tile_size;
			std::size_t ytile_shape = (ytile+1)*tile_size > pshape[row_major::y] ? (pshape[row_major::y]-(ytile*tile_size)) : tile_size;
			std::size_t tile_id = ztile*pn_tiles[sqeazy::row_major::y]*pn_tiles[sqeazy::row_major::x]
			  + ytile*pn_tiles[sqeazy::row_major::x]
			  ;

			auto acc_iter = _begin + z*pshape[row_major::y]*pshape[row_major::x] + y*pshape[row_major::x];

			for(shape_value_t x = 0;x<pshape[row_major::x];x+=tile_size, ++tile_id){
			  std::size_t xtile = x / tile_size;
			  std::size_t xtile_shape = (xtile+1)*tile_size > pshape[row_major::x] ? (pshape[row_major::x]-(xtile*tile_size)) : tile_size;
			  #pragma omp critical
			  {
			  ptiles[tile_id].resize(ztile_shape*ytile_shape*xtile_shape);
			  }

			  auto tiles_iter = ptiles[tile_id].data();
			  auto tiles_offset = (z_intile*xtile_shape*ytile_shape) + y_intile*xtile_shape;

			  std::copy(acc_iter, acc_iter + xtile_shape,
						tiles_iter + tiles_offset);

			  acc_iter+=xtile_shape;

			}
		  }
		}

		// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
		// median plus stddev around median or take 75% quantile directly

		std::vector<in_value_t> metric(len_tiles,0.);
		auto pmetric = metric.data();

#pragma omp parallel for												\
  shared( pmetric)														\
  firstprivate(ptiles)													\
  num_threads(_nthreads)
		for(std::size_t i = 0;i<len_tiles;++i){
		  median_acc_t acc;

		  for(std::size_t p = 0;p<n_elements_per_tile;++p){
			acc(ptiles[i][p]);
		  }

		  pmetric[i] = std::round(bacc::median(acc));
		}

		// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
		decode_map.resize(len_tiles);

		auto sorted_metric = metric;

		//TODO: this sort is the bottleneck currently
		std::sort(sorted_metric.begin(), sorted_metric.end());

		// COMPUTE SHUFFLED-ORIGINAL INDEX MAP //////////////////////////////////////////////////////////////////////////////////
		std::vector<char> tile_written(len_tiles,false);
		auto ptile_written = tile_written.data();
		auto psorted_metric = sorted_metric.data();
		auto pdecode_map = decode_map.data();

#pragma omp parallel for												\
  shared( ptile_written,pdecode_map)										\
  firstprivate(pmetric, psorted_metric)												\
  num_threads(_nthreads)
		for(shape_value_t i =0;i<metric.size();++i){
		  auto original_index = std::find(pmetric, pmetric+len_tiles, psorted_metric[i]) - pmetric;

		  #pragma omp critical
		  {
			while(ptile_written[original_index] == true)
			  original_index++;

			ptile_written[original_index] = true;
		  }


		  pdecode_map[i] = original_index;

		}


		// COMPUTE PREFIX SUM //////////////////////////////////////////////////////////////////////////////////
		std::vector<std::size_t> prefix_sum(len_tiles,0);
		prefix_sum_of(decode_map.begin(), decode_map.end(), prefix_sum.begin(),
					  [&](const std::size_t& _index){ return ptiles[_index].size(); },
					  _nthreads);

		// STORE CONTENT TO OUTPUT //////////////////////////////////////////////////////////////////////////////////
		auto pprefix_sum = prefix_sum.data();
		auto len = std::distance(_begin,_end);
		#pragma omp parallel for												\
		  shared( _out)													\
		  firstprivate(ptiles,pdecode_map,pprefix_sum)								\
		  num_threads(_nthreads)
		for(shape_value_t i =0;i<decode_map.size();++i){

		  std::copy(ptiles[pdecode_map[i]].begin(),
					ptiles[pdecode_map[i]].end(),
					_out + pprefix_sum[i]
			);

		}


		return _out + len;
	  }

	  /**
	   *  \brief decode a tile_shuffled stack into it's original form
	   *
	   *  TODO: parallelize this!
	   *
	   *  \param param
	   *  \return return type
	   */
	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int _nthreads = 1) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::tile_shuffle::encode] tile_shuffle received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::tile_shuffle::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::tile_shuffle::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		// const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		return decode_with_remainder(_begin, _end, _out,_shape);

	  }

	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int _nthreads = 1) const {



		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		// typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		// typedef typename std::remove_cv<out_value_type>::type out_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());

		auto value = _out;
		shape_container_t n_full_tiles = _shape;

		for(shape_value_t & tile_dim : n_full_tiles )
		  tile_dim = tile_dim / tile_size;

		shape_container_t n_tiles = n_full_tiles;
		for(int i =0;i<(int)rem.size();++i)
		  n_tiles[i] += rem[i] ? 1 : 0;

		const shape_value_t len_tiles = std::accumulate(n_tiles.begin(), n_tiles.end(),1,std::multiplies<shape_value_t>());

		// SETUP TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////

		std::vector< std::vector<in_value_t> > tiles(len_tiles, std::vector<in_value_t>(n_elements_per_tile,0));

		std::size_t ztile = 0;
		std::size_t ytile = 0;
		std::size_t xtile = 0;

		std::size_t ztile_shape = 0;
		std::size_t ytile_shape = 0;
		std::size_t xtile_shape = 0;

		std::size_t tile_id = 0;

		for(shape_value_t z = 0;z<_shape[row_major::z];z+=tile_size){
		  ztile = z / tile_size;
		  ztile_shape = (ztile+1)*tile_size > _shape[row_major::z] ? (_shape[row_major::z]-(ztile*tile_size)) : tile_size;

		  for(shape_value_t y = 0;y<_shape[row_major::y];y+=tile_size){
			ytile = y / tile_size;
			ytile_shape = (ytile+1)*tile_size > _shape[row_major::y] ? (_shape[row_major::y]-(ytile*tile_size)) : tile_size;

			tile_id = ztile*n_tiles[sqeazy::row_major::y]*n_tiles[sqeazy::row_major::x]
			  + ytile*n_tiles[sqeazy::row_major::x]
			  ;

			for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size, ++tile_id){
			  xtile = x / tile_size;
			  xtile_shape = (xtile+1)*tile_size > _shape[row_major::x] ? (_shape[row_major::x]-(xtile*tile_size)) : tile_size;

			  tiles[tile_id].resize(ztile_shape*ytile_shape*xtile_shape);
			}
		  }
		}

		auto input = _begin;
		std::size_t target_tile_index = 0;
		for(std::size_t i = 0;i<len_tiles;++i){
		  target_tile_index = decode_map[i];

		  std::copy(input,input+tiles[target_tile_index].size(),
					tiles[target_tile_index].begin());

		  input += tiles[target_tile_index].size();

		}

		// DUMP TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////
		ztile = 0;
		ytile = 0;
		xtile = 0;

		ztile_shape = 0;
		ytile_shape = 0;
		xtile_shape = 0;

		tile_id = 0;


		std::size_t z_intile = 0;
		std::size_t y_intile = 0;


		auto tiles_iter = tiles[0].begin();

		for(shape_value_t z = 0;z<_shape[row_major::z];++z){
		  ztile = z / tile_size;
		  z_intile = z % tile_size;

		  ztile_shape = (ztile+1)*tile_size > _shape[row_major::z] ? (_shape[row_major::z]-(ztile*tile_size)) : tile_size;

		  for(shape_value_t y = 0;y<_shape[row_major::y];++y){
			ytile = y / tile_size;
			y_intile = y % tile_size;
			ytile_shape = (ytile+1)*tile_size > _shape[row_major::y] ? (_shape[row_major::y]-(ytile*tile_size)) : tile_size;

			tile_id = ztile*n_tiles[sqeazy::row_major::y]*n_tiles[sqeazy::row_major::x]
			  + ytile*n_tiles[sqeazy::row_major::x]
			  ;

			for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size, ++tile_id){
			  xtile = x / tile_size;
			  xtile_shape = (xtile+1)*tile_size > _shape[row_major::x] ? (_shape[row_major::x]-(xtile*tile_size)) : tile_size;

			  tiles_iter = tiles[tile_id].begin() + (z_intile*xtile_shape*ytile_shape) + y_intile*xtile_shape;
			  std::copy(tiles_iter,
						tiles_iter + xtile_shape,
						value);

			  value+=xtile_shape;

			}
		  }
		}

		return value;
	  }


	};

  };

};

#endif /* _TILE_SHUFFLE_UTILS_H_ */
