#ifndef _SSE_UTILS_H_
#define _SSE_UTILS_H_

#include <stdexcept>
#include <climits>

#include <xmmintrin.h>
#include <smmintrin.h>

namespace sqeazy {

  namespace detail {
  
    template<typename T>
    struct shift_left_m128i {
      __m128i operator()(const __m128i& _block, int num_bits) const {return _block;}
    };

    template<>
    struct shift_left_m128i<unsigned short> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_slli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<short> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_slli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<unsigned int> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_slli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<int> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_slli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<unsigned long> {
      __m128i operator()(const __m128i& _block, long num_bits) const {
	return  _mm_slli_epi64(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<long> {
      __m128i operator()(const __m128i& _block, long num_bits) const {
	return  _mm_slli_epi64(_block, num_bits);
      }
    };


    template<>
    struct shift_left_m128i<unsigned char> {
      __m128i operator()(const __m128i& _block, int num_bits) const {

	//treat upper half
	__m128i first = _mm_cvtepu8_epi16(_block);
	first = _mm_slli_epi16(first, num_bits);
	//collapse to first half
	first = _mm_shuffle_epi8(first, _mm_set_epi8(15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0) );

	//treat lower half
#ifdef WIN32 
	__m128 casted_block = _mm_castsi128_ps(_block);
	__m128i second = _mm_castps_si128(_mm_movehl_ps(casted_block, casted_block));
#else
	__m128 casted_block = reinterpret_cast<__m128>(_block);
	__m128i second = reinterpret_cast<__m128i>(_mm_movehl_ps(casted_block, casted_block));
#endif

    
	second = _mm_cvtepu8_epi16(second);
	second = _mm_slli_epi16(second, num_bits);
	second = _mm_shuffle_epi8(second,  _mm_set_epi8(14,12,10,8,6,4,2,0,15,13,11,9,7,5,3,1) );
#ifdef WIN32 
	__m128i value = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(second), _mm_castsi128_pd(first)));
#else
	__m128i value = reinterpret_cast<__m128i>(_mm_move_sd(reinterpret_cast<__m128d>(second), reinterpret_cast<__m128d>(first)));
#endif
	return value;
      }
    };

    template<>
    struct shift_left_m128i<char> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
    
	shift_left_m128i<unsigned char> left_shifter = {};
	return left_shifter(_block,num_bits);
    
      }
    };


    template<typename T>
    struct shift_right_m128i {
      __m128i operator()(const __m128i& _block, int num_bits) const {return _block;}
    };

    template<>
    struct shift_right_m128i<unsigned short> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_srli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_right_m128i<short> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_srli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_right_m128i<unsigned int> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_srli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_right_m128i<int> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
	return  _mm_srli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_right_m128i<unsigned long> {
      __m128i operator()(const __m128i& _block, long num_bits) const {
	return  _mm_srli_epi64(_block, num_bits);
      }
    };

    template<>
    struct shift_right_m128i<long> {
      __m128i operator()(const __m128i& _block, long num_bits) const {
	return  _mm_srli_epi64(_block, num_bits);
      }
    };


