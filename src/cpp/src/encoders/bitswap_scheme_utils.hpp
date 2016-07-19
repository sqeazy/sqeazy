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
    rotate_left<1>(input): 1000 1011
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

      static const unsigned raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;
      static const unsigned num_planes = raw_type_num_bits/num_bits_per_plane;

      const unsigned segment_length = _length/num_planes;
      static const unsigned type_width = CHAR_BIT*sizeof(raw_type);

      const raw_type mask = ~(~0 << (num_bits_per_plane));
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
   

    template <const unsigned num_bits_per_plane, 
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

      static const std::size_t raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;

      if(raw_type_num_bits==8){
	std::cerr << "8-bit SSE accelerated bitswap not implemented yet\n";
	return FAILURE;
      }
      
      static const std::size_t items_per_register = 128/raw_type_num_bits;
      static const std::size_t num_planes = raw_type_num_bits/num_bits_per_plane;

      //prepare output array of pointers
      std::vector<raw_type*> output_ptr(num_planes,0);
      const std::size_t plane_width = _length/num_planes;
      std::size_t offset = 0;
      for(std::size_t i = 0;i<output_ptr.size();++i){
	output_ptr[i] = _output + offset;
	offset += plane_width;
      }

      //one item of the output array may contain multiple results of the iterations
      float planesets_per_output = raw_type_num_bits/float(items_per_register);
      std::size_t n_planesets_per_output_item = 0;
      
      //
      std::size_t shift_reorder_by = 0;
      if(raw_type_num_bits>8){
	n_planesets_per_output_item = 
	shift_reorder_by = raw_type_num_bits/std::round(planesets_per_output);
	
      }
      
      if(raw_type_num_bits == 8){
	shift_reorder_by = 0;
      }

      
      
      vec_xor<raw_type> xoring;
      vec_rotate_left<raw_type> rotate;
      __m128i input;
      std::size_t count = 0;

      for(size_type index = 0; index < _length; index += items_per_register, count++ ) {

	input = _mm_load_si128(reinterpret_cast<const __m128i*>(&_input[index]));

	// no need to xor, but we implement it anyway for the sake of completeness
	if(std::numeric_limits<raw_type>::is_signed)
	  xoring(&input);

	input = rotate(&input);
      
	//bitplane reordering starts here!
	reorder_bitplanes<num_bits_per_plane>(input,
					      output_ptr,
					      (count % (std::size_t)std::round(planesets_per_output) == 0) ? shift_reorder_by : 0);
    
	//moving the pointers to the output array forward
	if(count % (std::size_t)std::round(planesets_per_output) != 0){
	  for(std::size_t i = 0;i<output_ptr.size();++i){
	    ++output_ptr[i];
	  }
	}
      
      }

      return SUCCESS;
    }


  };
}; //sqeazy
#endif /* _BITSWAP_SCHEME_UTILS_H_ */
