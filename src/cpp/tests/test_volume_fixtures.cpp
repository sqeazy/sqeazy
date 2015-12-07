#define BOOST_TEST_MODULE TEST_VOLUME_FIXTURES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "tiff_utils.hpp"
#include "volume_fixtures.hpp"

BOOST_FIXTURE_TEST_SUITE( constructs , sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( embryo_filled ){

  BOOST_CHECK(embryo_.num_elements()>0);
  BOOST_CHECK_NE(std::accumulate(embryo_.data(), embryo_.data()+embryo_.num_elements(),0),0);

  //sqeazy::write_image_stack(embryo_,"embryo.tiff");
}

BOOST_AUTO_TEST_CASE( noisy_embryo_filled ){

  BOOST_CHECK(noisy_embryo_.num_elements()>0);
  BOOST_CHECK_NE(std::accumulate(noisy_embryo_.data(), noisy_embryo_.data()+noisy_embryo_.num_elements(),0),0);

  //sqeazy::write_image_stack(noisy_embryo_,"noisy_embryo.tiff");
}


BOOST_AUTO_TEST_SUITE_END()
