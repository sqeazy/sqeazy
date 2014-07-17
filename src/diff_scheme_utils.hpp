#ifndef _DIFF_SCHEME_UTILS_H_
#define _DIFF_SCHEME_UTILS_H_

#include <vector>

#include "sqeazy_traits.hpp"


namespace sqeazy {
  template < unsigned extent = 3>
  struct last_plane_neighborhood {
   
    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
 
    //the indexing assumed is :
    //axis_begin[0] = x_axis_begin
    //axis_begin[1] = y_axis_begin
    //axis_begin[2] = z_axis_begin

    //this is the inclusive start
    static const int z_offset_begin = -1;
    static const int y_offset_begin = -axis_half;
    static const int x_offset_begin = -axis_half;
    
    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = z_offset_begin+1;
    static const int y_offset_end = axis_half+1;
    static const int x_offset_end = axis_half+1;
    
    static const int traversed = (z_offset_end-z_offset_begin)*(y_offset_end-y_offset_begin)*(x_offset_end-x_offset_begin);
  };

  template < unsigned extent = 3>
  struct last_pixels_in_cube_neighborhood {
   
    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
 
    //the indexing assumed is :
    //axis_begin[0] = x_axis_begin
    //axis_begin[1] = y_axis_begin
    //axis_begin[2] = z_axis_begin

    //this is the inclusive start
    static const int z_offset_begin = -axis_half;
    static const int y_offset_begin = -axis_half;
    static const int x_offset_begin = -axis_half;
    
    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = 1;
    static const int y_offset_end = 1;
    static const int x_offset_end = 1;
    
    static const int traversed = (z_offset_end-z_offset_begin)*(y_offset_end-y_offset_begin)*(x_offset_end-x_offset_begin);

    
    
  };

  template < unsigned extent = 4>
  struct cube_neighborhood_excluding_pixel {
   
    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
 
    //the indexing assumed is :
    //axis_begin[0] = x_axis_begin
    //axis_begin[1] = y_axis_begin
    //axis_begin[2] = z_axis_begin

    //this is the inclusive start
    static const int z_offset_begin = -axis_length;
    static const int y_offset_begin = -axis_length;
    static const int x_offset_begin = -axis_length;
    
    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = 0;
    static const int y_offset_end = 0;
    static const int x_offset_end = 0;
    
    static const int traversed = (z_offset_end-z_offset_begin)*(y_offset_end-y_offset_begin)*(x_offset_end-x_offset_begin);

        
  };

  template <typename Neighborhood, typename T>
  std::vector<T> compute_offsets(const T& _w, 
					  const T& _h,
					  const T& _d){
    return std::vector<unsigned>();
  }

  template <typename Neighborhood, typename T, typename U>
  T halo_aware_sum(const T* _ptr, const U& _index, 
	const unsigned& _width, 
	const unsigned& _height,  
	const unsigned& _depth){

    typedef typename remove_unsigned<T>::type coord_t;

    U length = _width*_height*_depth;
    U frame = _width*_height;
    U sum_index = 0;
    unsigned long z_sum_index = 0;
    unsigned long y_sum_index = 0;
    unsigned long x_sum_index = 0;
    T sum = 0;
    
    coord_t z_pos = _index/frame;
	
    U frame_index = _index - z_pos*frame;

    coord_t y_pos = frame_index/_width;
    coord_t x_pos = _index - (z_pos*frame + y_pos*_width);

    for(long z_offset = Neighborhood::z_offset_begin;z_offset<Neighborhood::z_offset_end;++z_offset){
	  
      if((z_pos + z_offset)>-1 && (z_pos + z_offset)<_depth)
	z_sum_index =  (z_pos + z_offset)*frame ;
      else
	z_sum_index = length;
	  
      for(long y_offset = Neighborhood::y_offset_begin;y_offset<Neighborhood::y_offset_end;++y_offset){

	if((y_pos + y_offset)>-1 && (y_pos + y_offset)<_height)
	  y_sum_index =  (y_pos + y_offset)*_width ;
	else
	  y_sum_index = length;

	for(long x_offset = Neighborhood::x_offset_begin;x_offset<Neighborhood::x_offset_end;++x_offset){

	  if((x_pos + x_offset)>-1 && (x_pos + x_offset)<_width)
	    x_sum_index = x_pos + x_offset;
	  else
	    x_sum_index = length;

	  sum_index = z_sum_index + y_sum_index + x_sum_index;
	      
	  if(sum_index<length)
	    sum += _ptr[sum_index];
	  else
	    sum += 0;

	}
      }
    }
    
    return sum;
  }

template <typename Neighborhood, typename T, typename U>
T naive_sum(const T* _ptr, const U& _index, 
	    const unsigned& _width, 
	    const unsigned& _height,  
	    const unsigned& _depth){

    typedef typename remove_unsigned<T>::type coord_t;

    U length = _width*_height*_depth;
    U frame = _width*_height;
    U sum_index = 0;
    unsigned long z_sum_index = 0;
    unsigned long y_sum_index = 0;
    unsigned long x_sum_index = 0;
    T sum = 0;
    
    coord_t z_pos = _index/frame;
	
    U frame_index = _index - z_pos*frame;

    coord_t y_pos = frame_index/_width;
    coord_t x_pos = _index - (z_pos*frame + y_pos*_width);

    for(long z_offset = Neighborhood::z_offset_begin;z_offset<Neighborhood::z_offset_end;++z_offset){
	  
      for(long y_offset = Neighborhood::y_offset_begin;y_offset<Neighborhood::y_offset_end;++y_offset){

	for(long x_offset = Neighborhood::x_offset_begin;x_offset<Neighborhood::x_offset_end;++x_offset){

	  sum_index = z_sum_index + y_sum_index + x_sum_index;
	      
	  sum += _ptr[sum_index];

	}
      }
    }
    
    return sum;
  }


} //sqeazy
#endif /* _DIFF_SCHEME_UTILS_H_ */
