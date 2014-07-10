#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::diff_scheme<unsigned short> local_diff_scheme ;

BOOST_FIXTURE_TEST_SUITE( diff_scheme_suite, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( check_success )
{
  
  const unsigned axis_size = axis_length;
  const unsigned total_size = size;
  BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(0,axis_length,axis_length,axis_length),true);
  BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(axis_size+1,axis_size,axis_size,axis_size),true);
  BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(total_size/2,axis_size,axis_size,axis_size),false);
}

BOOST_AUTO_TEST_SUITE_END()
