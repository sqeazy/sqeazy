#ifndef _TILE_SHUFFLE_UTILS_H_
#define _TILE_SHUFFLE_UTILS_H_

#include <cstdint>
#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "traits.hpp"

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

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t encode(in_iterator_t _begin,
			    in_iterator_t _end,
			    out_iterator_t _out,
			    const shape_container_t& _shape)  {
	
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
	  return encode_with_remainder(_begin,_end,_out,_shape);
	else{
	  // static const std::size_t n_elements_per_simd_block = 16/sizeof(in_value_t);

	  // if(tile_size % n_elements_per_simd_block == 0)
	  //   return encode_full_simd(_begin,_end,_out,_shape);
	  // else
	    return encode_full(_begin,_end,_out,_shape);
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
				 const shape_container_t& _shape) {

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
	const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);
       
	shape_container_t n_full_tiles = _shape;
       
	for(shape_value_t & n_tiles : n_full_tiles )
	  n_tiles = n_tiles / tile_size;

	const shape_value_t len_tiles = std::accumulate(n_full_tiles.begin(), n_full_tiles.end(),1,std::multiplies<shape_value_t>());

	// COLLECT TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector< std::vector<in_value_t> > tiles(len_tiles, std::vector<in_value_t>(n_elements_per_tile,0) );
	
	std::size_t ztile = 0;
	std::size_t ytile = 0;
	// std::size_t xtile = 0;
	
	std::size_t z_intile = 0;
	std::size_t y_intile = 0;

	std::size_t tile_id = 0;

	auto acc_iter = _begin;
	auto tiles_iter = tiles[0].begin();
	
	for(shape_value_t z = 0;z<_shape[row_major::z];++z){
	  ztile = z / tile_size;
	  z_intile = z % tile_size;
	    
	  for(shape_value_t y = 0;y<_shape[row_major::y];++y){
	    ytile = y / tile_size;
	    y_intile = y % tile_size;

	    tile_id = ztile*n_full_tiles[sqeazy::row_major::y]*n_full_tiles[sqeazy::row_major::x]
		+ ytile*n_full_tiles[sqeazy::row_major::x]
		;

	    for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size, acc_iter+=tile_size,++tile_id){
	      // xtile = x / tile_size;

	      tiles_iter = tiles[tile_id].begin();
	      
	      std::copy(acc_iter, acc_iter + tile_size,
			tiles_iter + (z_intile*n_elements_per_tile_frame) + y_intile*tile_size);
	      
	      
	    }
	  }
	}

	// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
	// median plus stddev around median or take 75% quantile directly

	std::vector<in_value_t> metric(len_tiles,0.);
	//std::vector<median_acc_t> acc   (len_tiles, median_acc_t());

	for(std::size_t i = 0;i<len_tiles;++i){
	  median_acc_t acc;
	  
	  for(std::size_t p = 0;p<n_elements_per_tile;++p){
	    acc(tiles[i][p]);
	  }
	  
	  metric[i] = std::round(bacc::median(acc));
	}
	
	// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
	decode_map.resize(len_tiles);
	
	auto sorted_metric = metric;
	std::sort(sorted_metric.begin(), sorted_metric.end());

	auto dst = _out;
	for(shape_value_t i =0;i<metric.size();++i){
	  auto original_index = std::find(metric.begin(), metric.end(), sorted_metric[i]) - metric.begin();
	  decode_map[i] = original_index;
	  dst = std::copy(tiles[original_index].begin(), tiles[original_index].end(),
			  dst);
	}
	

	return dst;
       
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
					   const shape_container_t& _shape)  {

	
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

	// COLLECT TILE CONTENT /////////////////////////////////////////////////////////////////////////////////////////////////////
	ztile = 0;
	ytile = 0;
	xtile = 0;

	ztile_shape = 0;
	ytile_shape = 0;
	xtile_shape = 0;

	tile_id = 0;

	
	std::size_t z_intile = 0;
	std::size_t y_intile = 0;

	auto acc_iter = _begin;
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

	      tiles_iter = tiles[tile_id].begin();
	      std::copy(acc_iter, acc_iter + xtile_shape,
			tiles_iter + (z_intile*xtile_shape*ytile_shape) + y_intile*xtile_shape);

	      acc_iter+=xtile_shape;
	      
	    }
	  }
	}

	// COLLECT STATISTICS /////////////////////////////////////////////////////////////////////////////////////////////////////
	// median plus stddev around median or take 75% quantile directly

	std::vector<in_value_t> metric(len_tiles,0.);
	//std::vector<median_acc_t> acc   (len_tiles, median_acc_t());

	for(std::size_t i = 0;i<len_tiles;++i){
	  median_acc_t acc;
	  
	  for(std::size_t p = 0;p<n_elements_per_tile;++p){
	    acc(tiles[i][p]);
	  }
	  
	  metric[i] = std::round(bacc::median(acc));
	}
	
	// PERFORM SHUFFLE /////////////////////////////////////////////////////////////////////////////////////////////////////
	decode_map.resize(len_tiles);
	
	auto sorted_metric = metric;
	std::sort(sorted_metric.begin(), sorted_metric.end());

	auto dst = _out;
	for(shape_value_t i =0;i<metric.size();++i){
	  auto original_index = std::find(metric.begin(), metric.end(), sorted_metric[i]) - metric.begin();
	  decode_map[i] = original_index;
	  dst = std::copy(tiles[original_index].begin(), tiles[original_index].end(),
			  dst);
	}
	

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
	
	//return encode(_begin, _end, _out,_shape);
	return decode_with_remainder(_begin, _end, _out,_shape);
	
      }

      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t decode_with_remainder(in_iterator_t _begin,
					   in_iterator_t _end,
					   out_iterator_t _out,
					   const shape_container_t& _shape) const {


	typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
	typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

	typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
	typedef typename std::remove_cv<in_value_type>::type in_value_t;
	
	const shape_container_t rem = remainder(_shape);
	const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

	shape_container_t full_tiles_per_dim = _shape;
	for(shape_value_t & tile : full_tiles_per_dim )
	  tile = tile / tile_size;

	shape_container_t tiles_per_dim = _shape;
	for(shape_value_t & tile : tiles_per_dim )
	  tile = (tile + tile_size - 1)/ tile_size;
	
	const std::size_t n_tiles = std::accumulate(tiles_per_dim.begin(), tiles_per_dim.end(),
						    1,
						    std::multiplies<std::size_t>());

	std::array<std::size_t, 3> tile_shape; tile_shape.fill(tile_size);
	std::vector<std::array<std::size_t, 3> > tile_shapes(n_tiles, tile_shape);
	std::vector<std::size_t>		 tile_sizes  (n_tiles, std::pow(tile_size,_shape.size()));

	const std::size_t max_n_rows_per_tile = std::pow(tile_size,_shape.size()-1);
	std::vector<std::size_t>		 tile_n_rows (n_tiles, max_n_rows_per_tile);
	
	
	if(has_remainder){
	  auto current_tile_shape = tile_shapes.begin();
	  auto current_tile_sizes = tile_sizes.begin()  ;
	  auto current_tile_n_rows = tile_n_rows.begin()  ; 
	  
	  for(shape_value_t z = 0;z<tiles_per_dim[row_major::z];++z){
	    tile_shape[row_major::z] = z != tiles_per_dim[row_major::z]-1 ? tile_size : rem[row_major::z];
	  
	    for(shape_value_t y = 0;y<tiles_per_dim[row_major::y];++y){
	      tile_shape[row_major::y] = y != tiles_per_dim[row_major::y]-1 ? tile_size : rem[row_major::y];
	    
	      for(shape_value_t x = 0;x<tiles_per_dim[row_major::x];++x){
		tile_shape[row_major::x] = x != tiles_per_dim[row_major::x]-1 ? tile_size : rem[row_major::x];

		*current_tile_shape = tile_shape;
		++current_tile_shape;

		*current_tile_sizes = std::accumulate(tile_shape.begin(), tile_shape.end(),1,std::multiplies<std::size_t>());
		++current_tile_sizes;

		*current_tile_n_rows = std::accumulate(tile_shape.begin(), tile_shape.end(),1,std::multiplies<std::size_t>())/tile_shape[row_major::x];
		++current_tile_n_rows;
	      }
	    }
	  }
	}

	std::vector<std::size_t> tile_size_sums(tile_sizes.size(),0);
	std::size_t running_sum = 0;
	for(std::size_t i = 0;i<tile_size_sums.size();++i){
	  tile_size_sums[i] = running_sum;
	  running_sum += tile_sizes[i];
	}

	shape_value_t ztile = 0;
	shape_value_t ytile = 0;
	shape_value_t xtile = 0;

	const std::size_t output_frame_size = _shape[row_major::x]*_shape[row_major::y];

	
	std::vector<std::array<std::size_t, 3> > tile_offsets(n_tiles, tile_shape);
	auto linear_offsets_begin = tile_offsets.begin();
	std::array<std::size_t, 3> loc;
	
	for(shape_value_t z = 0;z<_shape[row_major::z];z+=tile_size){
	  for(shape_value_t y = 0;y<_shape[row_major::y];y+=tile_size){
	    for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size){
	      loc[row_major::x] = x;
	      loc[row_major::y] = y;
	      loc[row_major::z] = z;
	      *linear_offsets_begin = loc;
	      ++linear_offsets_begin;
	    }
	  }
	}

	auto dst = _out;
	shape_value_t z_in_tile = 0;
	
	shape_value_t y_in_tile = 0;
	shape_value_t output_offset = 0;

	std::vector<in_value_t> buffer_row(_shape[row_major::x],0);
	auto buffer_row_begin = buffer_row.begin();
	in_iterator_t src = _begin;
	
	std::array<std::size_t, 3> dst_offset;dst_offset.fill(0);
	std::size_t dst_start_tile = 0;
	std::size_t write_count = 0;
	
	for(std::size_t row_in_tile = 0;row_in_tile<max_n_rows_per_tile;++row_in_tile){


	  for(std::size_t tile = 0;tile<n_tiles;++tile){

	    src = _begin + tile_size_sums[tile];

	    if(!(row_in_tile<tile_n_rows[tile]))
	      continue;

	    src += row_in_tile*tile_shapes[tile][row_major::x];
	    
	    buffer_row_begin = std::copy(src, src + tile_shapes[tile][row_major::x],
					 buffer_row_begin);

	    if(buffer_row_begin == buffer_row.end()){

	      dst_offset = tile_offsets[tile];
		
	      z_in_tile = row_in_tile / tile_shapes[tile][row_major::y];
	      y_in_tile = row_in_tile % tile_shapes[tile][row_major::y];

	      dst_start_tile = (dst_offset[row_major::z]+z_in_tile)*output_frame_size;
	      dst_start_tile += (dst_offset[row_major::y]+y_in_tile)*_shape[row_major::x]  ;
	      dst_start_tile += dst_offset[row_major::x] + tile_shapes[tile][row_major::x] ;
	      dst_start_tile -= buffer_row.size();
	      
	      dst = _out + dst_start_tile ;
	      		
	      dst = std::copy(buffer_row.begin(),
			      buffer_row.end(),
			      dst);
	      write_count += buffer_row.size();
	      buffer_row_begin = buffer_row.begin();
	      
	    }
	    
	  }
	}

	const std::size_t n_elements = (_end - _begin);
	if(write_count == n_elements)
	  return _out + write_count;
	else
	  return _out;
	
      }

      
    };

  };

};

#endif /* _TILE_SHUFFLE_UTILS_H_ */