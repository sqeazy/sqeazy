#ifndef _SSE_UTILS_H_
#define _SSE_UTILS_H_

#include <stdexcept>
#include <climits>
#include <array>
#include <iterator>
#include <bitset>
#include <string>

#include <xmmintrin.h>
#include <smmintrin.h>

namespace sqeazy {

  namespace detail {

    

    /**
       \brief functor to 

       \param[in] 

       \return 
       \retval 

    */
    template<typename T>
    struct gather_msb {

      std::uint16_t operator()(const __m128i& _block) const {return std::uint16_t(0);}
      
    };

    template<>
    struct gather_msb<std::uint8_t> {

      std::uint16_t operator()(const __m128i& _block) const {

	//reverse m128
	static const __m128i reverse_bytes =  _mm_set_epi8(0,
							   1,
							   2,
							   3,
							   4,
							   5,
							   6,
							   7,
							   8,
							   9,
							   10,
							   11,
							   12,
							   13,
							   14,
							   15
							   );
	


	//NB:	the interpretation of  epi8 ordering of _mm_movemask_epi8 does not match the memory layout
	//	thus, the input m128 block needs to be reverse first
	__m128i input = _mm_shuffle_epi8(_block, reverse_bytes);
	const int result = _mm_movemask_epi8(input);

	std::uint16_t value = result;
	
	return value;

      }
      
    };

    template<>
    struct gather_msb<std::int8_t> {

      std::uint16_t operator()(const __m128i& _block) const {

	return gather_msb<std::uint8_t>()(_block);
	
      }
      
    };


    template<>
    struct gather_msb<std::uint16_t> {

      //collect MSBs of short elements in m128
      //result is always left shifted!
      std::uint16_t operator()(const __m128i& _block) const {

	//reverse and shift incoming block
	//lower half bytes are discarded
	static const __m128i shuffle_by = _mm_set_epi8(0xff	, 0xff	,
						       0xff	, 0xff	,
						       0xff	, 0xff	,
						       0xff	, 0xff  ,
						       1	, 3	,
						       5	, 7	,
						       9	, 11	,
						       13 	, 15
						       );
	
	__m128i temp = _mm_shuffle_epi8(_block,
					shuffle_by);

	const int result = _mm_movemask_epi8(temp);//result contains collapsed MSBs
	
	std::uint16_t value = result;
	value <<= 8;
	
	return value;

      }
      
    };

    template<>
    struct gather_msb<std::int16_t> {

      std::uint16_t operator()(const __m128i& _block) const {
	return gather_msb<std::uint16_t>()(_block);
      }
      
    };

    template<>
    struct gather_msb<std::uint32_t> {

      std::uint16_t operator()(const __m128i& _block) const {

	static const int reverse_mask = 0 |
	  3 |
	  (2 << 2) |
	  (1 << 4);
	
	auto input = _mm_castsi128_ps(_mm_shuffle_epi32(_block, reverse_mask));
	std::uint16_t value =  _mm_movemask_ps(input);
	value <<= 12;
	return value;
      }
    };

    template<>
    struct gather_msb<std::int32_t> {

