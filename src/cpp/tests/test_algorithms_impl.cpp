#define BOOST_TEST_MODULE TEST_SQEAZY_ALGORITHMS
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include "sqeazy_algorithms.hpp"


BOOST_AUTO_TEST_SUITE( prefix_sum )

BOOST_AUTO_TEST_CASE( works ){

  std::vector<std::uint16_t> src (1024, 2);
  std::vector<std::uint16_t> psum(1024, 0);

  auto resitr = sqeazy::prefix_sum(src.begin(),src.end(),psum.begin());

  BOOST_CHECK(psum[0] != psum[1]);
  BOOST_CHECK(resitr == psum.end());
  BOOST_CHECK(psum.front() != psum.back());
}

BOOST_AUTO_TEST_CASE( functional ){

  std::vector<std::uint16_t> src (1024, 2);
  std::vector<std::uint16_t> psum(1024, 0);

  auto resitr = sqeazy::prefix_sum(src.begin(),src.end(),psum.begin());

  BOOST_CHECK(resitr == psum.end());
  BOOST_CHECK_EQUAL(psum[0],0);
  BOOST_CHECK_EQUAL(psum[0],psum[1]-2);
  BOOST_CHECK_EQUAL(psum[1],psum[2]-2);

  for(std::size_t i = 1;i<src.size();++i)
    BOOST_CHECK_MESSAGE(psum[i] == psum[i-1]+2, i << "] prefix sum doesn't match [i]: " << psum[i] << " with previous one [i-1]: " << psum[i-1]+2);
}

BOOST_AUTO_TEST_CASE( in_parallel ){

  std::vector<std::uint16_t> src (1024, 2);
  std::vector<std::uint32_t> psum(1024, 0);

  auto resitr = sqeazy::prefix_sum(src.begin(),src.end(),psum.begin(), std::thread::hardware_concurrency());

  BOOST_CHECK(resitr == psum.end());
  BOOST_CHECK_EQUAL(psum[0],0u);
  BOOST_CHECK_EQUAL(psum[0],psum[1]-2);
  BOOST_CHECK_EQUAL(psum[1],psum[2]-2);

  for(std::size_t i = 1;i<src.size();++i)
    BOOST_CHECK_MESSAGE(psum[i] == psum[i-1]+2, i << "] prefix sum doesn't match [i]: " << psum[i] << " with previous one [i-1]: " << psum[i-1]+2);
}


BOOST_AUTO_TEST_SUITE_END()
