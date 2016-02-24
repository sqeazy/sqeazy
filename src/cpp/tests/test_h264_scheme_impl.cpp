#define BOOST_TEST_MODULE TEST_H264_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

#define DEBUG_H264

#include "encoders/h264.hpp"
#include "volume_fixtures.hpp"
#include "tiff_utils.hpp"
#include "yuv_utils.hpp"
#include "sqeazy_algorithms.hpp"
#include "traits.hpp"
#include "test_algorithms.hpp"

namespace sqy = sqeazy;

BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<uint8_t> )

BOOST_AUTO_TEST_CASE( encode_to_something_different ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<uint8_t> scheme;
  uint8_t* results_end = scheme.encode(embryo_.data(),
				       &results[0],
				       shape);
  std::size_t bytes_written = results_end - &results[0];
  results.resize(bytes_written);
  
  BOOST_CHECK(results_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  /* check that results is not filled with 0s anymore */
  double sum = std::accumulate(results.begin(), results.end(),0.);
  BOOST_CHECK_NE(sum,0);

}

BOOST_AUTO_TEST_CASE( lossy_roundtrip ){

  av_register_all();
  
  std::vector<uint8_t> encoded(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
				    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  sqeazy::h264_scheme<uint8_t> scheme;
  uint8_t* encoded_end = scheme.encode(embryo_.data(),
				       &encoded[0],
				       shape);
  std::size_t bytes_written = encoded_end - &encoded[0];
  encoded.resize(bytes_written);
  
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  std::vector<std::uint8_t> retrieved(embryo_.num_elements(),0);
  int err = scheme.decode(&encoded[0],
			  &retrieved[0],
			  shape);

  BOOST_CHECK_EQUAL(err,0u);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  double l1norm = sqeazy::l1norm(retrieved_cref,embryo_cref);
  
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );
  
  try{
    BOOST_REQUIRE_LT(l2norm,1);    
    // BOOST_REQUIRE_MESSAGE(std::equal(retrieved.begin(),
    // 				     retrieved.begin()+retrieved.size(),
    // 				     embryo_.data()),"raw volume and encoded/decoded volume do not match");

	
  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }
    
}

BOOST_AUTO_TEST_SUITE_END()
