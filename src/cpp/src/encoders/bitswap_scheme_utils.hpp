#ifndef _BITSWAP_SCHEME_UTILS_H_
#define _BITSWAP_SCHEME_UTILS_H_
#include <limits>
#include <climits>
#include <iostream>
#include "traits.hpp"
#include "sqeazy_common.hpp"
#include "bitplane_reorder_detail.hpp"

namespace sqeazy {


  /*
    rotate bits of integer by <shift> to the left (wrapping around the ends)
    input = 0001 0111
    rotate_left<1>(input): 0010 1110
    rotate_left<2>(input): 0101 1100
  
  */
  template <unsigned shift,typename T >
  T rotate_left(const T& _in)  {
    typedef typename sqeazy::add_unsigned<T>::type type;
    static const unsigned num_bits = (sizeof(type) * CHAR_BIT) - shift;
    
    type shifted = _in << shift;
    static const type mask = ~(~0 << shift);
    type left_over = (_in >> num_bits) & mask;

    return  (shifted | left_over);
  }

  /*
    rotate bits of integer by <shift> to the left (wrapping around the ends)
    input = 0001 0111
    rotate_right<1>(input): 1000 1011
    rotate_right<2>(input): 1100 0101
  */
  template <unsigned shift,typename T>
  T rotate_right(const T& _in)  {
    typedef typename sqeazy::add_unsigned<T>::type type;
    static const unsigned num_bits = (sizeof(type) * CHAR_BIT) - shift;
    // std::cout << "rotate_right\t" << sizeof(type) << "*" << CHAR_BIT << " - " << shift << " = "<< num_bits << "\n";
    static const type mask = ~(~0 << num_bits);
    type shifted = (_in >> shift) & mask;
    type right_over = _in << num_bits;

    return  (shifted | right_over);
  }

  /**
     \brief xor the given type with a 01...10 bitmask, for a 8-bit number 0x0b perform:
     -42 ^ 127 = -87
     0xd6 ^ 0x7f = 0xa9
     11010110 ^ 01111111 = 10101001

     \param[in] _in number to XOR

     \return 
     \retval 

  */
  template <typename T>
  static typename boost::enable_if_c<std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
    //signed version
    static const T mask = ~(T(1 << ((sizeof(T)*CHAR_BIT) - 1)));
    if ( _in & ~mask )
      return _in ^ mask;
    else
      return _in;
  }

  template <typename T>
  static typename boost::enable_if_c<!std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
    //unsigned version
    return _in;
  }
  

  template < typename T>
  T setbits_of_integertype(const T& destination, const T& source, unsigned at, unsigned numbits)
  {
    T ones = ((1<<(numbits))-1)<<at;
    return (ones|destination)^((~source<<at)&ones);
  }

  namespace detail {
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //scalar implementation
    template <const unsigned num_bits_per_plane, 
	      typename raw_type , 
	      typename size_type
	      >
    static const error_code scalar_bitplane_reorder_encode(const raw_type* _input,
							   raw_type* _output,
							   const size_type& _length){

      static const unsigned type_width = CHAR_BIT*sizeof(raw_type);
      static const unsigned num_planes = type_width/num_bits_per_plane;

      const unsigned segment_length = _length/num_planes;
      
      const raw_type mask = ~(~0 << (num_bits_per_plane));//mask to extract all last bits up to bit at num_bits_per_plane
      raw_type value = 0;

      for(size_type index = 0; index < _length; ++index) {
	  

	value = xor_if_signed(_input[index]);
	value = rotate_left<1>(value);

	size_type output_bit_offset = (type_width - num_bits_per_plane) - ((index % num_planes)*num_bits_per_plane);

	for(unsigned plane_index = 0; plane_index<num_planes; ++plane_index) {

	  size_type input_bit_offset = plane_index*num_bits_per_plane;
	  raw_type extracted_bits = (value >> input_bit_offset) & mask;

	  size_type output_index = ((num_planes-1-plane_index)*segment_length) + (index/num_planes);

	  _output[output_index] = setbits_of_integertype(_output[output_index],
							 extracted_bits,
							 output_bit_offset,
							 num_bits_per_plane);
	}
      }

      return SUCCESS;

    }


    template <const unsigned num_bits_per_plane, 
	      typename raw_type , 
	      typename size_type
	      >
    static const error_code scalar_bitplane_reorder_decode(const raw_type* _input,
							   raw_type* _output,
							   const size_type& _length){

      static const unsigned raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;
      static const unsigned num_planes = raw_type_num_bits/num_bits_per_plane;

      const unsigned segment_length = _length/num_planes;
      const raw_type mask = ~(~0 << (num_bits_per_plane));
      static const unsigned type_width = CHAR_BIT*sizeof(raw_type);

      for(size_type plane_index = 0; plane_index<num_planes; ++plane_index) {


	size_type output_bit_offset = (plane_index*num_bits_per_plane);
		
	for(size_type index = 0; index < _length; ++index) {

	  size_type input_bit_offset = (type_width - num_bits_per_plane) - (index % num_planes)*num_bits_per_plane;
	  size_type input_index = ((num_planes-1-plane_index)*segment_length) + index/num_planes;
	  raw_type extracted_bits = (_input[input_index] >> input_bit_offset) & mask;

	  _output[index] = setbits_of_integertype(_output[index],extracted_bits,
						  output_bit_offset,num_bits_per_plane);
	}
      }

      for(size_type index = 0; index < _length; ++index) {
	_output[index] = xor_if_signed(rotate_right<1>(_output[index]));
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


  };
}; //sqeazy
#endif /* _BITSWAP_SCHEME_UTILS_H_ */
