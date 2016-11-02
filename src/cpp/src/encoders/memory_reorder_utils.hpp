#ifndef _MEMORY_REORDER_UTILS_H_
#define _MEMORY_REORDER_UTILS_H_

#include <cstdint>
#include <iterator>

#include "traits.hpp"

namespace sqeazy {

  namespace detail {

    
    struct reorder {

      std::size_t tile_size;

      reorder(std::size_t _tsize):
	tile_size(_tsize){

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
			    const shape_container_t& _shape) const {

	typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
	typedef typename std::remove_cv<in_value_type>::type in_value_t;

	typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
	typedef typename std::remove_cv<out_value_type>::type out_value_t;

	typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
	typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

	static_assert(sizeof(in_value_type) == sizeof(out_value_type), "[sqeazy::detail::reorder::encode] reorder received non-matching types");

	if(_shape.size()!=3){
	  std::cerr << "[sqeazy::detail::reorder::encode] received non-3D shape which is currently unsupported!\n";
	  return _out;
	}
	
	std::size_t n_elements = _end - _begin;
	std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
							    1,
							    std::multiplies<std::size_t>());
	if(n_elements_from_shape != n_elements){
	  std::cerr << "[sqeazy::detail::reorder::encode] input iterator range does not match shape in 1D size!\n";
	  return _out;
	}
	
	const shape_container_t rem = remainder(_shape);
	const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
	const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);
	
	shape_container_t n_full_tiles = _shape;
	
	for(shape_value_t & n_tiles : n_full_tiles )
	  n_tiles = n_tiles / tile_size;

	std::size_t in_row  = 0;
	std::size_t out_tile_offset = 0;

	std::size_t ztile = 0;
	std::size_t ytile = 0;
	std::size_t xtile = 0;

	std::size_t z_intile_row_offset = 0;
	std::size_t y_intile_row_offset = 0;
	std::size_t x_intile_row_offset = 0;
	std::size_t intile_row_offset = 0;
	
	std::size_t n_elements_2_copy = 0;
	out_iterator_t dst = _out;
	
	for(shape_value_t z = 0;z<_shape[row_major::z];++z){
	  
	  ztile = z / tile_size;
	  z_intile_row_offset = z % tile_size;
	  
	  for(shape_value_t y = 0;y<_shape[row_major::y];++y){

	    ytile = y / tile_size;
	    y_intile_row_offset = y % tile_size;
	    
	    xtile = 0;
	    
	    in_row = (z*_shape[row_major::y]*_shape[row_major::x]) + (y*_shape[row_major::x]);
	    out_tile_offset = (ztile*n_full_tiles[row_major::x]*n_full_tiles[row_major::y] + ytile*n_full_tiles[row_major::x]);
	    
	    for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size,++xtile){
	      
	      intile_row_offset = z_intile_row_offset*n_elements_per_tile_frame + y_intile_row_offset*tile_size;
		
	      dst = std::copy(_begin + in_row + x,
			      _begin + in_row + x + tile_size,
			      _out + (out_tile_offset*n_elements_per_tile) + intile_row_offset
			      );
	      
	      out_tile_offset+=1;
	    }
	    
	  }

	}

	bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});
	if(!has_remainder)
	  return dst;//_out + n_elements;
	    


	//TODO: handle remainders


	
	return _out;
      }


      template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
      out_iterator_t decode(in_iterator_t _begin,
			    in_iterator_t _end,
			    out_iterator_t _out,
			    const shape_container_t& _shape) const {
	
	return encode(_begin, _end, _out,_shape);
	
      }
      
    };

  };

};

#endif /* _MEMORY_REORDER_UTILS_H_ */
