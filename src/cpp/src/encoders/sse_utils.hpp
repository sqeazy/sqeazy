#ifndef _SSE_UTILS_H_
#define _SSE_UTILS_H_

#include <stdexcept>
#include <climits>
#include <array>
#include <iterator>
#include <bitset>
#include <string>
#include <cassert>

#include <xmmintrin.h>
#include <smmintrin.h>

#ifdef _OPENMP
#include "omp.h"
#endif

namespace sqeazy {

  namespace detail {

    struct is_aligned
    {

      template <typename T>
      static bool for_sse(T* _data){

        const std::size_t address = (std::size_t)_data;
        return address % 16 == 0;

      }

      template <typename T>
      static bool for_avx(T* _data){

        const std::size_t address = (std::size_t)_data;
        return address % 32 == 0;

      }

    };

    /**
       \brief functor to collect the MSBs of the input block, the template parameter dictates the interpretation of the argument SSE block
       so for example, give a sse block of (in pseudo code)
       __m128i block = {0x80000000, 0x80000000, 0x80000000, 0x80000000}
       this functor would retrieve the MSBs and store it in a std::uint16_t value as
       gather_msb<int>(block) -> 1111000000000000 = 0xf000

       \param[in]

       \return
       \retval

    */
    template<typename T>
    struct gather_msb {

      std::uint16_t operator()(const __m128i& _block) const {return std::uint16_t(0);}

    };

    /**
       \brief 8bit specialisation of the MSB collector, give a sse block of (in pseudo code)
       __m128i block = {0x80, 0x80, 0x80, 0x80, ... <repeats 12 times> ...}
       this functor would retrieve the MSBs and store it in a std::uint16_t value as
       gather_msb<uint8_t>(block) 1111111111111111 = 0xffff
       block = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 , 0, 0, 0, 0, 0, 0, 0, 0}
       gather_msb<uint8_t>(block) 1111111100000000 = 0xff00

       \param[in] _block SSE block to collect MSBs from

       \return
       \retval

    */
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

/**
       \brief 16bit specialisation of the MSB collector, give a sse block of (in pseudo code)
       __m128i block = {0x8000, 0x8000, , ... <repeats 6 times> ...}
       this functor would retrieve the MSBs and store it in a std::uint16_t value as
       gather_msb<uint16_t>(block) 1111111100000000 = 0xff00
       block = {0x8000, 0x8000, 0x8000, 0x8000 , 0, 0, 0, 0, 0, 0, 0, 0}
       gather_msb<uint16_t>(block) 1111000000000000 = 0xf000

       \param[in] _block SSE block to collect MSBs from

       \return
       \retval

    */
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
                                                       13   , 15
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

      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {return _block;}
    };

    template<>
    struct shift_left_m128i<unsigned short> {
      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<short> {
      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi16(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<unsigned int> {
      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<int> {
      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi32(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<unsigned long> {

      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi64(_block, num_bits);
      }
    };

    template<>
    struct shift_left_m128i<long> {

      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {
        return  _mm_slli_epi64(_block, num_bits);
      }
    };


    template<>
    struct shift_left_m128i<unsigned char> {

      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {

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

      template <typename size_type>
      __m128i operator()(const __m128i& _block, size_type num_bits) const {

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

    static inline __m128i sse_insert_epi8(const __m128i& _data,
                                          int _value,
                                          int _position){

      switch(_position){
      case 0: return _mm_insert_epi8(_data,_value,0);break;
      case 1: return _mm_insert_epi8(_data,_value,1);break;
      case 2: return _mm_insert_epi8(_data,_value,2);break;
      case 3: return _mm_insert_epi8(_data,_value,3);break;
      case 4: return _mm_insert_epi8(_data,_value,4);break;
      case 5: return _mm_insert_epi8(_data,_value,5);break;
      case 6: return _mm_insert_epi8(_data,_value,6);break;
      case 7: return _mm_insert_epi8(_data,_value,7);break;
      case 8: return _mm_insert_epi8(_data,_value,0);break;
      case 9: return _mm_insert_epi8(_data,_value,1);break;
      case 10: return _mm_insert_epi8(_data,_value,2);break;
      case 11: return _mm_insert_epi8(_data,_value,3);break;
      case 12: return _mm_insert_epi8(_data,_value,4);break;
      case 13: return _mm_insert_epi8(_data,_value,5);break;
      case 14: return _mm_insert_epi8(_data,_value,6);break;
      case 15: return _mm_insert_epi8(_data,_value,7);break;

      default: return _data;break;
      };

    }

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

    static inline __m128i sse_insert_epi32(const __m128i& _data,
                                           int _value,
                                           int _position){

      switch(_position){
      case 0: return _mm_insert_epi32(_data,_value,0);break;
      case 1: return _mm_insert_epi32(_data,_value,1);break;
      case 2: return _mm_insert_epi32(_data,_value,2);break;
      case 3: return _mm_insert_epi32(_data,_value,3);break;
      default: return _data;break;
      };

    }

    template <typename T>
    struct sse_scalar {};
    template <>  struct sse_scalar<std::uint16_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi16( _data,
                                 _value,
                                 _position);
      }
    };
    template <>  struct sse_scalar<std::int16_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi16( _data,
                                 _value,
                                 _position);
      }
    };

    template <>  struct sse_scalar<std::uint32_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi32( _data,
                                 _value,
                                 _position);
      }
    };
    template <>  struct sse_scalar<std::int32_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi32( _data,
                                 _value,
                                 _position);
      }
    };

