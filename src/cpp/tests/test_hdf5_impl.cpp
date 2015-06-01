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

struct helpers_fixture {

  const std::string     tfile;
  const std::string	dname;
  const std::string	test_output_name;
  const bfs::path	fpath;

  helpers_fixture():
    tfile("sample.h5"),
    dname("IntArray"),
    test_output_name("hdf5_helpers.h5"),
    fpath(test_output_name){}
  
};

BOOST_FIXTURE_TEST_SUITE( hdf5_helpers, helpers_fixture )

BOOST_AUTO_TEST_CASE( query_for_dataset ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.has_dataset(dname));
  
}

BOOST_AUTO_TEST_SUITE_END()


  
BOOST_FIXTURE_TEST_SUITE( hdf5_uint16_suite, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( static_type_instance_works ){

  BOOST_CHECK(sqeazy::hdf5_dtype<unsigned short>::instance().getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_dtype<short>::instance().getSize() == 2);

  BOOST_CHECK(sqeazy::hdf5_dtype<unsigned char>::instance().getSize() == 1);
  BOOST_CHECK(sqeazy::hdf5_dtype<char>::instance().getSize() == 1);

}

BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint16_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned short>(test_output_name.string(), dname, &constant_cube[0], dims);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_CHECK(bfs::exists(test_output_name));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(test_output_name.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(test_output_name.string(), dname) == 2);
  
  bfs::remove(test_output_name);
  
}

BOOST_AUTO_TEST_CASE( roundtrip_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint16_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned short>(test_output_name.string(), dname, &constant_cube[0], dims);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  
  std::vector<int> read_shape;
  to_play_with.clear();

  rvalue = sqeazy::read_h5<unsigned short>(test_output_name.string(), dname, to_play_with, read_shape);
  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  read_shape.begin(), read_shape.end());
  
  bfs::remove(test_output_name);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( hdf5_uint8_suite, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint8_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned char>(test_output_name.string(), dname, &constant_cube[0], dims);
  bool remove_file = false;


  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_CHECK(bfs::exists(test_output_name));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(test_output_name.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(test_output_name.string(), dname) == 1);
  
  bfs::remove(test_output_name);
  
}

BOOST_AUTO_TEST_SUITE_END()


