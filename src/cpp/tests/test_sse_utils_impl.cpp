#define BOOST_TEST_MODULE TEST_SSE_UTILS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

#include "encoders/sse_utils.hpp"

#include "boost/dynamic_bitset.hpp"


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



BOOST_FIXTURE_TEST_SUITE( movemask_sanity_checks , sse_fixture )

BOOST_AUTO_TEST_CASE( from_char_buffer ){

  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));
  int plain_res = _mm_movemask_epi8(input);
  boost::dynamic_bitset<std::uint8_t> plain(32,plain_res);

  
  BOOST_CHECK_EQUAL(plain.any(),true);
  BOOST_CHECK_EQUAL(plain.test(0),true);
  BOOST_CHECK_EQUAL(plain.test(1),true);
  BOOST_CHECK_EQUAL(plain.test(15),true);
  BOOST_CHECK_EQUAL(plain.test(16),false);
  BOOST_CHECK_EQUAL(plain.test(31),false);
  
  
  msb_is_1[msb_is_1.size()-1] = 0;
  input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));
  int asymm_res = _mm_movemask_epi8(input);
  boost::dynamic_bitset<std::uint8_t> asymm(32,asymm_res);
    
  BOOST_CHECK_EQUAL(asymm.any(),true);
  BOOST_CHECK_EQUAL(asymm.count(),16-1);
  
  BOOST_CHECK_EQUAL(asymm.test(0),true);
  BOOST_CHECK_EQUAL(asymm.test(1),true);
  BOOST_CHECK_EQUAL(asymm.test(15),false);
  BOOST_CHECK_EQUAL(asymm.test(16),false);
  BOOST_CHECK_EQUAL(asymm.test(31),false);

}


BOOST_AUTO_TEST_CASE( from_asymm_char_buffer ){

  //						    v MSB from field at offset 15
  //MSB pattern to be generated { 0100 1111 1111 1100 }
  //                              ^ MSB from field at offset 0
  msb_is_1[0] = 0;
  msb_is_1[2] = 0;
  msb_is_1[3] = 0;

  msb_is_1[14] = 0;
  msb_is_1[15] = 0;

  
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));

  //reverse m128
  const __m128i mask_to_reverse =  _mm_set_epi8(0,
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
	
  input = _mm_shuffle_epi8(input, mask_to_reverse);
  int plain_res = _mm_movemask_epi8(input);

  BOOST_CHECK_EQUAL(plain_res,0x4ffc);
  
  boost::dynamic_bitset<std::uint8_t> plain(32,plain_res);

  BOOST_CHECK_EQUAL(plain.count(),16-5);
  BOOST_CHECK_EQUAL(plain.any(),true);
  
  BOOST_CHECK_EQUAL(plain.test(0),false);
  BOOST_CHECK_EQUAL(plain.test(1),false);
  BOOST_CHECK_EQUAL(plain.test(2),true);
  BOOST_CHECK_EQUAL(plain.test(3),true);

  BOOST_CHECK_EQUAL(plain.test(4),true);
  BOOST_CHECK_EQUAL(plain.test(5),true);
  BOOST_CHECK_EQUAL(plain.test(6),true);
  BOOST_CHECK_EQUAL(plain.test(7),true);

  BOOST_CHECK_EQUAL(plain.test( 8),true);
  BOOST_CHECK_EQUAL(plain.test( 9),true);
  BOOST_CHECK_EQUAL(plain.test(10),true);
  BOOST_CHECK_EQUAL(plain.test(11),true);

  BOOST_CHECK_EQUAL(plain.test(12),false);
  BOOST_CHECK_EQUAL(plain.test(13),false);
  BOOST_CHECK_EQUAL(plain.test(14),true);
  BOOST_CHECK_EQUAL(plain.test(15),false);
  
  BOOST_CHECK_EQUAL(plain.test(16),false);
  BOOST_CHECK_EQUAL(plain.test(31),false);

  std::array<std::uint8_t,4> static_recasted;
  boost::to_block_range(plain, static_recasted.rbegin());

  
  BOOST_CHECK_EQUAL(static_recasted[0],0);
  BOOST_CHECK_EQUAL(static_recasted[1],0);
  BOOST_CHECK_EQUAL(static_recasted[2],0x4f);
  BOOST_CHECK_EQUAL(static_recasted[3],0xfc);
  
  
}
BOOST_AUTO_TEST_SUITE_END()


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
  boost::dynamic_bitset<std::uint8_t> received(16,res);  

  BOOST_CHECK_EQUAL(received.count(),msb_is_1.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_8_order_right ){

  msb_is_1[msb_is_1.size()-1] = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(msb_is_1.data()));
  
  sqeazy::detail::gather_msb<std::uint8_t> op;
  auto res = op(input);
  boost::dynamic_bitset<std::uint8_t> received(16,res);  

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
  boost::dynamic_bitset<std::uint16_t> received(16,res);  

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_16.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_16_order_right ){

  msb_is_1_16.back() = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_16[0]));

  sqeazy::detail::gather_msb<std::uint16_t> op;
  auto res = op(input);
  BOOST_CHECK_EQUAL(res,0xfe00 );
  
  boost::dynamic_bitset<std::uint16_t> received(16,res); 

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
  boost::dynamic_bitset<std::uint16_t> received(16,res); 

  BOOST_CHECK_EQUAL(received.count(),4);
  
  for(std::uint32_t i = 0;i<msb_is_1_16.size();++i)
    BOOST_CHECK_MESSAGE(received.test(15-i) == (msb_is_1_16[i] > 0),
			i << ": bit " << received.test(16-i-1) << " versus input " << msb_is_1_16[i] );
}