    template <>  struct sse_scalar<std::uint8_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi8( _data,
                                 _value,
                                 _position);
      }
    };

    template <>  struct sse_scalar<std::int8_t> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi8( _data,
                                 _value,
                                 _position);
      }
    };

    template <>  struct sse_scalar<char> {
      static inline __m128i insert(const __m128i& _data,
                                   int _value,
                                   int _position){
        return sse_insert_epi8( _data,
                                _value,
                                _position);
      }
    };

    template <typename T>
    struct is_8bit : std::false_type {};
    template <>  struct is_8bit<char> : std::true_type {};
    template <>  struct is_8bit<unsigned char> : std::true_type {};

    /**
    \brief function to perform a bitplane extraction using SIMD registers (fixed to SSE4 and lower for now)
       the idea is to extract a bitplane from elements in [_begin, _end) and put the bits into _dst
       example:
       input array (16-bit): 0xffff, 0xffff, 0xffff, 0xffff, 0, 0 , 0, 0,...
       msb per item        : 1       1       1       1       0, 0 , 0, 0,...
       output array (16bit): 11110000, ...

       \param[in] _begin iterator/pointer to beginning of input array
       \param[in] _out   iterator/pointer to 1+ end of input array
       \param[inout] _dst   iterator/pointer to beginning of output array
       \param[in] _precondition   functor to take reference of type __m128i and apply arbitrary operations on it before bits are extracted
       \param[in] _bitplane_offset_from_msb  shift block left by this much before extracting the MSB

       \return
       \retval iterator to 1+ the end element of the output array
    */
    template <typename in_iterator_t, typename out_iterator_t >
    auto simd_collect_single_bitplane(in_iterator_t _begin,
                                      in_iterator_t _end,
                                      out_iterator_t _dst,
                                      int _bitplane_offset_from_msb = 0
      ) -> typename std::enable_if< !(is_8bit<decltype(*_begin)>::value) , out_iterator_t>::type
    {

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[simd_collect_bitplane] iterators for input and output differ!");
      static_assert(std::is_integral<in_value_t>::value, "[simd_collect_bitplane] received value is not integral");

      auto functor = [=](__m128i& _block){ return ;};
      // old precondition START
      // sqeazy::detail::vec_xor<in_value_t> xoring;
      // sqeazy::detail::vec_rotate_left<in_value_t> rotate_left;
      // if(std::numeric_limits<in_value_t>::is_signed)
      //   xoring(&input_block);

      // input_block = rotate_left(&input_block); //
      // old precondition END

      if(sizeof(*_begin)>1)
        return simd_collect_single_bitplane_impl(_begin, _end, _dst, functor, _bitplane_offset_from_msb);
      else
        return simd_collect_single_bitplane_impl_8bit(_begin, _end, _dst, functor, _bitplane_offset_from_msb);
    }

