#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_LZ4_ENCODING_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <sstream>
#include "array_fixtures.hpp"
#include <bitset>
#include "../src/external_encoders.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( lz4_out_of_place, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( decode_dimensions )
{
  std::string given = "my_pipeline,s,1,32x32x16|bla";
  std::vector<unsigned> dims = sqeazy::lz4_scheme<short>::decode_dimensions(given.c_str(),given.size());
  BOOST_CHECK_EQUAL(dims.size(),3);
  BOOST_CHECK_EQUAL(dims[0],32);
  BOOST_CHECK_EQUAL(dims[1],32);
  BOOST_CHECK_EQUAL(dims[2],16);
}


BOOST_AUTO_TEST_SUITE_END()


// BOOST_AUTO_TEST_SUITE_END()
