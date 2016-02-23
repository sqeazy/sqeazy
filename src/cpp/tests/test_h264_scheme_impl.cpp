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

BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<uint8_t> )

BOOST_AUTO_TEST_CASE( encode ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);
  std::vector<uint32_t> shape = {static_cast<uint32_t>(embryo_.shape()[0]),
				 static_cast<uint32_t>(embryo_.shape()[1]),
				 static_cast<uint32_t>(embryo_.shape()[2])};
  std::size_t bytes_written = 0;
  uint32_t err = sqeazy::h264_scheme<uint8_t>::static_encode(embryo_.data(),
							     &results[0],
							     shape,
							     bytes_written);
  results.resize(bytes_written);
  
  BOOST_CHECK_EQUAL(err,0u);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  /* check that results is not filled with 0s anymore */
  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);

}

BOOST_AUTO_TEST_SUITE_END()
