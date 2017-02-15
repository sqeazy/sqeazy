#define BOOST_TEST_MODULE TEST_HISTOGRAM_FILL

#include "boost/test/unit_test.hpp"

#include <climits>
#include <vector>
#include <iostream>
#include <cstdint>
#include <iterator>
#include <algorithm>


#include "volume_fixtures.hpp"

#include "encoders/histogram_utils.hpp"



BOOST_FIXTURE_TEST_SUITE( serial_fill, sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( works_with_pointers ){

  auto serial_histo = sqeazy::detail::serial_fill_histogram(embryo_.data(), embryo_.data()+embryo_.num_elements());

  auto sum = std::accumulate(serial_histo.begin(), serial_histo.end(), 0);

  BOOST_CHECK_NE(sum,0);

}

BOOST_AUTO_TEST_CASE( ramp ){

  std::vector<std::uint16_t> data(1024,0);
  std::uint32_t c = 0;
  for( std::uint16_t& el : data)
    el = c++ % 32;

  auto histo = sqeazy::detail::serial_fill_histogram(data.begin(), data.end());

  auto sum = std::accumulate(histo.begin(), histo.end(), 0);

  std::size_t expected_count = data.size()/32;

  BOOST_CHECK_NE(sum,0);
  BOOST_CHECK_EQUAL((std::size_t)sum,data.size());

  for(int i = 0;i<32;++i)
    BOOST_CHECK_EQUAL(histo[i],expected_count);

}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( parallel_fill, sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( ramp_with_1_thread ){

  std::vector<std::uint16_t> data(1024,0);
  std::uint32_t c = 0;
  for( std::uint16_t& el : data)
    el = c++ % 32;

  auto serial_histo = sqeazy::detail::serial_fill_histogram(data.begin(), data.end());
  auto parallel_histo = sqeazy::detail::parallel_fill_histogram(data.begin(), data.end(),1);

  std::size_t serial_sum = std::accumulate(serial_histo.begin(), serial_histo.end(), 0);
  std::size_t parallel_sum = std::accumulate(parallel_histo.begin(), parallel_histo.end(), 0);

  BOOST_CHECK_EQUAL( serial_sum, parallel_sum);
  BOOST_CHECK( std::equal(serial_histo.begin(), serial_histo.end(), parallel_histo.begin()) );

}

BOOST_AUTO_TEST_CASE( ramp_with_2_threads ){

  std::vector<std::uint16_t> data(1024,0);
  std::uint32_t c = 0;
  for( std::uint16_t& el : data)
    el = c++ % 32;

  auto serial_histo = sqeazy::detail::serial_fill_histogram(data.begin(), data.end());
  auto parallel_histo = sqeazy::detail::parallel_fill_histogram(data.begin(), data.end(),2);

  std::size_t serial_sum = std::accumulate(serial_histo.begin(), serial_histo.end(), 0);
  std::size_t parallel_sum = std::accumulate(parallel_histo.begin(), parallel_histo.end(), 0);

  BOOST_CHECK_EQUAL( serial_sum, parallel_sum);
}
BOOST_AUTO_TEST_SUITE_END()