BOOST_AUTO_TEST_CASE( gather_msb_32 ){

  
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_32[0]));

  sqeazy::detail::gather_msb<std::uint32_t> op;
  auto res = op(input);
  boost::dynamic_bitset<std::uint16_t> received(16,res); 

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_32.size());
}

BOOST_AUTO_TEST_CASE( gather_msb_32_order_right ){

  msb_is_1_32.back() = 0;
  __m128i input = _mm_load_si128(reinterpret_cast<const __m128i*>(&msb_is_1_32[0]));

  sqeazy::detail::gather_msb<std::uint32_t> op;
  auto res = op(input);

  BOOST_CHECK_EQUAL(res,0xe000);
  
  boost::dynamic_bitset<std::uint16_t> received(16,res); 

  BOOST_CHECK_EQUAL(received.count(),msb_is_1_32.size()-1);
  BOOST_CHECK_EQUAL(received.test(15),true);
  BOOST_CHECK_EQUAL(received.test(14),true);
  BOOST_CHECK_EQUAL(received.test(13),true);
  for(int r = 12;r>-1;--r)
    BOOST_CHECK_EQUAL(received.test(r),false);
  
}

BOOST_AUTO_TEST_SUITE_END()

template<typename in_type>
struct single_bitplane_fixture {

  typedef in_type type;
  

  std::vector<in_type> output	;

  std::vector<in_type> input_2p1	;
  std::vector<in_type> expected_2p1	;

  std::vector<in_type> input_msb_1	;
  std::vector<in_type> expected_msb_1	;


  __m128i input_block;
  
  single_bitplane_fixture():
    output(),
    input_msb_1(),
    expected_msb_1(),
    input_2p1(),
    expected_2p1(),
    input_block()
  {

    static const std::size_t simd_width = 128;
    std::size_t n_elements = sizeof(in_type)*CHAR_BIT*simd_width/(sizeof(in_type)*CHAR_BIT);
    
    input_msb_1.resize(n_elements);
    expected_msb_1.resize(n_elements);
    const in_type msb_1 = 1 << ((sizeof(in_type)*CHAR_BIT) -1 );
    std::fill(input_msb_1.begin(), input_msb_1.end(),
	      msb_1
	      );

    input_2p1.resize(n_elements);
    expected_2p1.resize(n_elements);
    const in_type val_2p1 = (3 << ((sizeof(in_type)*CHAR_BIT) -2 )) | 1;
    std::fill(input_2p1.begin(), input_2p1.end(),
	      val_2p1
	      );
    
    output.resize(input_msb_1.size());


    for(std::uint32_t i = 0;i < (input_msb_1.size()/(sizeof(in_type)*CHAR_BIT)); ++i)
      expected_msb_1[i] = ~(in_type(0));

    for(std::uint32_t i = 0;i < 2*(input_2p1.size()/(sizeof(in_type)*CHAR_BIT)); ++i){
      expected_2p1[i] = ~(in_type(0));
    }

    for(std::uint32_t i = expected_2p1.size()-(input_2p1.size()/(sizeof(in_type)*CHAR_BIT));i < expected_2p1.size(); ++i){
      expected_2p1[i] = ~(in_type(0));
    }

    
    input_block = _mm_load_si128(reinterpret_cast<const __m128i*>(input_msb_1.data()));
  }
  
};

namespace sqd = sqeazy::detail;

using single_bitplane_fixture_16 = single_bitplane_fixture<std::uint16_t>;
using single_bitplane_fixture_8 = single_bitplane_fixture<std::uint8_t>;

BOOST_FIXTURE_TEST_SUITE( with_16bit , single_bitplane_fixture_16 )

BOOST_AUTO_TEST_CASE( construct ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_NE(instance.segments.empty(),true);
  BOOST_CHECK_NE(instance.segments[0].any(),true);
  
}


BOOST_AUTO_TEST_CASE( gather_msb_range ){
  sqd::bitshuffle<type> instance;
  
  boost::dynamic_bitset<type> result = instance.gather_msb_range(input_block, 1);

  BOOST_CHECK_NE(result.any(),false);

  result = instance.gather_msb_range(input_block, 2);
  
  BOOST_CHECK_NE(result.any(),true);
  
}


