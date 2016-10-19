#ifndef _BITSWAP_SCHEME_SSE_H_
#define _BITSWAP_SCHEME_SSE_H_



#include "sqeazy_common.hpp"
#include "sse_utils.hpp"


namespace sqeazy {

  namespace detail {

    
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

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SSE implementation
    /*
      given an input array of type T: 
      * we loop through the array with stride n
      * load n elements into an SSE register
      * xor with 0x7f for 8-bit data, 0x7fff for 16-bit data, 0x7fffffff for 32-bit data, etc. if the input is signed
      * rotate the input left by one bit
      * perform the bitplane reordering
      * continue to next stride
      */
    template <const unsigned nbits_per_plane, 
	      typename raw_type , 
	      typename size_type
	      >
    static const error_code sse_bitplane_reorder_encode_deprecated(const raw_type* _input,
							raw_type* _output,
							const size_type& _length)
    {
      //unable to perform this task input array does fit into __m128
      if(_length*sizeof(raw_type)<16)
	return FAILURE;

      static const std::size_t raw_type_bitwidth = sizeof(raw_type)*CHAR_BIT;
      static const std::size_t input_items_per_m128 = 128/raw_type_bitwidth;
      static const std::size_t num_planes = raw_type_bitwidth/nbits_per_plane;
      const std::size_t output_segment_length = _length/num_planes;

      //prepare output array of pointers
      std::vector<raw_type*> output_ptr(num_planes,0);
      std::size_t offset = 0;
      for(std::size_t i = 0;i<output_ptr.size();++i){
	output_ptr[i] = _output + offset;
	offset += output_segment_length;
      }

      //prepare strides
      float output_items_per_m128 = nbits_per_plane*input_items_per_m128/float(raw_type_bitwidth);
      std::uint32_t advance_output_by = 1;
      std::uint32_t advance_output_every = 1;
      
      if(output_items_per_m128<1){
	advance_output_every = std::round(1./output_items_per_m128);
      } else {
	advance_output_by = output_items_per_m128;
      }

      // if(raw_type_bitwidth==8){
      // 	std::cerr << "8-bit SSE accelerated bitswap not implemented yet\n";
      // 	return FAILURE;
      // }
      
      //setup data structures for looping
      vec_xor<raw_type> xoring;
      vec_rotate_left<raw_type> rotate;
      __m128i input;
      std::size_t count = 0;

      for(size_type index = 0; index < _length; index += input_items_per_m128, count++ ) {

	input = _mm_load_si128(reinterpret_cast<const __m128i*>(&_input[index]));

	// no need to xor, but we implement it anyway for the sake of completeness
	if(std::numeric_limits<raw_type>::is_signed)
	  xoring(&input);

	input = rotate(&input);
      
	//bitplane reordering starts here
	std::size_t shift_left_by = (count % advance_output_every == 0) ? raw_type_bitwidth/advance_output_every : 0;
	reorder_bitplanes<nbits_per_plane>(input,
					   output_ptr,
					   shift_left_by);
    
	//moving the pointers to the output array forward
	if(shift_left_by == 0){
	  for(std::size_t i = 0;i<output_ptr.size();++i){
	    output_ptr[i]+=advance_output_by;
	  }
	}
	
      
      }

      return SUCCESS;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SSE implementation
    /*
      given an input array of type T: 
      * we loop through the array with stride n
      * load n elements into an SSE register
      * xor with 0x7f for 8-bit data, 0x7fff for 16-bit data, 0x7fffffff for 32-bit data, etc. if the input is signed
      * rotate the input left by one bit
      * perform the bitplane reordering
      * continue to next stride
      */
    template <const unsigned nbits_per_plane, 
	      typename raw_type , 
	      typename size_type
	      >
    static const error_code sse_bitplane_reorder_encode(const raw_type* _input,
							raw_type* _output,
							const size_type& _length)
    {
      //static_assert(nbits_per_plane == 1, "sse_bitplane_reorder_encode does not yet support bitplane widths larger than 1");

      sqeazy::detail::bitshuffle<raw_type, nbits_per_plane> instance;
      const std::uint32_t n_iterations = _length/(instance.num_elements());
      const std::uint32_t rest_iterations = _length % (instance.num_elements());
      if(rest_iterations)
	std::cerr << "[sse_bitplane_reorder_encode] WARNING input array size has remainder, skipping last "
		  << rest_iterations << " elements\n";

      std::size_t missed_elements_consume = 0;
      std::size_t missed_elements_written = 0;
      //setup data structures for looping
      vec_xor<raw_type> xoring;
      vec_rotate_left<raw_type> rotate;

      __m128i input;
      
     
      
      std::array<raw_type, sqeazy::detail::bitshuffle<raw_type, nbits_per_plane>::n_elements> temp;
      
      for( std::uint32_t it = 0;it < n_iterations;++it){

	auto in_begin	= _input + it*instance.num_elements();
	auto in_end	= in_begin + instance.num_elements();

	std::copy(in_begin,
		  in_end,
		  temp.begin());

	for(auto chunk = temp.begin();
	    chunk!= temp.end();
	    chunk += sqeazy::detail::bitshuffle<raw_type, nbits_per_plane>::n_elements_per_simd){

	  //TODO: replace this with _mm_stream_store_si128?
	  input = _mm_load_si128(reinterpret_cast<const __m128i*>(&*chunk));
	  
	  // no need to xor, but we implement it anyway for the sake of completeness
	  if(std::numeric_limits<raw_type>::is_signed)
	    xoring(&input);
	  
	  input = rotate(&input);
	  
	  //store input back
	  _mm_stream_si128(reinterpret_cast<__m128i*>(&*chunk),
			   input);
	  
	}
	
	
	instance.reset();

	auto consumed = instance.consume(temp.begin(),
					 temp.end()
					 );

	missed_elements_consume += (temp.end() - consumed);
		
	auto written = instance.write_segments(_output,
					       _output + _length,
					       it*sqeazy::detail::bitshuffle<raw_type>::n_elements_per_simd);

	missed_elements_written += (_output + _length - written);
	
      }
      
      if(missed_elements_consume){
	std::cerr << "[sse_bitplane_reorder_encode] failed to reorder array for "<< missed_elements_consume <<" elements \n";
	return FAILURE;
      }

      if(missed_elements_written){
	std::cerr << "[sse_bitplane_reorder_encode] failed to write "<< missed_elements_written <<" elements of aggregated bitplanes\n";
	return FAILURE;
      }

      return SUCCESS;
      
    }
    
  }//namespace detail

}//namespace sqeazy

#endif /* _BITPLANE_REORDER_DETAIL_H_ */
