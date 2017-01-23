#define BOOST_TEST_MODULE TEST_SHIFT_BY_INTRINSICS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <numeric> // for accumulate
#include <iterator> // for ostream_iterator
#include "encoders/sse_utils.hpp"

#include "sse_test_utils.hpp"
//#include "bitplane_reorder_detail.hpp"


#include <xmmintrin.h>
#include <smmintrin.h>


typedef const_anyvalue_fixture<8, 2> default_128bit_fixture;
typedef const_anyvalue_fixture<16, 2, unsigned char> default_128bit_ofuchar_fixture;
typedef const_anyvalue_fixture<16, 2, char> default_128bit_ofchar_fixture;
typedef const_anyvalue_fixture<4, 2, unsigned> default_128bit_ofunsigned_fixture;
typedef const_anyvalue_fixture<(1 << 8), 2> default_cv_fixture;


BOOST_FIXTURE_TEST_SUITE( shift_16_bits, default_cv_fixture )

BOOST_AUTO_TEST_CASE( shift_right_by_one ){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    v_in = _mm_srli_epi16(v_in,1);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);

    BOOST_REQUIRE(output[0]!=input[0]);

    try{
    BOOST_REQUIRE(output[0]==1);
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
      BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0u)==output.size());
    }
  }

BOOST_AUTO_TEST_CASE( shift_left_by_one ){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    v_in = _mm_slli_epi16(v_in,1);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);

    BOOST_REQUIRE(output[0]!=input[0]);

    try{
    BOOST_REQUIRE(output[0]==4);
    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0u)==4*8);
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    }
  }
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( shift_16_bits_any_value, default_128bit_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){

  static const __m128i v_const = _mm_set1_epi16(2);

  sqeazy::detail::shift_left_m128i<unsigned short> shifter;
  __m128i result = shifter(v_const,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == (2 << 1));
}

BOOST_AUTO_TEST_CASE( shift_msb_left_by_constant ){

  static const __m128i v_msb = _mm_set1_epi16(0x8000);

  sqeazy::detail::shift_left_m128i<unsigned short> shifter;
  __m128i result = shifter(v_msb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == 0);
}


BOOST_AUTO_TEST_CASE( shift_right_by_constant ){


  static const __m128i v_const = _mm_set1_epi16(2);

  sqeazy::detail::shift_right_m128i<unsigned short> shifter;
  __m128i result = shifter(v_const,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == (2 >> 1));
}

BOOST_AUTO_TEST_CASE( shift_lsb_right_by_constant ){
  static const __m128i v_lsb = _mm_set1_epi16(1);

  sqeazy::detail::shift_right_m128i<unsigned short> shifter;
  __m128i result = shifter(v_lsb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  BOOST_REQUIRE(output[0] == 0);

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( shift_8_bits, default_128bit_ofuchar_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){


  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

  sqeazy::detail::shift_left_m128i<unsigned char> shifter;
  __m128i result = shifter(v_input,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  for (unsigned i = 0; i < input.size()/2; ++i)
    {
      try{

    BOOST_REQUIRE(output[i] == i << 1);

      }catch(...){
    BOOST_TEST_MESSAGE("[shift-left-m128i-uchar] item " << i << " malformed");
    throw;
      }
    }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

  sqeazy::detail::shift_left_m128i<unsigned char> shifter;
  static const __m128i v_msb = _mm_set1_epi8(0x80);
  __m128i result = shifter(v_msb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  for (unsigned i = 0; i < input.size(); ++i)
    {
      try{
    BOOST_REQUIRE(output[i] == (unsigned char)0);
      }catch(...){
    BOOST_TEST_MESSAGE("[shift-left-m128i-uchar] item " << i << " malformed");
    throw;
      }
    }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi8(1);

    sqeazy::detail::shift_right_m128i<unsigned char> shifter;

    __m128i result = shifter(v_lsb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (unsigned char)0);
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-uchar] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    sqeazy::detail::shift_right_m128i<unsigned char> shifter;

    __m128i result = shifter(v_input,1);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (unsigned char)(i >> 1));
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-uchar] item " << i << " malformed");
      throw;
    }
      }

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( shift_8_bits_signed, default_128bit_ofchar_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){

  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

    sqeazy::detail::shift_left_m128i<char> shifter;
    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

    for (unsigned i = 0; i < input.size()/2; ++i)
      {
    try{

      BOOST_REQUIRE(output[i] == char(i << 1));

    }catch(...){
      BOOST_TEST_MESSAGE("[shift-left-m128i-char] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

    static const __m128i v_msb = _mm_set1_epi8(0x80);

    sqeazy::detail::shift_left_m128i<char> shifter;

    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (char)0);
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-left-m128i-char] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi8(1);

    sqeazy::detail::shift_right_m128i<char> shifter;

    __m128i result = shifter(v_lsb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (char)0);
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-char] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    sqeazy::detail::shift_right_m128i<char> shifter;

    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (char)(i >> 1));
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-char] item " << i << " malformed");
      throw;
    }
      }

}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( shift_32_bits, default_128bit_ofunsigned_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){

  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

    sqeazy::detail::shift_left_m128i<unsigned> shifter;
    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

    for (unsigned i = 0; i < input.size()/2; ++i)
      {
    try{

      BOOST_REQUIRE(output[i] == i << 1);

    }catch(...){
      BOOST_TEST_MESSAGE("[shift-left-m128i-unsigned] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

    static const __m128i v_msb = _mm_set1_epi8(0x8000);

    sqeazy::detail::shift_left_m128i<unsigned> shifter;

    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (unsigned)0);
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-left-m128i-unsigned] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi32(1);

    sqeazy::detail::shift_right_m128i<unsigned> shifter;

    __m128i result = shifter(v_lsb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (unsigned)0);
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-unsigned] item " << i << " malformed");
      throw;
    }
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    sqeazy::detail::shift_right_m128i<unsigned> shifter;

    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (unsigned i = 0; i < input.size(); ++i)
      {
    try{
      BOOST_REQUIRE(output[i] == (unsigned)(i >> 1));
    }catch(...){
      BOOST_TEST_MESSAGE("[shift-right-m128i-unsigned] item " << i << " malformed");
      throw;
    }
      }

}
BOOST_AUTO_TEST_SUITE_END()
