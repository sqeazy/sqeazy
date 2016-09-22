#define BOOST_TEST_MODULE TEST_COMPASS
#include "boost/test/unit_test.hpp"

#include "compass.hpp"

#include <vector>
#include <iostream>




BOOST_AUTO_TEST_SUITE( compass_fundamentals )

BOOST_AUTO_TEST_CASE( compass_works_at_runtime ){

  auto value = compass::runtime::works();

  BOOST_CHECK(value);
  
}

BOOST_AUTO_TEST_CASE( compass_yields_vendor_name ){

  auto value = compass::runtime::vendor();
  //  std::cout << "this machine was made by " << value << "\n";
  BOOST_CHECK_NE(value.size(),0);
  
}
BOOST_AUTO_TEST_SUITE_END()

