#define BOOST_TEST_MODULE TEST_BITPLANE_REORDER
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include "encoders/sqeazy_impl.hpp"
#include "sse_test_utils.hpp"



typedef const_anyvalue_fixture<(1 << 8), 2> default_cv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0xff00> default_hicv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0xff> default_locv_fixture; 
typedef const_anyvalue_fixture<(1 << 15), 0x0ff0> default_micv_fixture; 

typedef ramp_fixture<(1 << 15)> default_ramp_fixture; 



BOOST_FIXTURE_TEST_SUITE( bitplane_reorder, default_cv_fixture )

BOOST_AUTO_TEST_CASE( runs_on_data ){

  sqeazy::bitswap_scheme<unsigned short,1>::static_encode(&input[0], &reference[0],input.size());
  
  try{
    BOOST_REQUIRE( reference[input.size()-1] == 0 );
    BOOST_REQUIRE( reference[(input.size()-1)-(32)] != 0 );
  }
  catch(...){
    std::copy(reference.begin(), reference.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
  }

}

BOOST_AUTO_TEST_CASE( vectorized_version_is_callable ){

  int rc = 0;
  BOOST_REQUIRE( rc == sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0],input.size()));
}

BOOST_AUTO_TEST_CASE(produces_same_output){


  sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0],input.size());
    
  BOOST_REQUIRE( reference[0] == output[0] );
  try{
    BOOST_REQUIRE( reference[input.size()-3] == output[input.size()-3] );
  }
  catch(...){
    std::cout << "reference:\n";
    std::copy(reference.begin(), reference.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    std::cout << "\noutput:\n";
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned short>(std::cout, " "));
    std::cout << "\n";
  }
}
BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE( check_sse_default_highbit, default_hicv_fixture )

BOOST_AUTO_TEST_CASE( versus_default_first_16 ){
  
  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  BOOST_REQUIRE(ret1 == ret2);
  for(int i = 0;i<16;++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-highbit 16] " << i << " item does not match" );
      throw;
    }
  }
}


BOOST_AUTO_TEST_CASE( versus_default_all ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], input.size());
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], input.size());

  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-highbit all] " << i << " item does not match" );
      throw;
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( check_sse_default_lobit, default_locv_fixture )

BOOST_AUTO_TEST_CASE( versus_default_first_16 ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<16;++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-lowbit 16] " << i << " item does not match" );
      throw;
    }
  }
}

BOOST_AUTO_TEST_CASE( versus_default_all ){
  
  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-lowbit all] " << i << " item does not match" );
      throw;
    }
  }
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( check_sse_default_mibit, default_micv_fixture )

BOOST_AUTO_TEST_CASE( versus_default_first_16 ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<16;++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-midbit 16] " << i << " item does not match" );
      throw;
    }
  }
}

BOOST_AUTO_TEST_CASE( versus_default_all ){
  
  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      BOOST_TEST_MESSAGE("[check-sse-default-midbit all] " << i << " item does not match" );
      throw;
    }
  }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( check_sse_ramp, default_ramp_fixture )

BOOST_AUTO_TEST_CASE( versus_default_first_16 ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  
    BOOST_REQUIRE(ret1 == ret2);
    for(unsigned i = 0;i<16;++i){
      BOOST_CHECK(output[i] == calc_first_16_hand[i]);
      BOOST_CHECK(reference[i] == calc_first_16_hand[i]);
      try{
	BOOST_REQUIRE(output[i] == reference[i]);
      }
      catch(...){
	BOOST_TEST_MESSAGE("[check-sse-default 16 zick-zack] sse-vs-scalar" << i << " / "<< 16<<" item does not match" );
	throw;
      }
      
    }
  }

BOOST_AUTO_TEST_CASE( versus_default_all ){

 int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 16);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 16);

  
    BOOST_REQUIRE(ret1 == ret2);
    for(unsigned i = 0;i<input.size();++i){
      try{
	BOOST_REQUIRE(output[i] == reference[i]);
      }
      catch(...){
	BOOST_TEST_MESSAGE("[check-sse-default zick-zack] " << i << " / "<< input.size()<<" item does not match" );
	throw;
      }
    }
  }

BOOST_AUTO_TEST_SUITE_END()

typedef const_anyvalue_fixture<(1 << 8), 2, unsigned char> default_cv_fixture_8bit;

BOOST_FIXTURE_TEST_SUITE( bitplane_reorder_8bit, default_cv_fixture_8bit )

// BOOST_AUTO_TEST_CASE( runs_on_data ){

//   sqeazy::bitswap_scheme<unsigned char,1>::static_encode(&input[0], &reference[0],input.size());
  
//   try{
//     BOOST_REQUIRE( reference[input.size()-1] == 0 );
//     BOOST_REQUIRE( reference[(input.size()-1)-(32)] != 0 );
//   }
//   catch(...){
//     std::copy(reference.begin(), reference.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
//   }

// }

BOOST_AUTO_TEST_CASE( vectorized_version_is_callable ){

  int rc = 0;
  BOOST_REQUIRE( rc == sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0],input.size()));
}

BOOST_AUTO_TEST_CASE(produces_same_output){


  sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0],input.size());
    
  BOOST_REQUIRE( reference[0] == output[0] );
  try{
    BOOST_REQUIRE( reference[input.size()-3] == output[input.size()-3] );
  }
  catch(...){
    std::cout << "reference:\n";
    std::copy(reference.begin(), reference.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
    std::cout << "\noutput:\n";
    std::copy(output.begin(), output.end(), std::ostream_iterator<unsigned char>(std::cout, " "));
    std::cout << "\n";
  }
}
BOOST_AUTO_TEST_SUITE_END()
//TODO!
// TEST_CASE("2-bit extraction for 16bit input of all bits to short","[bp2-reorder-const]"){

//   unsigned short value = 1 << 15;
//   __m128i testi = _mm_set1_epi16(value);
    
//   std::vector<unsigned short> bitplanes(16,0);

//   std::vector<unsigned short*> bitplanes_ptr(16);
//   for(int i = 0;i<bitplanes.size();++i)
//     bitplanes_ptr[i] = &bitplanes[i];
    
//   reorder_bitplanes<2>(testi, bitplanes_ptr,8);
    
//   REQUIRE(bitplanes[0] == (0xaa00 << 1));
//   for(int i = 1;i<bitplanes.size();++i)
//     REQUIRE(bitplanes[i] == 0);

  
// }

