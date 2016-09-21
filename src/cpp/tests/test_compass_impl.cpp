#define BOOST_TEST_MODULE TEST_COMPASS
#include "boost/test/unit_test.hpp"

#include "compass.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator



BOOST_AUTO_TEST_SUITE( compass_fundamentals )

BOOST_AUTO_TEST_CASE( compass_works_at_runtime ){

  auto value = compass::runtime::works();

  BOOST_CHECK(value);
  
}
BOOST_AUTO_TEST_SUITE_END()