/**
    \brief function to perform a bitplane extraction using SIMD registers (fixed to SSE4 and lower for now)
       the idea is to extract a bitplane from elements in [_begin, _end) and put the bits into _dst
       example:
       input array (16-bit): 0xffff, 0xffff, 0xffff, 0xffff, 0, 0 , 0, 0,...
       msb per item        : 1       1       1       1       0, 0 , 0, 0,...
       output array (16bit): 11110000, ...

       \param[in] _begin iterator/pointer to beginning of input array
       \param[in] _out   iterator/pointer to 1+ end of input array
       \param[inout] _dst   iterator/pointer to beginning of output array
       \param[in] _precondition   functor to take reference of type __m128i and apply arbitrary operations on it before bits are extracted
       \param[in] _bitplane_offset_from_msb  shift block left by this much before extracting the MSB

       \return
       \retval iterator to 1+ the end element of the output array
    */
    template <typename in_iterator_t, typename out_iterator_t, typename functor_t>
    auto simd_collect_single_bitplane(in_iterator_t _begin,
                                      in_iterator_t _end,
                                      out_iterator_t _dst,
                                      functor_t precondition,
                                      int _bitplane_offset_from_msb = 0
      ) -> typename std::enable_if< !(is_8bit<decltype(*_begin)>::value) , out_iterator_t>::type
    {

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[simd_collect_bitplane] iterators for input and output differ!");
      static_assert(std::is_integral<in_value_t>::value, "[simd_collect_bitplane] received value is not integral");

      if(!is_aligned::for_sse(&*_begin)){
        std::cerr << "[sqeazy::detail::simd_collect_single_bitplane_impl] unable to use input for SSE operations due to misalignment\n";
        return _dst;
      }

      if(!is_aligned::for_sse(&*_dst)){
        std::cerr << "[sqeazy::detail::simd_collect_single_bitplane_impl] unable to use output for SSE operations due to misalignment\n";
        return _dst;
      }

      if(sizeof(*_begin)>1)
        return simd_collect_single_bitplane_impl(_begin, _end, _dst, precondition, _bitplane_offset_from_msb);
      else
        return simd_collect_single_bitplane_impl_8bit(_begin, _end, _dst, precondition, _bitplane_offset_from_msb);
    }


    /**
       \brief function to perform a bitplane extraction using SIMD registers (fixed to SSE4 and lower for now)
       the idea is to extract a bitplane from elements in [_begin, _end) and put the bits into _dst
       example:
       input array (16-bit): 0xffff, 0xffff, 0xffff, 0xffff, 0, 0 , 0, 0,...
       msb per item        : 1       1       1       1       0, 0 , 0, 0,...
       output array (16bit): 11110000, ...

       Note this implementation allows only 16-,32-,64-bit integer types

       \param[in] _begin iterator/pointer to beginning of input array
       \param[in] _out   iterator/pointer to 1+ end of input array
       \param[inout] _dst   iterator/pointer to beginning of output array
       \param[in] _precondition   functor to take reference of type __m128i and apply arbitrary operations on it before bits are extracted
       \param[in] _bitplane_offset_from_msb  shift block left by this much before extracting the MSB

       \return
       \retval iterator to 1+ the end element of the output array

    */
    template <typename in_iterator_t, typename out_iterator_t, typename functor_t >
    auto simd_collect_single_bitplane_impl(in_iterator_t _begin,
                                      in_iterator_t _end,
                                      out_iterator_t _dst,
                                      functor_t precondition,
                                      int _bitplane_offset_from_msb = 0
      ) -> typename std::enable_if< !(is_8bit<decltype(*_begin)>::value) , out_iterator_t>::type

    {

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[simd_collect_bitplane] iterators for input and output differ!");
      static_assert(std::is_integral<in_value_t>::value, "[simd_collect_bitplane] received value is not integral");

      static const std::size_t n_bits_per_simd      = sizeof(__m128i)*CHAR_BIT;
      static const std::size_t n_bits_in_value_t	= sizeof(in_value_t)*CHAR_BIT;
      static const std::size_t n_elements_per_simd	= n_bits_per_simd/(n_bits_in_value_t);

      const        std::size_t len                  = std::distance(_begin,_end);
      const        std::size_t n_iterations         = len/n_elements_per_simd;
      const        std::size_t n_collected_bits_per_simd        = n_elements_per_simd;

      /* will be .5 for uint8, 2 for uint16 and 4 for uint32 */
      const        std::size_t n_inner_loops        = n_bits_in_value_t / n_collected_bits_per_simd;



      sqeazy::detail::gather_msb<in_value_t>        collect;
      sqeazy::detail::shift_left_m128i<in_value_t>  shift_left;

      auto output = _dst;
      auto input = _begin;
      std::uint32_t pos = 0;
      __m128i output_block = _mm_set1_epi8(0);

      for(std::size_t i = 0;i<n_iterations;i+=n_inner_loops)//loop through memory
      {

        in_value_t result = 0;

        for(std::size_t l = 0;
            l<n_inner_loops;
            ++l){

          __m128i input_block = _mm_load_si128(reinterpret_cast<const __m128i*>(&*input));

          precondition(input_block);

          input_block = shift_left(input_block,_bitplane_offset_from_msb);
          in_value_t temp = collect(input_block) << (n_bits_in_value_t - 16);

          result |= (temp >> (l*n_collected_bits_per_simd));

          input += n_elements_per_simd;

        }// l filled_segments


        // output_block = sse_insert_epi16(output_block,result,pos);
        output_block = sse_scalar<in_value_t>::insert(output_block,result,pos);
        pos++;

        if(pos > (n_elements_per_simd-1)){//flush to output memory
          _mm_store_si128(reinterpret_cast<__m128i*>(&*output),output_block);
          output_block = _mm_set1_epi8(0);
          output += n_elements_per_simd;
          pos = 0;
        }

      }

      return output;
    }


    /**
       \brief function to perform a bitplane extraction using SIMD registers (fixed to SSE4 and lower for now)
       the idea is to extract a bitplane from elements in [_begin, _end) and put the bits into _dst
       example:
       input array  (8 bit): 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb per item        : 1     1     1     1     1     1     1     1     0, 0 , 0, 0, 0, 0 , 0, 0,...
       output array (8 bit): 11111111, 11111111, 0, 0 ...

       Note this implementation allows only 8-bit integer types

       \param[in] _begin iterator/pointer to beginning of input array
       \param[in] _out   iterator/pointer to 1+ end of input array
       \param[inout] _dst   iterator/pointer to beginning of output array
       \param[in] _precondition   functor to take reference of type __m128i and apply arbitrary operations on it before bits are extracted
       \param[in] _bitplane_offset_from_msb  shift block left by this much before extracting the MSB

       \return
       \retval iterator to 1+ the end element of the output array

    */
    template <typename in_iterator_t, typename out_iterator_t, typename functor_t >
    out_iterator_t simd_collect_single_bitplane_impl_8bit(in_iterator_t _begin,
                                      in_iterator_t _end,
                                      out_iterator_t _dst,
                                      functor_t precondition,
                                      int _bitplane_offset_from_msb = 0
      )

    {

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[simd_collect_bitplane] iterators for input and output differ!");
      static_assert(std::is_integral<in_value_t>::value, "[simd_collect_bitplane] received value is not integral");

      static const std::size_t n_bits_per_simd      = sizeof(__m128i)*CHAR_BIT;
      static const std::size_t n_bits_in_value_t	= sizeof(in_value_t)*CHAR_BIT;
      static const std::size_t n_elements_per_simd	= n_bits_per_simd/(n_bits_in_value_t);

      const        std::size_t len                  = std::distance(_begin,_end);

      sqeazy::detail::gather_msb<in_value_t>        collect;
      sqeazy::detail::shift_left_m128i<in_value_t>  shift_left;

      auto output = _dst;
      std::uint32_t pos = 0;
      __m128i output_block = _mm_set1_epi8(0);

      for(std::size_t i = 0;i<len;i+=n_elements_per_simd)//loop through memory
      {


        __m128i input_block = _mm_load_si128(reinterpret_cast<const __m128i*>(&*(_begin + i)));

        precondition(input_block);

        input_block = shift_left(input_block,_bitplane_offset_from_msb);
        std::uint16_t temp = collect(input_block);

        output_block = sse_scalar<std::uint16_t>::insert(output_block,temp,pos);
        pos++;

        if(pos > 7){//flush to output memory
          _mm_store_si128(reinterpret_cast<__m128i*>(&*output),output_block);
          output_block = _mm_set1_epi8(0);
          output += n_elements_per_simd;
          pos = 0;
        }

      }

      return output;
    }


    /**
       \brief function to perform a full bitplane reshuffle on the input where each bitplane is 1bit wide

       example:
       input array  (8 bit, 128 items): 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0, 0 , 0, 0, 0, 0 , 0, 0,...

       msb   per item                 : 1     1     1     1     1     1     1     1     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+1 per item                 : 1     1     1     1     1     1     1     1     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+2 per item                 : 1     1     1     1     1     1     1     1     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+3 per item                 : 1     1     1     1     1     1     1     1     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+4 per item                 : 0     0     0     0     0     0     0     0     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+5 per item                 : 0     0     0     0     0     0     0     0     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+6 per item                 : 0     0     0     0     0     0     0     0     0, 0 , 0, 0, 0, 0 , 0, 0,...
       msb+7 per item                 : 0     0     0     0     0     0     0     0     0, 0 , 0, 0, 0, 0 , 0, 0,...

       output array (8 bit, 128 items):
       [ 0:15] 11111111, 0, 0, 0 ...
       [16:31] 11111111, 0, 0, 0 ...
       [32:47] 11111111, 0, 0, 0 ...
       [48:63] 11111111, 0, 0, 0 ...
       [64:  ] 0, 0, 0, 0 ...
       [80:  ] 0, 0, 0, 0 ...
       [96:  ] 0, 0, 0, 0 ...
       [112: ] 0, 0, 0, 0 ...

       Note this implementation allows only 8-bit integer types

       \param[in] _begin iterator/pointer to beginning of input array
       \param[in] _end   iterator/pointer to 1+ end of input array
       \param[inout] _dst   iterator/pointer to beginning of output array
       \param[in] _nthreads   number of threads to use

       \return
       \retval iterator to 1+ the end element of the output array

    */
    template <typename in_iterator_t, typename out_iterator_t>
    void simd_segment_broadcast(in_iterator_t _begin,
                                in_iterator_t _end,
                                out_iterator_t _dst,
                                int _nthreads = 1){

      typedef typename std::make_signed<std::size_t>::type loop_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation

      typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_t;
      typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_t;

      static_assert(std::is_same<in_value_t,out_value_t>::value, "[sse_segment_broadcast] received value is not integral");
      static_assert(std::is_integral<in_value_t>::value, "[sse_segment_broadcast] received value is not integral");
      //static_assert((std::is_same<value_t,std::uint16_t>::value || std::is_same<value_t,std::int16_t>::value) && sizeof(value_t) == 2, "[simd_copy] unsigned short != 2");

      static const std::size_t n_bits_per_simd      = sizeof(__m128i)*CHAR_BIT;
      static const std::size_t n_bits_in_value_t	= sizeof(in_value_t)*CHAR_BIT;
      static const std::size_t n_elements_per_simd	= n_bits_per_simd/(n_bits_in_value_t);

      //TODO: the segments are the bitplane chunks that are to be extracted from each value_type
      //      we here assume n_bits_per_plane = 1
      const std::size_t n_segments		= n_bits_in_value_t;
      const loop_size_type n_segments_signed		= n_segments;
      const loop_size_type chunk_size = n_segments_signed/_nthreads;
      assert(chunk_size % _nthreads == 0);

      const std::size_t len		= std::distance(_begin,_end);
      const std::size_t segment_offset	= len/n_segments;
      if(len % n_segments != 0){
        std::cerr << "unable to bitshuffle an array where its size (" << len
                  << ") is incompatible with the number of bitplanes per datum ("
                  << n_segments<< ")\n";
        return;
      }

      const std::size_t n_elements_per_seg	= len/n_bits_in_value_t;

      if(len <= n_elements_per_simd){
        std::cerr << "received iterator range (size "<< len <<") is smaller than SIMD register width ("<< n_elements_per_simd <<")\n";
        return;
      }

      if(n_elements_per_seg > segment_offset)
        return;


      static_assert(std::is_same<in_value_t,out_value_t>::value, "[simd_collect_bitplane] iterators for input and output differ!");
      static_assert(std::is_integral<in_value_t>::value, "[simd_collect_bitplane] received value is not integral");



#pragma omp parallel for                        \
  shared(_begin, _end,_dst)                     \
  num_threads(_nthreads)                        \
  schedule(static,chunk_size)
      for(loop_size_type seg = 0;seg<n_segments_signed;++seg){

        out_iterator_t output = _dst + seg*segment_offset;

        auto functor = [=](__m128i& _block){

          sqeazy::detail::vec_xor<in_value_t> xoring;
          sqeazy::detail::vec_rotate_left<in_value_t> rotate_left;

          if(std::numeric_limits<in_value_t>::is_signed)
            xoring(&_block);

          _block = rotate_left(&_block); //

          return _block;
        };

        simd_collect_single_bitplane(_begin, _end, output, functor, seg);

      } // s segments

    }

  }//detail

}//sqeazy
#endif
