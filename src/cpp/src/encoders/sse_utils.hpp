#ifndef _SSE_UTILS_H_
#define _SSE_UTILS_H_

#include <stdexcept>
#include <climits>
#include <array>
#include <iterator>
#include <bitset>
#include <string>

#include "boost/dynamic_bitset.hpp"


#include <xmmintrin.h>
#include <smmintrin.h>

namespace sqeazy {

  namespace detail {

    template <std::size_t N, typename iterator_t>
    void to_block_range(const std::bitset<N>& _src,
			iterator_t _begin,
			iterator_t _end){
  
      static const std::size_t ulong_bytes = sizeof(unsigned long);  
      static const std::size_t ulong_bits = ulong_bytes*CHAR_BIT;
      static const std::size_t n_chunks = (N + ulong_bits - 1)/ulong_bits;

      typedef typename std::remove_reference<decltype(*_begin)>::type value_t;
      
      // const std::size_t size = _end - _begin;
      // const std::size_t size_bytes = size*sizeof(*_begin);
      // const std::size_t size_bits = size_bytes*CHAR_BIT;
      
      auto dest = reinterpret_cast<value_t*>(&*_begin);
      unsigned long val = 0;
      
      for(std::size_t c = 0;c < n_chunks;++c){
	int right_shift = (n_chunks - 1 - c )*ulong_bits;
	auto temp = _src >> right_shift;
	temp &= ~((temp >> ulong_bits) << ulong_bits);
	val = temp.to_ulong();

	value_t* vbegin = reinterpret_cast<value_t*>(&val);
	value_t* vend = vbegin + (sizeof(val)/sizeof(value_t));
	std::reverse_iterator<value_t*> r(dest + ((c+1)*sizeof(val)/sizeof(value_t)));
	std::copy(vbegin,
		  vend,
		  r);
    
      }
    
  
    }

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


    template<typename in_type, std::uint32_t n_bits_per_segment = 1>
    struct bitshuffle
    {

      typedef typename std::make_unsigned<in_type>::type type;
      
      static const int type_width = sizeof(type)*CHAR_BIT;
      static const int simd_width = 128;
      static const int simd_width_bytes = simd_width/CHAR_BIT;
      static const int n_elements_per_simd = simd_width/type_width;
      static const int n_segments = type_width/n_bits_per_segment;
      static const int n_elements = simd_width_bytes*n_segments/sizeof(type);
      
      static_assert(n_bits_per_segment <= type_width,
		    "sqeazy::detail::bitshuffle received more n_bits_per_segment that given type yields");
      
      typedef typename std::bitset<simd_width> bitset_t;
      
      std::array<bitset_t, n_segments> segments;
      std::uint32_t n_bits_consumed;

      bitshuffle():
	segments(),
	n_bits_consumed(0){

	// std::fill(segments.begin(),
	// 	  segments.end(),
	// 	  boost::dynamic_bitset<type>(simd_width)
	// 	  );
	
      }


      bool empty() const {
	return n_bits_consumed == 0;
      }

      bool full() const {
	return n_bits_consumed == simd_width;
      }

      std::size_t size() const {
	return simd_width*n_segments;
      }

      std::size_t size_in_bytes() const {
	return simd_width*n_segments/CHAR_BIT;
      }

      
      std::size_t num_elements() const {
	return size_in_bytes()/sizeof(type);
      }
      
      void set(std::size_t seg_id, std::size_t bit_id = 0)  {

	if(seg_id>=segments.size())
	  return;

	if(bit_id>=segments[0].size())
	  return;

	segments[seg_id].set(bit_id);
	n_bits_consumed += 1;
      }
      
      void reset()  {
	for( auto & seg : segments ){
	  seg.reset();
	}
	n_bits_consumed = 0;;
      }
      
      bool any() const {
	bool value = false;
	for(auto & bits : segments )
	  value = value || bits.any() ;
	
	return value;
      }

      /**
	 \brief collect msb values from block in chunks of type, the resulting bitset contains the aquired values starting at the MSB

	 \param[in] 

	 \return 
	 \retval 

      */
      bitset_t gather_msb_range(__m128i block, int n_bits){

	bitset_t value;

	if(n_bits == 1){
	  gather_msb<type> op;
	  const auto val = op(block);
	  value = bitset_t(val);
	  value <<= (simd_width - (sizeof(val)*CHAR_BIT));
	}
	
	return value;
      }

      template <typename iterator_t>
      iterator_t consume(iterator_t _begin, iterator_t _end){

	static_assert(sizeof(decltype(*_begin)) == sizeof(type), "received iterator type does not match assumed type");

	std::size_t size = _end - _begin;
	
	if(size*sizeof(type) < simd_width_bytes)
	  return _begin;

	
	iterator_t value = _begin;
	const int step_size = simd_width/type_width;
	std::uint32_t segment_counter = 0;
	
	__m128i current;
	shift_left_m128i<type> left_shifter;
	bitset_t extracted;
	
	for(;value!=_end;value+=step_size, ++segment_counter){

	  if(full())
	    return value;
	  
	  current = _mm_load_si128(reinterpret_cast<const __m128i*>(&*value));

	  for(int s = 0;s<n_segments;++s){

	    //extract msb(s)
	    extracted = gather_msb_range(current,n_bits_per_segment);

	    //update segment
	    segments[s] |= (extracted >> (segment_counter*n_elements_per_simd));
	    
	    //left shift to bring the next segment to the MSB
	    current = left_shifter(current,n_bits_per_segment);
	  }

	  n_bits_consumed += (n_bits_per_segment*n_elements_per_simd);
	  
	}
		
	return value;
      }

      template <typename iterator_t>
      iterator_t write_segments(iterator_t _begin, iterator_t _end,
				std::size_t offset = 0){

	iterator_t value = _begin;
	std::size_t size = _end - _begin;


	if(size % segments.size() > n_elements_per_simd)
	  return value;

	std::size_t min_elements_required = segments[0].size()*segments.size()/(sizeof(*_begin)*CHAR_BIT);
	
	if(size < min_elements_required)
	  return value;


	std::size_t segment_spacing =  size / segments.size() ;

	if(segment_spacing < n_elements_per_simd)
	  return value;
	
	std::size_t global_offset = 0;
	
	for( std::uint32_t s = 0;s < segments.size();++s){
	  global_offset = s*segment_spacing;
	  value = _begin + global_offset + offset;
	  
	  to_block_range(segments[s],
			 value,
			 value+n_elements_per_simd);
	  
	}
	
	return _end;
      }

      
    };
    
  }//detail

}//sqeazy
#endif
