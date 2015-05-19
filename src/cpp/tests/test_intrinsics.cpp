#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <numeric> 
#include <limits>
#include <climits>

#include "bitplane_reorder_detail.hpp"


#include <xmmintrin.h>
#include <smmintrin.h>


typedef const_anyvalue_fixture<8, 2> default_128bit_fixture; 
typedef const_anyvalue_fixture<16, 2, unsigned char> default_128bit_ofuchar_fixture;
typedef const_anyvalue_fixture<16, 2, char> default_128bit_ofchar_fixture;
typedef const_anyvalue_fixture<4, 2, unsigned> default_128bit_ofunsigned_fixture;
typedef const_anyvalue_fixture<(1 << 8), 2> default_cv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0xff00> default_hicv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0xff> default_locv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0x0ff0> default_micv_fixture; 

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
      BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0)==output.size());
    }
  }

BOOST_AUTO_TEST_CASE( shift_left_by_one ){


    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    v_in = _mm_slli_epi16(v_in,1);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
   
    BOOST_REQUIRE(output[0]!=input[0]);
    
    try{
    BOOST_REQUIRE(output[0]==4);
    BOOST_REQUIRE(std::accumulate(output.begin(), output.end(),0)==4*output.size());
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    }
  }
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( shift_16_bits, default_128bit_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){
  
  static const __m128i v_const = _mm_set1_epi16(2);

  shift_left_m128i<unsigned short> shifter;
  __m128i result = shifter(v_const,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == (2 << 1));
}

BOOST_AUTO_TEST_CASE( shift_msb_left_by_constant ){

  static const __m128i v_msb = _mm_set1_epi16(0x8000);

  shift_left_m128i<unsigned short> shifter;
  __m128i result = shifter(v_msb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == 0);
}


BOOST_AUTO_TEST_CASE( shift_right_by_constant ){


  static const __m128i v_const = _mm_set1_epi16(2);

  shift_right_m128i<unsigned short> shifter;
  __m128i result = shifter(v_const,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

  BOOST_REQUIRE(output[0] == (4 >> 1));
}

BOOST_AUTO_TEST_CASE( shift_lsb_right_by_constant ){
  static const __m128i v_lsb = _mm_set1_epi16(1);

  shift_right_m128i<unsigned short> shifter;
  __m128i result = shifter(v_lsb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  BOOST_REQUIRE(output[0] == 0);
  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( shift_8_bits, default_128bit_ofuchar_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){
  
  
  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

  shift_left_m128i<unsigned char> shifter;
  __m128i result = shifter(v_input,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  for (int i = 0; i < input.size()/2; ++i)
    {
      try{    

	BOOST_REQUIRE(output[i] == i << 1);
	  
      }catch(...){
	WARN("[shift-left-m128i-uchar] item " << i << " malformed");
	throw;
      }
    }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

  shift_left_m128i<unsigned char> shifter;
  static const __m128i v_msb = _mm_set1_epi8(0x80);
  __m128i result = shifter(v_msb,1);
  _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
  for (int i = 0; i < input.size(); ++i)
    {
      try{    
	BOOST_REQUIRE(output[i] == (unsigned char)0);
      }catch(...){
	WARN("[shift-left-m128i-uchar] item " << i << " malformed");
	throw;
      }
    }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi8(1);

    shift_right_m128i<unsigned char> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (unsigned char)0);
	}catch(...){
	  WARN("[shift-right-m128i-uchar] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    shift_right_m128i<unsigned char> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (unsigned char)(i >> 1));
	}catch(...){
	  WARN("[shift-right-m128i-uchar] item " << i << " malformed");
	  throw;
	}
      }

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( shift_8_bits_signed, default_128bit_ofchar_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){

  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

    shift_left_m128i<char> shifter;
    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

    for (int i = 0; i < input.size()/2; ++i)
      {
	try{    

	  BOOST_REQUIRE(output[i] == i << 1);
	  
	}catch(...){
	  WARN("[shift-left-m128i-char] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

    static const __m128i v_msb = _mm_set1_epi8(0x80);

    shift_left_m128i<char> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (char)0);
	}catch(...){
	  WARN("[shift-left-m128i-char] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi8(1);

    shift_right_m128i<char> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (char)0);
	}catch(...){
	  WARN("[shift-right-m128i-char] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    shift_right_m128i<char> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (char)(i >> 1));
	}catch(...){
	  WARN("[shift-right-m128i-char] item " << i << " malformed");
	  throw;
	}
      }

}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( shift_32_bits, default_128bit_ofunsigned_fixture )

BOOST_AUTO_TEST_CASE( shift_left_by_constant ){

  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));

    shift_left_m128i<unsigned> shifter;
    __m128i result = shifter(v_input,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);

    for (int i = 0; i < input.size()/2; ++i)
      {
	try{    

	  BOOST_REQUIRE(output[i] == i << 1);
	  
	}catch(...){
	  WARN("[shift-left-m128i-unsigned] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_left_msb_by_constant ){

    static const __m128i v_msb = _mm_set1_epi8(0x80);

    shift_left_m128i<unsigned> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (unsigned)0);
	}catch(...){
	  WARN("[shift-left-m128i-unsigned] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_right_msb_by_constant ){

    static const __m128i v_lsb = _mm_set1_epi8(1);

    shift_right_m128i<unsigned> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (unsigned)0);
	}catch(...){
	  WARN("[shift-right-m128i-unsigned] item " << i << " malformed");
	  throw;
	}
      }

}

BOOST_AUTO_TEST_CASE( shift_right_by_constant ){

  for (int i = 0; i < input.size(); ++i)
    {
      input[i] = i;
    }

  __m128i v_input = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));


    shift_right_m128i<unsigned> shifter;
    
    __m128i result = shifter(v_msb,1);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]),result);
    for (int i = 0; i < input.size(); ++i)
      {
	try{    
	  REQUIRE(output[i] == (unsigned)(i >> 1));
	}catch(...){
	  WARN("[shift-right-m128i-unsigned] item " << i << " malformed");
	  throw;
	}
      }

}
BOOST_AUTO_TEST_SUITE_END()

