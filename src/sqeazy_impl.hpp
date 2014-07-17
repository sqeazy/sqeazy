#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_

#include <algorithm>

#include "sqeazy_traits.hpp"
#include "diff_scheme_utils.hpp"
#include "bitswap_scheme_utils.hpp"

namespace sqeazy {

  // template <typename T> struct remove_unsigned { typedef T type; };
  // template <> struct remove_unsigned<unsigned short> 		{ typedef short		type; };
  // template <> struct remove_unsigned<unsigned int  > 		{ typedef int		type; };
  // template <> struct remove_unsigned<unsigned long > 		{ typedef long		type; };
  // template <> struct remove_unsigned<unsigned long long >	{ typedef long long	type; };

  // template <typename T> struct add_unsigned { typedef T type; };
  // template <> struct add_unsigned<short> 	{ typedef unsigned short	type; };
  // template <> struct add_unsigned<int  > 	{ typedef unsigned int		type; };
  // template <> struct add_unsigned<long > 	{ typedef unsigned long		type; };
  // template <> struct add_unsigned<long long >	{ typedef unsigned long long	type; };

  // template <typename T> struct twice_as_wide	{};
  // template	<> struct twice_as_wide<unsigned char> { typedef unsigned short		type; 	};
  // template	<> struct twice_as_wide<unsigned short>{ typedef unsigned int		type; 	};
  // template	<> struct twice_as_wide<unsigned int>  { typedef unsigned long		type;	};
  // template	<> struct twice_as_wide<unsigned long> { typedef unsigned long long	type;	};
  // template	<> struct twice_as_wide<char>	       { typedef short		type; };
  // template	<> struct twice_as_wide<short>	       { typedef int		type; };
  // template	<> struct twice_as_wide<int>	       { typedef long		type; };
  // template	<> struct twice_as_wide<long>	       { typedef long long	type; };



  enum error_code {
    
    SUCCESS = 0,
    FAILURE = 1
    
  };

  
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

      std::copy(_input, _input + length, _output);//crossing fingers due to possible type mismatch
      std::vector<size_type> offsets = sqeazy::compute_offsets<Neighborhood>(_width,_height,_depth);
      
      for(unsigned long index = 0;index < length;++index){
	
	local_sum = halo_aware_sum<Neighborhood>(_input,index,_width, _height,_depth);
	
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
	local_sum = halo_aware_sum<Neighborhood>(_output,index,_width, _height,_depth);
	
	_output[index] = _input[index] + local_sum/Neighborhood::traversed;
      }
      
      return SUCCESS;
    }

    
  };


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
