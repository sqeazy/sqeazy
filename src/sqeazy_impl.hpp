#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_

#include <algorithm>

#include "sqeazy_traits.hpp"
#include "diff_scheme_utils.hpp"
#include "bitswap_scheme_utils.hpp"
#include "hist_impl.hpp"

namespace sqeazy {

  enum error_code {
    
    SUCCESS = 0,
    FAILURE = 1,
    NOT_IMPLEMENTED_YET = 42
    
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
      
      unsigned long length = _width*_height*_depth;
      std::copy(_input, _input + length, _output);//crossing fingers due to possible type mismatch

      std::vector<size_type> offsets; 
      sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
      geometry.compute_offsets_in_x(offsets);
      std::vector<size_type>::const_iterator offsetsItr = offsets.begin(); 

      const size_type end_ = geometry.non_halo_end(0)-1;
      sum_type local_sum = 0;

      for(;offsetsItr!=offsets.end();++offsetsItr){
	for(unsigned long index = 0;index < end_;++index){
	  
	  const size_type local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_input,local_index,_width, _height,_depth);
	  _output[local_index] = _input[local_index] - local_sum/Neighborhood::traversed;

	}
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

      std::vector<size_type> offsets; 
      sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
      geometry.compute_offsets_in_x(offsets);
      std::vector<size_type>::const_iterator offsetsItr = offsets.begin(); 

      const size_type end_ = geometry.non_halo_end(0)-1;
      sum_type local_sum = 0;

      for(;offsetsItr!=offsets.end();++offsetsItr){
	for(unsigned long index = 0;index < end_;++index){
	  
	  const size_type local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_output,local_index,_width, _height,_depth);
	  _output[local_index] = _input[local_index] + local_sum/Neighborhood::traversed;

	}
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

	  raw_type value = rotate_left<1>(xor_if_signed(_input[index]));
	  raw_type extracted_bits = (value >> input_bit_offset) & mask;
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
	  
      for(size_type index = 0;index < _length;++index){
	_output[index] = xor_if_signed(rotate_right<1>(_output[index]));
      }
  
      return SUCCESS;
    }

    
  };

  

  template < typename T, typename S = long >
  struct remove_background {
    
    typedef T raw_type;
    typedef S size_type;
    
    
    static const error_code encode(raw_type* _input,
				   raw_type* _output,
				   const size_type& _length,
				   const raw_type& _epsilon) 
    {
      histogram<raw_type> incoming(_input, _length);
      raw_type threshold = incoming.mode() + _epsilon;
      
      if(_output)
	return encode_out_of_place(_input, _output, _length, threshold);
      else
	return encode_inplace(_input, _length, threshold);
      
    }

    static const error_code estimated_encode(raw_type* _input,
				   raw_type* _output,
				   const size_type& _length) 
    {
      histogram<raw_type> incoming(_input, _length);
      raw_type threshold = 2*incoming.mode();
      
      if(_output)
	return encode_out_of_place(_input, _output, _length, threshold);
      else
	return encode_inplace(_input, _length, threshold);
      
    }

    static const error_code encode_out_of_place(raw_type* _input,
						raw_type* _output,
						const size_type& _length,
						const raw_type& _threshold) 
    {

      for(unsigned long vox = 0;vox<_length;++vox){
	_output[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
      }
      
      return SUCCESS;
    }


    static const error_code encode_inplace(raw_type* _input,
					   const size_type& _length,
					   const raw_type& _threshold) 
    {

      for(unsigned long vox = 0;vox<_length;++vox){
	_input[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
      }
      
      
      return SUCCESS;
    }

    static const error_code decode(const raw_type* _input,
				   raw_type* _output,
				   const size_type& _length) 
    {
      
      return NOT_IMPLEMENTED_YET;
    }

    
  };


} //sqeazy

#endif /* _SQEAZY_IMPL_H_ */
