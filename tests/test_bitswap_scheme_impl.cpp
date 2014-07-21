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

BOOST_FIXTURE_TEST_SUITE( shift_signed_bits_calculation, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( offset_called )
{
  unsigned short test_unsigned = 42;
  short test_signed = -42;

  unsigned short result_unsigned = sqeazy::shift_signed_bit_if_present<unsigned short>(test_unsigned);
  short result_signed = sqeazy::shift_signed_bit_if_present<unsigned short>(test_signed);
  
  BOOST_CHECK_EQUAL(result_unsigned,test_unsigned);
  BOOST_CHECK_NE(result_signed,(test_unsigned << 1) | 1);
  /*
    signed/unsigned integers are encoded by two's complement 
    http://en.wikipedia.org/wiki/Two_complement
    http://www.mathcs.emory.edu/~cheung/Courses/561/Syllabus/1-Intro/2-data-repr/signed.html
    
    Before you click on the above, here is what I mean for int32:
    -42 = 11111111111111111111111111010110
    42 = 00000000000000000000000000101010
    There is actually no signed bit per se.
  */

}

BOOST_AUTO_TEST_SUITE_END()
