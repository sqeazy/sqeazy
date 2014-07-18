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

// BOOST_FIXTURE_TEST_SUITE( diff_scheme_suite, uint16_cube_of_8 )
 

// BOOST_AUTO_TEST_CASE( check_success )
// {
  
//   const unsigned axis_size = axis_length;
//   const unsigned total_size = size;
//   BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(0,axis_length,axis_length,axis_length),true);
//   BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(axis_size+1,axis_size,axis_size,axis_size),true);
//   BOOST_CHECK_EQUAL(local_diff_scheme::index_in_ghost_layer(total_size/2,axis_size,axis_size,axis_size),false);
// }

// BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( offset_calculation, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( offset_called )
{
  const unsigned axis_size = axis_length;
  std::vector<unsigned> offsets;

  sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
  geometry.compute_offsets_in_x(offsets);
  
  BOOST_CHECK_GT(offsets.size(),0);
  unsigned expected = (axis_size-2)*(axis_size-2);
  BOOST_CHECK_EQUAL(offsets.size(),expected);
}


BOOST_AUTO_TEST_CASE( offset_exact )
{
  const unsigned axis_size = axis_length;
  std::vector<unsigned> offsets;
  
  sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
  geometry.compute_offsets_in_x(offsets);
  
  BOOST_CHECK_EQUAL(offsets.at(0),axis_size*axis_size + axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.at(1),axis_size*axis_size + 2*axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.back(),(axis_size-1-1)*axis_size*axis_size + ((axis_size-1-1))*axis_size + 1);
}
BOOST_AUTO_TEST_SUITE_END()