// REFACTOR OUT
#include "bitplane_reorder_detail.hpp"
#include "bitswap_helpers.hpp"

TEST_CASE("constant 8bit array", "rotate-char"){

  //one 128bit buffer
  std::vector<unsigned char> input(8,2);
  std::vector<unsigned char> output(8,0);  



  SECTION("rotate left by one"){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    vec_rotate_left<unsigned char> rotate;
    v_in = rotate(&v_in);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
   
    REQUIRE(output[0]!=input[0]);
    
    try{
  REQUIRE(output[0]==sqeazy::rotate_left<1>(input[0]));

      REQUIRE(std::accumulate(output.begin(), output.end(),0)==4*output.size());
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
    }
  }

  SECTION("rotate left by two and cycle"){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    vec_rotate_left<unsigned char> rotate(2);
    v_in = rotate(&v_in);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    REQUIRE(output[0]!=input[0]);
    
    try{
      REQUIRE(output[0]==sqeazy::rotate_left<2>(input[0]));
      REQUIRE(std::accumulate(output.begin(), output.end(),0)==sqeazy::rotate_left<2>(input[0])*output.size());
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
    }
  }

  SECTION("rotate left and flip"){

    for (int i = 0; i < input.size(); ++i)
      {
	input[i] = 0x80;
      }
    
    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));
    vec_rotate_left<unsigned char> rotate(1);
    v_in = rotate(&v_in);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    
    REQUIRE(output[0] == 1);
  }

 }

