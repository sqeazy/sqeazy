#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_

#include <algorithm>

namespace sqeazy {

  template <typename T> struct remove_unsigned { typedef T type; };
  template <> struct remove_unsigned<unsigned short> 		{ typedef short		type; };
  template <> struct remove_unsigned<unsigned int  > 		{ typedef int		type; };
  template <> struct remove_unsigned<unsigned long > 		{ typedef long		type; };
  template <> struct remove_unsigned<unsigned long long >	{ typedef long long	type; };

  template <typename T> struct add_unsigned { typedef T type; };
  template <> struct add_unsigned<short> 	{ typedef unsigned short	type; };
  template <> struct add_unsigned<int  > 	{ typedef unsigned int		type; };
  template <> struct add_unsigned<long > 	{ typedef unsigned long		type; };
  template <> struct add_unsigned<long long >	{ typedef unsigned long long	type; };

  template <typename T> struct twice_as_wide	{};
  template	<> struct twice_as_wide<unsigned char> { typedef unsigned short		type; 	};
  template	<> struct twice_as_wide<unsigned short>{ typedef unsigned int		type; 	};
  template	<> struct twice_as_wide<unsigned int>  { typedef unsigned long		type;	};
  template	<> struct twice_as_wide<unsigned long> { typedef unsigned long long	type;	};
  template	<> struct twice_as_wide<char>	       { typedef short		type; };
  template	<> struct twice_as_wide<short>	       { typedef int		type; };
  template	<> struct twice_as_wide<int>	       { typedef long		type; };
  template	<> struct twice_as_wide<long>	       { typedef long long	type; };



  enum error_code {
    
    SUCCESS = 0,
    FAILURE = 1
    
  };

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

  template <typename Neighborhood, typename T, typename U>
  T sum(const T* _ptr, const U& _index, 
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

  template <typename T, typename Neighborhood = last_plane_neighborhood<3> >
  struct diff_scheme {
    
    typedef T raw_type;
    typedef typename remove_unsigned<T>::type compressed_type;
    typedef typename add_unsigned<typename twice_as_wide<T>::type >::type sum_type;
    typedef unsigned size_type;

    static const error_code encode(const size_type& _width,
				   const size_type& _height,
				   const size_type& _depth,
				   const raw_type* _input,
				   compressed_type* _output) 
    {
      sum_type local_sum = 0;
      unsigned long length = _width*_height*_depth;
      
      for(unsigned long index = 0;index < length;++index){
	
	local_sum = sum<Neighborhood>(_input,index,_width, _height,_depth);
	
	_output[index] = _input[index] - local_sum/Neighborhood::traversed;
      }
      
      return SUCCESS;
    }

    static const error_code decode(const size_type& _width,
				   const size_type& _height,
				   const size_type& _depth,
				   const compressed_type* _input,
				   raw_type* _output) 
    {
      unsigned long length = _width*_height*_depth;

      std::copy(_input,_input + length, _output);
      
      sum_type local_sum = 0;
      for(unsigned long index = 0;index < length;++index){
	local_sum = sum<Neighborhood>(_output,index,_width, _height,_depth);
	
	_output[index] = _input[index] + local_sum/Neighborhood::traversed;
      }
      
      return SUCCESS;
    }

    
  };

  template < typename T>
  T setbits_of_integertype(const T& destination, const T& source, unsigned at, unsigned numbits)
  {
    T ones = ((1<<(numbits))-1)<<at;
    return (ones|destination)^((~source<<at)&ones);
  }

  template < typename T, const unsigned num_segments = 4  >
  struct bitswap_scheme {
    
    typedef T raw_type;
    typedef unsigned size_type;
    
    static const unsigned raw_type_num_bits = sizeof(T)*8;
    static const unsigned raw_type_num_bits_per_segment = raw_type_num_bits/num_segments;

    
    static const error_code encode(const raw_type* _input,
				   raw_type* _output,
				   const size_type& _length) 
    {
      
      const unsigned segment_length = _length/num_segments;
      const raw_type mask = ~(~0 << (raw_type_num_bits_per_segment));

      for(size_type seg_index = 0;seg_index<num_segments;++seg_index){

	size_type input_bit_offset = seg_index*raw_type_num_bits_per_segment;

	for(size_type index = 0;index < _length;++index){
	  
	  raw_type extracted_bits = (_input[index] >> input_bit_offset) & mask;
	  size_type output_bit_offset = ((index % num_segments)*raw_type_num_bits_per_segment);
	  size_type output_index = ((num_segments-1-seg_index)*segment_length) + (index/num_segments);

	  _output[output_index] = setbits_of_integertype(_output[output_index],extracted_bits,
							 output_bit_offset,
							 raw_type_num_bits_per_segment);
	}
      }
      
      return SUCCESS;
    }

    static const error_code decode(const raw_type* _input,
				   raw_type* _output,
				   const size_type& _length) 
    {
      
      const unsigned segment_length = _length/num_segments;
      const raw_type mask = ~(~0 << (raw_type_num_bits_per_segment));
	    
      for(size_type seg_index = 0;seg_index<num_segments;++seg_index){
	for(size_type index = 0;index < _length;++index){

	  size_type input_bit_offset = (index % num_segments)*raw_type_num_bits_per_segment;

	  size_type input_index = ((num_segments-1-seg_index)*segment_length) + index/num_segments;
	  raw_type extracted_bits = (_input[input_index] >> input_bit_offset) & mask;

	  size_type output_bit_offset = seg_index*raw_type_num_bits_per_segment;
	  

	  _output[index] = setbits_of_integertype(_output[index],extracted_bits,
						  output_bit_offset,raw_type_num_bits_per_segment);
	}
      }
	    
      return SUCCESS;
    }

    
  };

} //sqeazy

#endif /* _SQEAZY_IMPL_H_ */
