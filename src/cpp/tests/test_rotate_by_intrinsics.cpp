#define BOOST_TEST_MODULE TEST_ROTATE_BY_INTRINSICS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
//#include "encoders/sqeazy_impl.hpp"
#include "sse_test_utils.hpp"

#include "encoders/sse_utils.hpp"
#include "encoders/scalar_utils.hpp"
#include "encoders/bitplane_reorder_sse.hpp"



typedef const_anyvalue_fixture<128, 0> null_fixture; 
typedef const_anyvalue_fixture<8, 2> default_128bit_fixture; 
typedef const_anyvalue_fixture<16, 2, unsigned char> default_128bit_ofuchar_fixture;
typedef const_anyvalue_fixture<16, 2, char> default_128bit_ofchar_fixture;
typedef const_anyvalue_fixture<4, 2, unsigned> default_128bit_ofunsigned_fixture;
typedef const_anyvalue_fixture<(1 << 8), 2> default_cv_fixture; 

BOOST_FIXTURE_TEST_SUITE( const_8_bit_array, default_128bit_ofuchar_fixture )
BOOST_AUTO_TEST_CASE( rotate_left_by_one ){

  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

  sqeazy::detail::vec_rotate_left<unsigned char> rotate;
  v_in = rotate(&v_in);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
   
  BOOST_REQUIRE(output[0]!=input[0]);
    
  try{
    BOOST_REQUIRE(output[0]==sqeazy::detail::rotate_left<1>(input[0]));

    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0u)==4*output.size());
  }
  catch(...){
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
  }
}


BOOST_AUTO_TEST_CASE( rotate_left_by_two_and_cycle ){


  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

  sqeazy::detail::vec_rotate_left<unsigned char> rotate(2);
  v_in = rotate(&v_in);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
  BOOST_REQUIRE(output[0]!=input[0]);
    
  try{
    BOOST_REQUIRE(output[0]==sqeazy::detail::rotate_left<2>(input[0]));
    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),(unsigned char)0)==sqeazy::detail::rotate_left<2>(input[0])*output.size());
  }
  catch(...){
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
  }
}

BOOST_AUTO_TEST_CASE( rotate_left_by_two_and_flip ){


  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = 0x80;
    }
    
  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));
  sqeazy::detail::vec_rotate_left<unsigned char> rotate(1);
  v_in = rotate(&v_in);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    
  BOOST_REQUIRE(output[0] == 1);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( const_16_bit_array, default_128bit_fixture )
BOOST_AUTO_TEST_CASE( rotate_left_by_one ){

  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

  sqeazy::detail::vec_rotate_left<unsigned short> rotate;
  v_in = rotate(&v_in);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
   
  BOOST_REQUIRE(output[0]!=input[0]);
    
  try{
    BOOST_REQUIRE(output[0]==sqeazy::detail::rotate_left<1>(input[0]));

    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0u)==4u*output.size());
  }
  catch(...){
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
  }
}

BOOST_AUTO_TEST_CASE( rotate_left_by_two_and_cycle ){


  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

  sqeazy::detail::vec_rotate_left<unsigned short> rotate(2);
  v_in = rotate(&v_in);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
  BOOST_REQUIRE(output[0]!=input[0]);
    
  try{
    BOOST_REQUIRE(output[0]==sqeazy::detail::rotate_left<2>(input[0]));
    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),(unsigned short)0)==sqeazy::detail::rotate_left<2>(input[0])*output.size());
  }
  catch(...){
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
  }
}

BOOST_AUTO_TEST_CASE( rotate_left_and_flip ){


  for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = 0x8000;
    }
    
  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));
  sqeazy::detail::vec_rotate_left<unsigned short> rotate(1);
  v_in = rotate(&v_in);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    
  BOOST_REQUIRE(output[0] == 1);
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( variable_16_bit_array, null_fixture )
BOOST_AUTO_TEST_CASE( rotate_left_by_one ){


  for(unsigned i = 0;i < input.size();++i){
    input[i]+=1;
    reference[i] = sqeazy::detail::rotate_left<1>(input[i]);
  }

  sqeazy::detail::vec_rotate_left<unsigned short> rotate;

  for(unsigned i = 0;i < input.size();i+=8){
    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));
    v_in = rotate(&v_in);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[i]), v_in);
  }
    
    
    BOOST_REQUIRE(output[0]!=input[0]);
    for(unsigned i = 0;i < input.size();++i){
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    
  }

BOOST_AUTO_TEST_CASE( rotate_left_by_two ){

  for(unsigned i = 0;i < input.size();++i){
    input[i]+=1;
    reference[i] = sqeazy::detail::rotate_left<2>(input[i]);
  }

  sqeazy::detail::vec_rotate_left<unsigned short> rotate(2);

  for(unsigned i = 0;i < input.size();i+=8){
    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));
    v_in = rotate(&v_in);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[i]), v_in);
  }
    
    
  BOOST_REQUIRE(output[0]!=input[0]);
  for(unsigned i = 0;i < input.size();++i){
    BOOST_REQUIRE(output[i] == reference[i]);
  }
    
}

BOOST_AUTO_TEST_SUITE_END()

namespace fixed_value_fixture{

  static const unsigned short src = 3; // 11
  static const unsigned at = 6;
  static const unsigned size = 2;
  static const unsigned short dest = 4; // 100
  static const unsigned short exp = 196; // 11000100


};

