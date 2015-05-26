#define BOOST_TEST_MODULE TEST_HDF5_IMPLEMENTATION
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>
#include <stdexcept>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

#include "hdf5_utils.hpp"
#include "hdf5_test_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;


namespace bfs = boost::filesystem;
  
BOOST_FIXTURE_TEST_SUITE( hdf5_uint16_suite, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( static_type_instance_works ){

  BOOST_CHECK(sqeazy::hdf5_dtype<unsigned short>::instance().getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_dtype<short>::instance().getSize() == 2);

  BOOST_CHECK(sqeazy::hdf5_dtype<unsigned char>::instance().getSize() == 1);
  BOOST_CHECK(sqeazy::hdf5_dtype<char>::instance().getSize() == 1);

}

BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path fname = "hdf5_uint16_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned short>(fname.string(), dname, &constant_cube[0], dims);
  bool remove_file = false;


  BOOST_REQUIRE(rvalue == 0);
  BOOST_CHECK(bfs::exists(fname));
  BOOST_REQUIRE(dataset_in_h5_file(fname.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(fname.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(fname.string(), dname) == 2);
  
  bfs::remove(fname);
  
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( hdf5_uint8_suite, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path fname = "hdf5_uint8_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned char>(fname.string(), dname, &constant_cube[0], dims);
  bool remove_file = false;


  BOOST_REQUIRE(rvalue == 0);
  BOOST_CHECK(bfs::exists(fname));
  BOOST_REQUIRE(dataset_in_h5_file(fname.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(fname.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(fname.string(), dname) == 1);
  
  bfs::remove(fname);
  
}

BOOST_AUTO_TEST_SUITE_END()