      std::uint16_t operator()(const __m128i& _block) const {
	return gather_msb<std::uint32_t>()(_block);
      }
    };
    
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
    struct to_32bit_field<std::uint16_t>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepu16_epi32(_block);
      }
    };

    template <>
    struct to_32bit_field<std::int16_t>
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
    struct to_32bit_field<std::uint8_t>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepu8_epi32(_block);
      }
    };

    template <>
    struct to_32bit_field<std::int8_t>
    {
      static const __m128i conversion(const __m128i& _block){
	return _mm_cvtepi8_epi32(_block);
      }
    };

    static inline __m128i sse_insert_epi16(const __m128i& _data,
					  int _value,
					  int _position){

      switch(_position){
      case 0: return _mm_insert_epi16(_data,_value,0);break;
      case 1: return _mm_insert_epi16(_data,_value,1);break;
      case 2: return _mm_insert_epi16(_data,_value,2);break;
      case 3: return _mm_insert_epi16(_data,_value,3);break;
      case 4: return _mm_insert_epi16(_data,_value,4);break;
      case 5: return _mm_insert_epi16(_data,_value,5);break;
      case 6: return _mm_insert_epi16(_data,_value,6);break;
      case 7: return _mm_insert_epi16(_data,_value,7);break;
      default: return _data;break;
      };
  
    }

    template <typename in_iterator_t, typename out_iterator_t>
    void simd_segment_broadcast(in_iterator_t _begin,
				in_iterator_t _end,
				out_iterator_t _dst){

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[sse_segment_broadcast] received value is not integral");
      static_assert(std::is_integral<in_value_t>::value, "[sse_segment_broadcast] received value is not integral");
      //static_assert((std::is_same<value_t,std::uint16_t>::value || std::is_same<value_t,std::int16_t>::value) && sizeof(value_t) == 2, "[simd_copy] unsigned short != 2");
  
				// static const std::size_t n_bytes_simd		= sizeof(__m128i);
      //      static const std::size_t n_bits_simd		= n_bytes_simd*CHAR_BIT;
      static const std::size_t n_bits_in_value_t	= sizeof(in_value_t)*CHAR_BIT;
      constexpr    std::size_t n_elements_per_simd	= 128/(CHAR_BIT*sizeof(in_value_t));

      const std::size_t n_segments		= sizeof(in_value_t)*CHAR_BIT;

      const std::size_t n_elements		= _end - _begin;
      const std::size_t segment_offset	= n_elements/n_segments;
      const std::size_t n_elements_per_seg	= n_elements*1/n_bits_in_value_t;
      const std::size_t n_iterations	= n_elements/n_elements_per_simd;
      constexpr std::size_t n_filled_segments_per_simd	= 16/n_elements_per_simd;
  
      assert(n_elements >= n_elements_per_simd && "received iterator range is smaller than SIMD register");
      if(n_elements_per_seg > segment_offset)
	return;
  
      __m128i input_block;
      __m128i output_block;
      sqeazy::detail::gather_msb<in_value_t> collect;
      sqeazy::detail::vec_xor<in_value_t> xoring;
      sqeazy::detail::vec_rotate_left<in_value_t> rotate_left;
      sqeazy::detail::shift_left_m128i<in_value_t> shift_left;
  
      in_iterator_t input;
      out_iterator_t output;  // out_iterator_t outpute = output + n_elements;
      std::uint16_t result = 0;
      int pos = 0;
      
      for(std::size_t s = 0;s<n_segments;++s){
	input = _begin;
	output = _dst + s*segment_offset;

	pos = 0;
	
	for(std::size_t i = 0;i<n_iterations;i+=n_filled_segments_per_simd){

	  result = 0;

	  //TODO: this can be unrolled
      	  for(std::size_t l = 0;l<n_filled_segments_per_simd;++l){
	
	    input_block = _mm_load_si128(reinterpret_cast<const __m128i*>(&*input));

	    if(std::numeric_limits<in_value_t>::is_signed)
	      xoring(&input_block);
	  
	    input_block = rotate_left(&input_block); // 
	    
	    input_block = shift_left(input_block,s);
	
	    //result <<= l*(16/n_filled_segments_per_simd);
	    std::uint16_t temp = collect(input_block);
	    
	    result |= (temp >> l*(16/n_filled_segments_per_simd) );
	
	    input += n_elements_per_simd;
	    
	  }
      

	  output_block = sse_insert_epi16(output_block,result,pos);
	  pos++;
      
	  if(pos > 7){
	    _mm_store_si128(reinterpret_cast<__m128i*>(&*output),output_block);
	    output += n_elements_per_simd;
	    pos = 0;
	  }
	  
	}// i iteratos
      } // s segments 
    }

  }//detail

}//sqeazy
#endif
