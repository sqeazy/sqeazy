#define BOOST_TEST_MODULE TEST_SSE_UTILS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include "sse_utils.hpp"


int pop(unsigned x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

struct sse_fixture {

  
  static const int size = 128/8;
  std::vector<std::uint8_t> msb_is_1;
  std::vector<std::uint8_t> msb_lsb_both_1;
  std::vector<std::uint8_t> half_msb_is_1;
  std::vector<std::uint8_t> half_msb_lsb_both_1;
  
  sse_fixture():
    msb_is_1(size,0x80),
    msb_lsb_both_1(size,0x90),
    half_msb_is_1(size,0x80),
    half_msb_lsb_both_1(size,0x90)
  {
    std::fill(half_msb_is_1.begin(), half_msb_is_1.begin() + size/2,0);
    std::fill(half_msb_lsb_both_1.begin(), half_msb_lsb_both_1.begin() + size/2,0);
  }
  
    

};


BOOST_FIXTURE_TEST_SUITE( to_32bit , sse_fixture )

BOOST_AUTO_TEST_CASE( from_16_to_4_bits ){


  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1[0]));
  int icount = 0;
  for(auto & el: msb_is_1)
    icount += pop(el);
    
  __m128i received = to_32bit_field<std::uint8_t>::conversion(input);
  int msb_mask = _mm_movemask_epi8(received);
  int rcount = pop(msb_mask);

  BOOST_CHECK_LT(rcount,icount);
  BOOST_CHECK_EQUAL(rcount,4);
}

BOOST_AUTO_TEST_CASE( from_8_to_4_bits ){

  for(int id = msb_is_1.size()-1;id>=0;id-=2)
    msb_is_1[id] = 0;
  
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1[0]));
  
  int icount = 0;
  for(auto & el: msb_is_1)
    icount += pop(el);
    
  __m128i received = to_32bit_field<std::uint16_t>::conversion(input);
  int msb_mask = _mm_movemask_epi8(received);
  int rcount = pop(msb_mask);

  BOOST_CHECK_LT(rcount,icount);
  BOOST_CHECK_EQUAL(rcount,4);
}

BOOST_AUTO_TEST_SUITE_END()