BOOST_AUTO_TEST_CASE( fill_anything ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_EQUAL(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.full(),false);
  BOOST_CHECK_EQUAL(instance.any(),false);

  auto consumed = instance.consume(input_msb_1.begin()+input_msb_1.size()-1,
				   input_msb_1.end());
  std::size_t diff = consumed-(input_msb_1.begin()+input_msb_1.size()-1);
  BOOST_CHECK_EQUAL(diff,0);
  
  
  consumed = instance.consume(input_msb_1.begin(),
			      input_msb_1.end());
  
  BOOST_CHECK_NE(consumed-input_msb_1.begin(),0);
  BOOST_CHECK(consumed == input_msb_1.end());
  
  BOOST_CHECK_NE(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.any(),true);

  BOOST_CHECK_EQUAL(instance.segments[0].any(),true);
  BOOST_CHECK_EQUAL(instance.segments[0].count(),input_msb_1.size());

  for(std::uint32_t i = 1;i<instance.segments.size();++i){
    BOOST_CHECK_EQUAL(instance.segments[i].any(),false);
  }
    
}


BOOST_AUTO_TEST_CASE( consume_and_write ){

  sqd::bitshuffle<type> instance;
  auto consumed = instance.consume(input_msb_1.begin(),
				   input_msb_1.end());
  
  BOOST_CHECK_NE(consumed-input_msb_1.begin(),0);

  auto old_output = output;
  auto written = instance.write_segments(output.begin(),
					 output.end()
					 );

  BOOST_CHECK_NE(written-output.begin(),0);
  BOOST_CHECK_EQUAL(written-output.end(),0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(output.begin(), output.end(),
				  expected_msb_1.begin(), expected_msb_1.end());
  BOOST_CHECK_NE(std::equal(output.begin(), output.end(), old_output.begin()),true);
  
}

BOOST_AUTO_TEST_CASE( consume_and_write_2p1 ){

  sqd::bitshuffle<type> instance;
  auto consumed = instance.consume(input_2p1.begin(),
				   input_2p1.end());
  
  BOOST_CHECK_NE(consumed-input_2p1.begin(),0);

  auto old_output = output;
  auto written = instance.write_segments(output.begin(),
					 output.end()
					 );

  BOOST_CHECK_NE(written-output.begin(),0);
  BOOST_CHECK_EQUAL(written-output.end(),0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(output.begin(), output.end(),
				  expected_2p1.begin(), expected_2p1.end());
  BOOST_CHECK_NE(std::equal(output.begin(), output.end(), old_output.begin()),true);
  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( with_8bit , single_bitplane_fixture_8 )

BOOST_AUTO_TEST_CASE( construct ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_NE(instance.segments.empty(),true);
  BOOST_CHECK_NE(instance.segments[0].any(),true);
  
}



BOOST_AUTO_TEST_CASE( fill_anything ){

  sqd::bitshuffle<type> instance;
  
  BOOST_CHECK_EQUAL(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.full(),false);
  BOOST_CHECK_EQUAL(instance.any(),false);

  auto consumed = instance.consume(input_msb_1.begin()+input_msb_1.size()-1,
				   input_msb_1.end());
  std::size_t diff = consumed-(input_msb_1.begin()+input_msb_1.size()-1);
  BOOST_CHECK_EQUAL(diff,0);
  
  
  consumed = instance.consume(input_msb_1.begin(),
			      input_msb_1.end());
  
  BOOST_CHECK_NE(consumed-input_msb_1.begin(),0);
  BOOST_CHECK(consumed == input_msb_1.end());
  
  BOOST_CHECK_NE(instance.empty(),true);
  BOOST_CHECK_EQUAL(instance.any(),true);

  BOOST_CHECK_EQUAL(instance.segments[0].any(),true);
  BOOST_CHECK_EQUAL(instance.segments[0].count(),input_msb_1.size());

  for(std::uint32_t i = 1;i<instance.segments.size();++i){
    BOOST_CHECK_EQUAL(instance.segments[i].any(),false);
  }
    
}

BOOST_AUTO_TEST_CASE( consume_and_write ){

  sqd::bitshuffle<type> instance;
  auto consumed = instance.consume(input_msb_1.begin(),
				   input_msb_1.end());
  
  BOOST_CHECK_NE(consumed-input_msb_1.begin(),0);

  auto old_output = output;
  auto written = instance.write_segments(output.begin(),
					 output.end()
					 );

  BOOST_CHECK_NE(written-output.begin(),0);
  BOOST_CHECK_EQUAL(written-output.end(),0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(output.begin(), output.end(),
				  expected_msb_1.begin(), expected_msb_1.end());
  BOOST_CHECK_NE(std::equal(output.begin(), output.end(), old_output.begin()),true);
  
}

BOOST_AUTO_TEST_CASE( consume_and_write_2p1 ){

  sqd::bitshuffle<type> instance;
  auto consumed = instance.consume(input_2p1.begin(),
				   input_2p1.end());
  
  BOOST_CHECK_NE(consumed-input_2p1.begin(),0);

  auto old_output = output;
  auto written = instance.write_segments(output.begin(),
					 output.end()
					 );

  BOOST_CHECK_NE(written-output.begin(),0);
  BOOST_CHECK_EQUAL(written-output.end(),0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(output.begin(), output.end(),
				  expected_2p1.begin(), expected_2p1.end());
  BOOST_CHECK_NE(std::equal(output.begin(), output.end(), old_output.begin()),true);
  
}

BOOST_AUTO_TEST_SUITE_END()