BOOST_FIXTURE_TEST_SUITE( understand_setbits_of_integertype, default_128bit_fixture )
BOOST_AUTO_TEST_CASE( use_scalar_version ){

    unsigned short result = sqeazy::detail::setbits_of_integertype(fixed_value_fixture::dest,
							   fixed_value_fixture::src,
							   fixed_value_fixture::at,
							   2);
  
    BOOST_REQUIRE(result != fixed_value_fixture::dest);
    BOOST_REQUIRE(result == fixed_value_fixture::exp);
}


BOOST_AUTO_TEST_CASE( use_vectorized_version ){

  std::fill(reference.begin(), reference.end(), fixed_value_fixture::dest);

  for(unsigned i = 0;i < reference.size();++i){
    reference[i] = sqeazy::detail::setbits_of_integertype(reference[i], 
						  fixed_value_fixture::src, 
						  fixed_value_fixture::at, 
						  fixed_value_fixture::size);
  }
  

  std::fill(output.begin(), output.end(), fixed_value_fixture::dest);

  __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&output[0]));
  static const __m128i v_src = _mm_set1_epi16(fixed_value_fixture::src);

  set_bits<fixed_value_fixture::size, unsigned short>::inside(v_in, v_src, fixed_value_fixture::at);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);

  BOOST_REQUIRE(output[0]==reference[0]);
  
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( understand_bit_plane_reorder, default_128bit_fixture )

BOOST_AUTO_TEST_CASE( shift_m128i_left_by_a_byte ){

  std::fill(input.begin(),input.end(),255);

  __m128i v_const = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

  v_const = _mm_slli_si128(v_const,1);

  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_const);

  BOOST_REQUIRE(output[0]!=255);
  BOOST_REQUIRE(output[0]==(255 << 8));
}

BOOST_AUTO_TEST_CASE( understand_movemask_for_8bit_input ){

    unsigned char value = 128;
    __m128i test = _mm_set1_epi8(value);
    int result = _mm_movemask_epi8(test);
    BOOST_REQUIRE(result == std::numeric_limits<unsigned short>::max());
    
}

BOOST_AUTO_TEST_CASE( understand_movemask_for_16bit_input ){

    unsigned short value = 1 << 15;
    __m128i test = _mm_set1_epi16(value);
    int result = _mm_movemask_epi8(test);
    BOOST_REQUIRE(result != std::numeric_limits<unsigned short>::max());
    BOOST_REQUIRE(result == 170 + (170 << 8)); // that is 1010101010101010
  }

BOOST_AUTO_TEST_CASE( _bit_extraction_for_16bit_input ){

  unsigned short value = 1 << 15;
  __m128i testi = _mm_set1_epi16(value);
    
  __m128i v_low_items =  _mm_slli_si128(_mm_cvtepu16_epi32(testi),2); 
  __m128 low_part = *reinterpret_cast<__m128*>(&v_low_items);
  int result = _mm_movemask_ps (low_part); 

  __m128 test = reinterpret_cast<__m128>(testi);
  __m128i swapped_hi_lo = reinterpret_cast<__m128i>(_mm_movehl_ps(test,test));
  __m128i v_high_items = _mm_slli_si128(_mm_cvtepu16_epi32(swapped_hi_lo),2);
    
  result += _mm_movemask_ps (reinterpret_cast<__m128>(v_high_items)) << 4; 

  BOOST_REQUIRE(result != 0);
  BOOST_REQUIRE(result == 0xff);
}

BOOST_AUTO_TEST_SUITE_END()
  

BOOST_AUTO_TEST_CASE( _bit_extraction_for_16bit_input_of_all_bits ){


  unsigned short value = 1 << 15;
  __m128i testi = _mm_set1_epi16(value);
    
  std::vector<unsigned short> bitplanes(16,0);

  std::vector<unsigned short*> bitplanes_ptr(16);
  for(unsigned i = 0;i<bitplanes.size();++i)
    bitplanes_ptr[i] = &bitplanes[i];
    
  sqeazy::detail::reorder_bitplanes<1>(testi, bitplanes_ptr,8);
    
  BOOST_REQUIRE(bitplanes[0] == 255 << 8);
  for(unsigned i = 1;i<bitplanes.size();++i)
    BOOST_REQUIRE(bitplanes[i] == 0);


}

BOOST_FIXTURE_TEST_SUITE( understand_movemask, default_128bit_ofunsigned_fixture )

BOOST_AUTO_TEST_CASE( first_2_0_with_leading_1 ){

    input[2] = 0x80000000;
    input[3] = 0x80000000;
    
    __m128i v_const = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));
    __m128 c_input = reinterpret_cast<__m128>(v_const);
    
    int result = _mm_movemask_ps(c_input);

    BOOST_REQUIRE(result != 3);
  }

BOOST_AUTO_TEST_CASE( first_2_0_with_leading_1_with_shuffle ){

    input[2] = 0x80000000;
    input[3] = 0x80000000;
    
    __m128i v_const = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));
    __m128 c_input = reinterpret_cast<__m128>(_mm_shuffle_epi32(v_const,_MM_SHUFFLE(0,1,2,3)));
    
    int result = _mm_movemask_ps(c_input);

    BOOST_REQUIRE(result == 3);
  

}

BOOST_AUTO_TEST_SUITE_END()