TEST_CASE("constant 16bit array", "rotate-short"){

  //one 128bit buffer
  std::vector<unsigned short> input(8,2);
  std::vector<unsigned short> output(8,0);  



  SECTION("rotate left by one"){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    vec_rotate_left<unsigned short> rotate;
    v_in = rotate(&v_in);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
   
    REQUIRE(output[0]!=input[0]);
    
    try{
  REQUIRE(output[0]==sqeazy::rotate_left<1>(input[0]));

      REQUIRE(std::accumulate(output.begin(), output.end(),0)==4*output.size());
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    }
  }

  SECTION("rotate left by two and cycle"){

    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));

    vec_rotate_left<unsigned short> rotate(2);
    v_in = rotate(&v_in);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    REQUIRE(output[0]!=input[0]);
    
    try{
      REQUIRE(output[0]==sqeazy::rotate_left<2>(input[0]));
      REQUIRE(std::accumulate(output.begin(), output.end(),0)==sqeazy::rotate_left<2>(input[0])*output.size());
    }
    catch(...){
      std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    }
  }

  SECTION("rotate left and flip"){

    for (int i = 0; i < input.size(); ++i)
      {
	input[i] = 0x8000;
      }
    
    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));
    vec_rotate_left<unsigned short> rotate(1);
    v_in = rotate(&v_in);
    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);
    
    REQUIRE(output[0] == 1);
  }

 }


TEST_CASE("variable 16bit array", "rotate"){

  //one 128bit buffer
  std::vector<unsigned short> input(8*16,0);
  std::vector<unsigned short> reference(input.size(),0);  
  
  for(int i = 0;i < input.size();++i){
    input[i]+=1;
  }
  
  std::vector<unsigned short> output(input.size(),0);  


  SECTION("rotate left by one"){

    for(int i = 0;i < input.size();++i){
      reference[i] = sqeazy::rotate_left<1>(input[i]);
    }

    vec_rotate_left<unsigned short> rotate;

    for(int i = 0;i < input.size();i+=8){
      __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));
      v_in = rotate(&v_in);
      _mm_store_si128(reinterpret_cast<__m128i*>(&output[i]), v_in);
    }
    
    
    REQUIRE(output[0]!=input[0]);
    for(int i = 0;i < input.size();++i){
      REQUIRE(output[i] == reference[i]);
    }
    
  }

  SECTION("rotate left by two"){

    for(int i = 0;i < input.size();++i){
      reference[i] = sqeazy::rotate_left<2>(input[i]);
    }

    vec_rotate_left<unsigned short> rotate(2);

    for(int i = 0;i < input.size();i+=8){
      __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));
      v_in = rotate(&v_in);
      _mm_store_si128(reinterpret_cast<__m128i*>(&output[i]), v_in);
    }
    
    
    REQUIRE(output[0]!=input[0]);
    for(int i = 0;i < input.size();++i){
      REQUIRE(output[i] == reference[i]);
    }
    
  }

}

TEST_CASE("understand setbits_of_integertype", "set bits"){

  static const unsigned short src = 3; // 11
  static const unsigned at = 6;
  static const unsigned size = 2;
  static const unsigned short dest = 4; // 100
  static const unsigned short exp = 196; // 11000100

  
  SECTION("use scalar version"){
    unsigned short result = sqeazy::setbits_of_integertype(dest,src,at,2);
  
    REQUIRE(result != dest);
    REQUIRE(result == exp);
  }

  std::vector<unsigned short> reference(8,dest);
  
  for(int i = 0;i < reference.size();++i){
    reference[i] = sqeazy::setbits_of_integertype(reference[i], src, at, size);
  }
  
  std::vector<unsigned short> output(reference.size(),dest);  

  SECTION("set bits with vectorized chunk"){
    __m128i v_in = _mm_load_si128(reinterpret_cast<const __m128i*>(&output[0]));
    static const __m128i v_src = _mm_set1_epi16(src);

    set_bits<size, unsigned short>::inside(v_in, v_src, at);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_in);

    REQUIRE(output[0]==reference[0]);
  }
}


