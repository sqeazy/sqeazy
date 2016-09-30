#define BOOST_TEST_MODULE TEST_SSE_UTILS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include "encoders/sse_utils.hpp"


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
  std::bitset<16> received = op(input);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_8_order_right ){

  msb_is_1[msb_is_1.size()-1] = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));
  __m128i exp = _mm_set_epi8(0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
			     0x80,0x80,0x80,0x80,0x80,0x80,0x80,0);

  std::fill(msb_lsb_both_1.begin(), msb_lsb_both_1.end(),0);
  _mm_store_si128(reinterpret_cast<__m128i*>(msb_lsb_both_1.data()), input);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(msb_is_1.begin(), msb_is_1.end(),
				msb_lsb_both_1.begin(), msb_lsb_both_1.end());
  
  sqeazy::detail::gather_msb<std::uint8_t> op;
  std::bitset<16> received = op(exp);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1.size()-1);
  BOOST_CHECK_EQUAL(received.test(15),true);
  BOOST_CHECK_EQUAL(received.test(14),true);
  BOOST_CHECK_EQUAL(received.test(0),false);
}

BOOST_AUTO_TEST_CASE( gather_msb_16 ){

  
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  std::bitset<16> received = op(input);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_16.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_16_order_right ){

  msb_is_1_16.back() = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  std::bitset<16> received = op(input);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_16.size()-1);
  BOOST_CHECK_EQUAL(received.test(8),false);
  BOOST_CHECK_EQUAL(received.test(9),true);
}

BOOST_AUTO_TEST_CASE( gather_msb_16_pattern_right ){

  msb_is_1_16 = {0x8000, 0, 0, 0x8000, 0x8000, 0x8000, 0, 0 };
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  
  std::bitset<16> received = op(input);

  BOOST_CHECK_EQUAL(received.count(),4);
  
  for(std::uint32_t i = 0;i<msb_is_1_16.size();++i)
    BOOST_CHECK_MESSAGE(received.test(16-i-1) == (msb_is_1_16[i] > 0),
			i << ": bit " << received.test(16-i-1) << " versus input " << msb_is_1_16[i] );
}

BOOST_AUTO_TEST_CASE( gather_msb_32 ){

  
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_32[0]));

  sqeazy::detail::gather_msb<std::uint32_t> op;
  std::bitset<16> received = op(input);

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_32.size());
}

BOOST_AUTO_TEST_SUITE_END()

template<typename in_type>
struct bitplane_fixture {

  typedef in_type type;
  
  std::vector<in_type> input	;
  std::vector<in_type> output	;
  std::vector<in_type> expected	;
  __m128i input_block;
  
  bitplane_fixture():
    input(),
    output(),
    expected(),
    input_block()
  {
    
    input.resize(32/sizeof(in_type));
    std::fill(input.begin(), input.end(),
	      1 << ((sizeof(in_type)*CHAR_BIT) -1 )
	      );
    
    output.resize(input.size());
    expected.resize(input.size());

    for(std::uint32_t i = 0;i < (input.size()/(sizeof(in_type)*CHAR_BIT)); ++i)
      expected[i] = ~(in_type(0));

    input_block = _mm_load_si128(reinterpret_cast<const __m128i*>(input.data()));
  }
  
};

namespace sqd = sqeazy::detail;

using bitplane_fixture_16 = bitplane_fixture<std::uint16_t>;
using bitplane_fixture_8 = bitplane_fixture<std::uint8_t>;

BOOST_FIXTURE_TEST_SUITE( with_16bit , bitplane_fixture_16 )

BOOST_AUTO_TEST_CASE( construct ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_NE(instance.segments.empty(),true);
  BOOST_CHECK_NE(instance.segments[0].any(),true);
  
}


BOOST_AUTO_TEST_CASE( gather_msb_range ){
  sqd::bitshuffle<type> instance;
  
  std::bitset<128> result = instance.gather_msb_range(input_block, 1);

  BOOST_CHECK_NE(result.any(),false);

  result = instance.gather_msb_range(input_block, 2);
  
  BOOST_CHECK_NE(result.any(),true);
  
}


BOOST_AUTO_TEST_CASE( fill_anything ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_EQUAL(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.full(),false);
  BOOST_CHECK_EQUAL(instance.any(),false);

  auto consumed = instance.consume(input.begin()+input.size()-1,
				   input.end());
  std::size_t diff = consumed-(input.begin()+input.size()-1);
  BOOST_CHECK_EQUAL(diff,0);
  
  
  consumed = instance.consume(input.begin(),
			      input.end());
  
  BOOST_CHECK_NE(consumed-input.begin(),0);
  BOOST_CHECK(consumed == input.end());
  
  BOOST_CHECK_NE(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.any(),true);

  BOOST_CHECK_EQUAL(instance.segments[0].any(),true);
  BOOST_CHECK_EQUAL(instance.segments[0].count(),input.size());
}



BOOST_AUTO_TEST_SUITE_END()
