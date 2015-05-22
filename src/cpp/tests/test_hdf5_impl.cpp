#define BOOST_TEST_MODULE TEST_HDF5_IMPLEMENTATION
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

#include "hdf5_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( hdf5_infrastructure, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( static_type_instance_works ){

  BOOST_CHECK(sqeazy::hdf5_dtype<unsigned short>::instance().getSize() == 2);
  
}

BOOST_AUTO_TEST_SUITE_END()
