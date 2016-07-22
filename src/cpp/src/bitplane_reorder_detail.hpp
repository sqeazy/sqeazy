#ifndef _BITPLANE_REORDER_DETAIL_H_
#define _BITPLANE_REORDER_DETAIL_H_

#include "sse_utils.hpp"

/**
   \brief bitplane reordering given sse register _block, new planes will be put into _reordered
   
   \param[in] _block SSE block that contains X times of the type identified by T
   \param[out] _reordered bit planes to write reordered data into
   \param[in] _left_shift_output move the result number of bits (given by _left_shift_output) left 
   before writing it to _reordered

   Example: 
   given an 128-bit input block of 
   {1000000000000000, 1000000000000000, 1000000000000000, 1000000000000000, 
    1000000000000000, 1000000000000000, 1000000000000000, 1000000000000000}
   the expected output is
   {1111111100000000, 0000000000000000, 0000000000000000, 0000000000000000, 
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000,
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000,
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000}

    IMPORTANT: It is assumed that the items stored in __m128i originally were encoded as type T
   \return 
   \retval 
   
*/
template <unsigned plane_size, typename T>
void reorder_bitplanes(const __m128i& _block, 
		       std::vector<T*>& _reordered,
		       const unsigned& _left_shift_output = 0
		       ){

  static const unsigned type_width = sizeof(T)*CHAR_BIT;
  static const unsigned __m128i_in_bytes = 16;
  static const unsigned n_items_per_m128i = __m128i_in_bytes/(sizeof(T));
  static const unsigned n_items_per_m128i_half = n_items_per_m128i/2;
  static const unsigned n_planes_per_T = type_width/plane_size;

  // static_assert(n_items_per_m128i*plane_size <= 32,
  // 		"[reorder_bitplanes]\t SSE API forbids to extract more than 32bit\n");
  // static_assert((n_items_per_m128i*plane_size) % type_width == 0,
  // 		"[reorder_bitplanes]\t writing odd-sized planes to result array not implemented yet\n");
  static_assert(plane_size <= type_width,
  		"[reorder_bitplanes]\t plane_size larger than input type. Doing nothing.\n");
  
  float output_items_per_plane = n_items_per_m128i*plane_size/float(type_width);

  // unsigned reordered_byte = _reordered.size()*sizeof(T);
  // if(!(reordered_byte>=__m128i_in_bytes)){
  //   std::cerr << "[reorder_bitplanes]\t buffer for output has insufficent size, " 
  // 	      << "found " << reordered_byte << " B, min. required " << __m128i_in_bytes << "\n"
  // 	      // << "Doing Nothing."
  // 	      << "\n";
  //   return;
  // }
    
  const unsigned planes_per_output_segment = n_planes_per_T/_reordered.size();
  const unsigned bits_per_plane_on_output_item = type_width/planes_per_output_segment;
  static const unsigned char left_shift_to_msb = sizeof(int) - sizeof(T);
  
  shift_left_m128i<T> left_shifter;
  int result = 0;
  T to_write_to_output = 0;
  std::size_t output_index = 0;
  
  for(unsigned plane = 0;plane<(n_planes_per_T);plane+=plane_size){

    result = 0;
    to_write_to_output = 0;
    
    //left shift by plane to make the bit of interest the msb
    __m128i input = left_shifter(_block,plane);


    //aquire the leading bits of int32 blocks in m128
    //m128 consists of 4 32bit integers that originate from T-bitwidth blocks
    __m128i v_first_items =  _mm_slli_si128(
					    //convert to 32bits in order to get only the first (items 0,1,2,3 in _block) half of m128i
					    to_32bit_field<T>::conversion(input),
					    left_shift_to_msb //left shift by 2 bytes to move the bits towards the MSB
					    ); 

    //invert the sequence of v_first_items
    v_first_items = _mm_shuffle_epi32(v_first_items, _MM_SHUFFLE(0,1,2,3));
    
    __m128 first_part = *reinterpret_cast<__m128*>(&v_first_items);

    //collect the msb per 32bit item into result
    result = _mm_movemask_ps (first_part); 
    result <<= (n_items_per_m128i_half*plane_size);

#ifdef WIN32
    __m128 input_casted = _mm_castsi128_ps(input);
#else
    __m128 input_casted = reinterpret_cast<__m128>(input);
#endif

    //move second half of 128bits up
#ifdef WIN32
    __m128i swapped_second_first = _mm_castps_si128(//swap second with first half of 128bits
						    _mm_movehl_ps(input_casted, input_casted)
						    );
#else
    __m128i swapped_second_first = reinterpret_cast<__m128i>(//swap second with first half of 128bits
							     _mm_movehl_ps(input_casted,input_casted)
							     );
#endif
    
    __m128i v_second_items = _mm_slli_si128(//convert to 32bits in order to get only the first (items 0,1,2,3 in _block) half of m128i
					    to_32bit_field<T>::conversion(swapped_second_first),
					    //left shift by n bytes to get the lower half of 32bits 
					    //(which is the higher half of the original)
					    left_shift_to_msb
					    );

    //invert the sequence of v_second_items
    v_second_items = _mm_shuffle_epi32(v_second_items, _MM_SHUFFLE(0,1,2,3));

    //collect the msb per 32bit item into result
#ifdef WIN32
    result += _mm_movemask_ps(_mm_castsi128_ps(v_second_items));
#else
    result += _mm_movemask_ps (reinterpret_cast<__m128>(v_second_items)) ; 
#endif

    result <<= _left_shift_output;
       
    

    //dump the result to the output
    output_index = planes_per_output_segment > 1 ? plane/planes_per_output_segment : plane;

    if(!(output_items_per_plane>1)){
      //compute what needs to be written to the output
      if(planes_per_output_segment!=1){
	unsigned offset = ((plane % planes_per_output_segment)*bits_per_plane_on_output_item);
	to_write_to_output = T(result << offset);
      }
      else{
	to_write_to_output = T(result);
      }
    
      *(_reordered[output_index]) += to_write_to_output;

    }
    else{
      int n_output_items_per_plane = std::round(output_items_per_plane);
      for(int out_offset = 0; out_offset < n_output_items_per_plane; ++out_offset)
	{
	  *(_reordered[output_index]+n_output_items_per_plane-1-out_offset) += (result >> out_offset) & T(~0);
	}
    }

  }

}


#endif /* _BITPLANE_REORDER_DETAIL_H_ */
