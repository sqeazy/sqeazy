#define BOOST_TEST_MODULE TEST_BENCHMARK_FIXTURE_IMPL
#include "boost/test/unit_test.hpp"

#include <iostream>

#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> default_static_synthetic_data;

BOOST_FIXTURE_TEST_SUITE( fundamentals , default_static_synthetic_data )

BOOST_AUTO_TEST_CASE( fixture_can_be_setup ){

  BOOST_CHECK(!shape.empty());

  // std::cout << "x,y,z = "
  //           << shape[sqeazy::row_major::x] << ","
  //           << shape[sqeazy::row_major::y] << ","
  //           << shape[sqeazy::row_major::z] << "\n";

  BOOST_CHECK(shape[sqeazy::row_major::x] % 2 == 0);
  BOOST_CHECK_GT(shape[sqeazy::row_major::x], 0);

  BOOST_CHECK(shape[sqeazy::row_major::y] % 2 == 0);
  BOOST_CHECK_GT(shape[sqeazy::row_major::y], 0);

  BOOST_CHECK(shape[sqeazy::row_major::z] % 2 == 0);
  BOOST_CHECK_GT(shape[sqeazy::row_major::z], 0);

}
BOOST_AUTO_TEST_SUITE_END()
