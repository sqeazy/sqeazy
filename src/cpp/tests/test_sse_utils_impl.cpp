#define BOOST_TEST_MODULE TEST_SSE_UTILS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

#include "encoders/sse_utils.hpp"
namespace sqd = sqeazy::detail;

//#include "boost/dynamic_bitset.hpp"


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

  std::vector<std::uint16_t> msb_is_1_16;
  std::vector<std::uint32_t> msb_is_1_32;

  std::vector<std::uint8_t> msb_lsb_both_1;
  std::vector<std::uint8_t> half_msb_is_1;
  std::vector<std::uint8_t> half_msb_lsb_both_1;

  sse_fixture():
    msb_is_1(size,0x80),//10000000
    msb_is_1_16(128/16,0x8000),//10000000
    msb_is_1_32(128/32,0x80000000),//10000000
    msb_lsb_both_1(size,0x90),//10010000
    half_msb_is_1(size,0x80),//10000000
    half_msb_lsb_both_1(size,0x90)//10010000
  {
    std::fill(half_msb_is_1.begin(), half_msb_is_1.begin() + size/2,0);
    std::fill(half_msb_lsb_both_1.begin(), half_msb_lsb_both_1.begin() + size/2,0);
  }



};

BOOST_AUTO_TEST_CASE( check_movemask ){

  int mask = _mm_movemask_epi8(_mm_set1_epi8(0xff));
  BOOST_CHECK_EQUAL(pop(mask),16);

  mask = _mm_movemask_epi8(_mm_set_epi8(0x0,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
                    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80));
  BOOST_CHECK_EQUAL(pop(mask),15);
  BOOST_CHECK_EQUAL(mask,0x7fff);

  mask = _mm_movemask_epi8(_mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
                    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x0));
  BOOST_CHECK_EQUAL(pop(mask),15);
  BOOST_CHECK_EQUAL(mask,0xfffe);

  mask = _mm_movemask_epi8(_mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
                    0,0,0,0,0,0,0,0));
  BOOST_CHECK_EQUAL(pop(mask),8);
  BOOST_CHECK_EQUAL(mask,0xff00);
}


BOOST_FIXTURE_TEST_SUITE( to_32bit , sse_fixture )

BOOST_AUTO_TEST_CASE( from_16_to_4_bits ){


  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1[0]));
  int icount = 0;
  for(auto & el: msb_is_1)
    icount += pop(el);

  __m128i received = sqeazy::detail::to_32bit_field<std::uint8_t>::conversion(input);
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

  __m128i received = sqeazy::detail::to_32bit_field<std::uint16_t>::conversion(input);
  int msb_mask = _mm_movemask_epi8(received);
  int rcount = pop(msb_mask);

  BOOST_CHECK_LT(rcount,icount);
  BOOST_CHECK_EQUAL(rcount,4);
}

BOOST_AUTO_TEST_CASE( gather_msb_8 ){


  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1[0]));

  sqeazy::detail::gather_msb<std::uint8_t> op;

  auto res = op(input);
  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_8_order_right ){

  msb_is_1[msb_is_1.size()-1] = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));

  sqeazy::detail::gather_msb<std::uint8_t> op;
  auto res = op(input);
  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1.size()-1);
  BOOST_CHECK_EQUAL(received.test(1),true);
  BOOST_CHECK_EQUAL(received.test(14),true);

  BOOST_CHECK_EQUAL(received.test(0),false);//LSB
  BOOST_CHECK_EQUAL(received.test(15),true);//MSB
}

BOOST_AUTO_TEST_CASE( gather_msb_16 ){


  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  auto res = op(input);
  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_16.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_16_order_right ){

  msb_is_1_16.back() = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  auto res = op(input);
  BOOST_CHECK_EQUAL(res,0xfe00 );

  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_16.size()-1);

  BOOST_CHECK_EQUAL(received.test(0),false);
  BOOST_CHECK_EQUAL(received.test(7),false);
  BOOST_CHECK_EQUAL(received.test(8),false);
  BOOST_CHECK_EQUAL(received.test(9),true);
  BOOST_CHECK_EQUAL(received.test(15),true);

}

BOOST_AUTO_TEST_CASE( gather_msb_16_pattern_right ){

  msb_is_1_16 = {0x8000, 0, 0, 0x8000, 0x8000, 0x8000, 0, 0 };
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  auto res = op(input);
  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),4u);

  for(std::uint32_t i = 0;i<msb_is_1_16.size();++i)
    BOOST_CHECK_MESSAGE(received.test(15-i) == (msb_is_1_16[i] > 0),
            i << ": bit " << received.test(16-i-1) << " versus input " << msb_is_1_16[i] );
}

BOOST_AUTO_TEST_CASE( gather_msb_32 ){


  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_32[0]));

  sqeazy::detail::gather_msb<std::uint32_t> op;
  auto res = op(input);
  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_32.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_32_order_right ){

  msb_is_1_32.back() = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_32[0]));

  sqeazy::detail::gather_msb<std::uint32_t> op;
  auto res = op(input);

  BOOST_CHECK_EQUAL(res,0xe000);

  std::bitset<16> received(res);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_32.size()-1);
  BOOST_CHECK_EQUAL(received.test(15),true);
  BOOST_CHECK_EQUAL(received.test(14),true);
  BOOST_CHECK_EQUAL(received.test(13),true);
  for(int r = 12;r>-1;--r)
    BOOST_CHECK_EQUAL(received.test(r),false);

}

BOOST_AUTO_TEST_SUITE_END()