    template<>
    struct shift_right_m128i<unsigned char> {
      __m128i operator()(const __m128i& _block, int num_bits) const {

	//treat upper half
	__m128i first = _mm_cvtepu8_epi16(_block);
	first = _mm_srli_epi16(first, num_bits);
	//corlapse to first half
	first = _mm_shuffle_epi8(first, _mm_set_epi8(15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0) );

	//treat lower half
#ifdef WIN32 
	__m128 casted_block = _mm_castsi128_ps(_block);
	__m128i second = _mm_castps_si128(_mm_movehl_ps(casted_block, casted_block));
#else
	__m128 casted_block = reinterpret_cast<__m128>(_block);
	__m128i second = reinterpret_cast<__m128i>(_mm_movehl_ps(casted_block,casted_block));
#endif
	second = _mm_cvtepu8_epi16(second);
	second = _mm_srli_epi16(second, num_bits);
	second = _mm_shuffle_epi8(second,  _mm_set_epi8(14,12,10,8,6,4,2,0,15,13,11,9,7,5,3,1) );

#ifdef WIN32 
	__m128i value = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(second), _mm_castsi128_pd(first)));
#else
	__m128i value = reinterpret_cast<__m128i>(_mm_move_sd(reinterpret_cast<__m128d>(second), reinterpret_cast<__m128d>(first)));
#endif
	return value;
      }
    };

    template<>
    struct shift_right_m128i<char> {
      __m128i operator()(const __m128i& _block, int num_bits) const {
    
	shift_right_m128i<unsigned char> left_shifter;
	return left_shifter(_block,num_bits);
    
      }
    };

    template <typename T>
    struct vec_xor {

      typedef T value_type;

      void operator()(__m128i* _inout){
	throw std::runtime_error("vec_xor called with unknown type");
	return;
      }

    };

    template <>
    struct vec_xor<unsigned short> {

      typedef unsigned short value_type;
      static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

      void operator()(__m128i* _inout){
    
	static const __m128i vec_mask = _mm_set1_epi16(mask);
    
	*_inout = _mm_xor_si128(*_inout,vec_mask);

	return;
      }

    };

    template <>
    struct vec_xor< short> {

      typedef  short value_type;
      static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

      void operator()(__m128i* _inout){
    
	static const __m128i vec_mask = _mm_set1_epi16(mask);
    
	*_inout = _mm_xor_si128(*_inout,vec_mask);

	return;
      }

    };

    template <>
    struct vec_xor<unsigned char> {

      typedef unsigned short value_type;
      static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

      void operator()(__m128i* _inout){
    
	static const __m128i vec_mask = _mm_set1_epi8(mask);
    
	*_inout = _mm_xor_si128(*_inout,vec_mask);

	return;
      }

    };


    template <>
    struct vec_xor< char> {

      typedef  short value_type;
      static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

      void operator()(__m128i* _inout){
    
	static const __m128i vec_mask = _mm_set1_epi8(mask);
    
	*_inout = _mm_xor_si128(*_inout,vec_mask);

	return;
      }

    };

    template <typename T>
    struct vec_rotate_left
    {
  
  
      int shift;

      /**
	 \brief construct a vectorised left rotation
     
	 example:
	 input = 0001 0111

	 for _shift = 1:
	 vec_rotate_left(input): 0010 1110

	 for _shift = 2
	 vec_rotate_left(input): 0101 1100

	 \param[in] _shift number of bits to rotate by
     
	 \return 
	 \retval 

      */
      vec_rotate_left(int _shift = 1):
	shift(_shift)
      {};

      /**
	 \brief vectorized rotate of register
	 rotate bits of integer by <shift> to the left (wrapping around the ends)
	 input = 0001 0111
	 vec_rotate_left(input): 0010 1110
     
	 \param[in] _in SSE register to rotate

	 \return 
	 \retval 

      */
      __m128i operator()(const __m128i* _in){
	__m128i value;
	throw std::runtime_error("rotate_left called with unknown type");
	return value;
      }
    };

    template <>
    struct vec_rotate_left<unsigned short>
    {
      typedef unsigned short value_type;
      int shift;
      vec_rotate_left(int _shift = 1):
	shift(_shift)
      {};

      __m128i operator()(const __m128i* _in){
    
	static const value_type num_bits = (sizeof(value_type) * CHAR_BIT) - shift;

	// type shifted = _in << shift;
	__m128i value = *_in;
	value = _mm_slli_epi16(value, shift);

	static const value_type mask = ~(~0 << shift);
	static const __m128i vec_mask = _mm_set1_epi16(mask);

	// type left_over = (_in >> num_bits) & mask;
	__m128i left_over = _mm_srli_epi16(*_in,num_bits);
	left_over = _mm_and_si128(left_over, vec_mask);

	// return  (shifted | left_over);
	value = _mm_or_si128(value, left_over);
	return value;
      }
    };


    template <>
    struct vec_rotate_left<short>
    {
      typedef short value_type;
      int shift;
      vec_rotate_left(int _shift = 1):
	shift(_shift)
      {};

      __m128i operator()(const __m128i* _in){
	vec_rotate_left<unsigned short> rotater(shift);
	return rotater(_in);
      }
    };

    template <>
    struct vec_rotate_left<unsigned char>
    {
      typedef unsigned char value_type;
      int shift;
      vec_rotate_left(int _shift = 1):
	shift(_shift)
      {};

      __m128i operator()(const __m128i* _in){

	static const value_type num_bits = (sizeof(value_type) * CHAR_BIT) - shift;


	static const shift_left_m128i<value_type> left_shifter = {};
	static const shift_right_m128i<value_type> right_shifter = {};

	// type shifted = _in << shift;
	__m128i value = *_in;

    
    
	// value = _mm_slli_epi16(value, shift);
	value = left_shifter(value, shift);

	static const value_type mask = ~(~0 << shift);
	static const __m128i vec_mask = _mm_set1_epi8(mask);

	// type left_over = (_in >> num_bits) & mask;
	__m128i left_over = right_shifter(*_in,num_bits);
	left_over = _mm_and_si128(left_over, vec_mask);

	// return  (shifted | left_over);
	value = _mm_or_si128(value, left_over);
	return value;
      }
    };

    /**
       \brief zero extend last n blocks of T bitwidth and convert them 32bit blocks saved in result
   
       \param[in] 

       \return 
       \retval 

    */
    template <typename T>
    struct to_32bit_field
    {
      static const __m128i conversion(const __m128i& _block){
    
	//TODO convert to static assert
	if(sizeof(T)>4){
	  std::cerr << "[to_32bit_field::conversion]\tfailed due to incompatible type (sizeof(T)="<< sizeof(T) <<" > 4)\n";
	  throw std::runtime_error("[to_32bit_field::conversion]\tfailed due to incompatible type");
	}
	return _block;
      }
    };

    /**
       \brief zero extend input block per 16-bit element and save to 32-bit element in output
       example: 
       input  ...-0x80-0x80
       output 0x8000-0x8000-0x8000-0x8000
   
       \param[in] 

       \return 
       \retval 

    */
    template <>
    struct to_32bit_field<unsigned short>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepu16_epi32(_block);
      }
    };

    template <>
    struct to_32bit_field<short>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepi16_epi32(_block);
      }
    };


    /**
       \brief zero extend input block per 8-bit element and save to 32-bit element in output
       example: 
       input  ...-0x8-0x8-0x8-0x8
       output 0x8000-0x8000-0x8000-0x8000
   
       \param[in] 

       \return 
       \retval 

    */
    template <>
    struct to_32bit_field<unsigned char>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepu8_epi32(_block);
      }
    };

    template <>
    struct to_32bit_field<char>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepi8_epi32(_block);
      }
    };

  }

}
#endif
