#define BOOST_TEST_MODULE TEST_HEVC_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

#define DEBUG_HEVC

#include "encoders/external_encoders.hpp"
#include "volume_fixtures.hpp"
#include "tiff_utils.hpp"
#include "yuv_utils.hpp"
#include "sqeazy_algorithms.hpp"
#include "traits.hpp"
#include "test_algorithms.hpp"

namespace sqy = sqeazy;

BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<uint8_t> )

BOOST_AUTO_TEST_CASE( encode ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);
  std::vector<uint32_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};
  std::size_t bytes_written = 0;
  uint32_t err = sqeazy::hevc_scheme<uint8_t>::static_encode(embryo_.data(),
							     &results[0],
							     shape,
							     bytes_written);
  results.resize(bytes_written);
  
  BOOST_CHECK_EQUAL(err,0u);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);
 
}

BOOST_AUTO_TEST_CASE( roundtrip ){


  av_register_all();
  
  std::vector<std::uint8_t> encoded(embryo_.num_elements(),0);
  std::vector<uint32_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};
  std::size_t bytes_written = 0;
  uint32_t err = sqeazy::hevc_scheme<uint8_t>::static_encode(embryo_.data(),
							     &encoded[0],
							     shape,
							     bytes_written);
  encoded.resize(bytes_written);
  
  BOOST_CHECK_EQUAL(err,0u);
  BOOST_CHECK_GT(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());



  std::vector<std::uint8_t> retrieved(embryo_.num_elements(),0);
  err = sqeazy::hevc_scheme<std::uint8_t>::static_decode(&encoded[0],
							 &retrieved[0],
							 encoded.size(),
							 embryo_.num_elements());

  BOOST_CHECK_EQUAL(err,0u);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm);
  
  try{
    BOOST_REQUIRE_LT(l2norm,1e-2);    
    BOOST_WARN_MESSAGE(std::equal(retrieved.begin(),
    				     retrieved.begin()+retrieved.size(),
    				     embryo_.data()),
		       "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);

	
  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }
    
}

BOOST_AUTO_TEST_CASE( noisy_roundtrip ){


  av_register_all();
  
  std::vector<std::uint8_t> encoded(noisy_embryo_.num_elements(),0);
  std::vector<uint32_t> shape = {static_cast<uint32_t>(noisy_embryo_.shape()[0]),
				 static_cast<uint32_t>(noisy_embryo_.shape()[1]),
				 static_cast<uint32_t>(noisy_embryo_.shape()[2])};
  std::size_t bytes_written = 0;
  uint32_t err = sqeazy::hevc_scheme<uint8_t>::static_encode(noisy_embryo_.data(),&encoded[0],shape, bytes_written);
  encoded.resize(bytes_written);
  
  BOOST_CHECK_EQUAL(err,0u);
  BOOST_CHECK_GT(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,noisy_embryo_.num_elements());

  std::vector<std::uint8_t> retrieved(noisy_embryo_.num_elements(),0);
  err = sqeazy::hevc_scheme<uint8_t>::static_decode(&encoded[0],
					     &retrieved[0],
					     encoded.size(),
					     noisy_embryo_.num_elements());

  BOOST_CHECK_EQUAL(err,0u);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm);
  try{
    
    BOOST_REQUIRE_LT(l2norm,1e-2);

    //TODO: looks like h265 is not bit exaxt for lossless encoding/decoding
    //TODO: check https://bitbucket.org/multicoreware/x265/issues/173/lossless-mode-is-not-bit-exact-and-the
    BOOST_WARN_MESSAGE(std::equal(retrieved.begin(),
				  retrieved.begin()+retrieved.size(),
				  noisy_embryo_.data()),
		       "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);

	
  }
  catch(...){
    sqeazy::write_image_stack(noisy_embryo_,"noisy_embryo.tiff");
    sqeazy::write_stack_as_y4m(noisy_embryo_,"noisy_embryo",true);
    sqeazy::write_stack_as_yuv(noisy_embryo_,"noisy_embryo",true);

    stack_t::extent_gen extent;
    retrieved_.resize(extent[noisy_embryo_.shape()[0]][noisy_embryo_.shape()[1]][noisy_embryo_.shape()[2]]);
    std::copy(retrieved.begin(), retrieved.end(),retrieved_.data());
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }


}

BOOST_AUTO_TEST_SUITE_END()