TEST_CASE("understand bit plane reorder", "intrinsics"){

  std::vector<unsigned short> input(8, 255 ); // 0xff or 11111111
  __m128i v_const = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[0]));
  std::vector<unsigned short> output(input.size(),0);

  SECTION("shift __m128i left  by a byte"){
    v_const = _mm_slli_si128(v_const,1);

    _mm_store_si128(reinterpret_cast<__m128i*>(&output[0]), v_const);

    REQUIRE(output[0]!=255);
    REQUIRE(output[0]==(255 << 8));
  }

  SECTION("understand movemask for 8bit input"){
    unsigned char value = 128;
    __m128i test = _mm_set1_epi8(value);
    int result = _mm_movemask_epi8(test);
    REQUIRE(result == std::numeric_limits<unsigned short>::max());
    
  }

  SECTION("understand movemask for 16bit input"){
    unsigned short value = 1 << 15;
    __m128i test = _mm_set1_epi16(value);
    int result = _mm_movemask_epi8(test);
    REQUIRE(result != std::numeric_limits<unsigned short>::max());
    REQUIRE(result == 170 + (170 << 8)); // that is 1010101010101010
  }

  SECTION("bit extraction for 16bit input"){
    unsigned short value = 1 << 15;
    __m128i testi = _mm_set1_epi16(value);
    
    __m128i v_low_items =  _mm_slli_si128(_mm_cvtepu16_epi32(testi),2); 
    __m128 low_part = *reinterpret_cast<__m128*>(&v_low_items);
    int result = _mm_movemask_ps (low_part); 

    __m128 test = reinterpret_cast<__m128>(testi);
    __m128i swapped_hi_lo = reinterpret_cast<__m128i>(_mm_movehl_ps(test,test));
  __m128i v_high_items = _mm_slli_si128(_mm_cvtepu16_epi32(swapped_hi_lo),2);
    
    result += _mm_movemask_ps (reinterpret_cast<__m128>(v_high_items)) << 4; 

    REQUIRE(result != 0);
    REQUIRE(result == 0xff);
  }

  
  
}

TEST_CASE("bit extraction for 16bit input of all bits to short","[bp-reorder-const]"){

  unsigned short value = 1 << 15;
  __m128i testi = _mm_set1_epi16(value);
    
  std::vector<unsigned short> bitplanes(16,0);

  std::vector<unsigned short*> bitplanes_ptr(16);
  for(int i = 0;i<bitplanes.size();++i)
    bitplanes_ptr[i] = &bitplanes[i];
    
  reorder_bitplanes<1>(testi, bitplanes_ptr,8);
    
  REQUIRE(bitplanes[0] == 255 << 8);
  for(int i = 1;i<bitplanes.size();++i)
    REQUIRE(bitplanes[i] == 0);


}

TEST_CASE("understand movemask", "[movemask]"){
  
  std::vector<unsigned> input(4,0);

  SECTION("first 2 0, next with leading 1"){
    input[2] = 0x80000000;
    input[3] = 0x80000000;
    
    __m128i v_const = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));
    __m128 c_input = reinterpret_cast<__m128>(v_const);
    
    int result = _mm_movemask_ps(c_input);

    REQUIRE(result != 3);
  }

  SECTION("first 2 0, next with leading 1, with shuffle"){
    input[2] = 0x80000000;
    input[3] = 0x80000000;
    
    __m128i v_const = _mm_load_si128(reinterpret_cast<__m128i*>(&input[0]));
    __m128 c_input = reinterpret_cast<__m128>(_mm_shuffle_epi32(v_const,_MM_SHUFFLE(0,1,2,3)));
    
    int result = _mm_movemask_ps(c_input);

    REQUIRE(result == 3);
  }

}


TEST_CASE("2-bit extraction for 16bit input of all bits to short","[bp2-reorder-const]"){

  unsigned short value = 1 << 15;
  __m128i testi = _mm_set1_epi16(value);
    
  std::vector<unsigned short> bitplanes(16,0);

  std::vector<unsigned short*> bitplanes_ptr(16);
  for(int i = 0;i<bitplanes.size();++i)
    bitplanes_ptr[i] = &bitplanes[i];
    
  reorder_bitplanes<2>(testi, bitplanes_ptr,8);
    
  REQUIRE(bitplanes[0] == (0xaa00 << 1));
  for(int i = 1;i<bitplanes.size();++i)
    REQUIRE(bitplanes[i] == 0);

  
}
