#define BOOST_TEST_MODULE TEST_HDF5_INTERFACE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

//taken from 
//http://www.hdfgroup.org/ftp/HDF5/current/src/unpacked/c++/examples/h5tutr_cmprss.cpp
#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

#include "sqeazy_hdf5_impl.hpp"
#include "hdf5_utils.hpp"

static sqeazy::loaded_hdf5_plugin always_load;/*
					  as the loaded_hdf5_plugin class is instantiated as a static object, 
					  the sqy h5 filter will be registered at the start of any program that 
					  includes this (either by code or by linking against the library).
					  The filter will be unregistered once the app/lib finishes running
					  or being called. 
					*/

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( encode_decode_using_hdf5_filter, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( filter_available ){

  htri_t avail;
  avail = H5Zfilter_avail(H5Z_FILTER_SQY);

  BOOST_CHECK(avail);
  
}

BOOST_AUTO_TEST_CASE( filter_supports_encoding ){

  unsigned filter_config = 0;
  herr_t status = H5Zget_filter_info (H5Z_FILTER_SQY, &filter_config);
  
  BOOST_CHECK(filter_config & H5Z_FILTER_CONFIG_ENCODE_ENABLED);
  
}

BOOST_AUTO_TEST_CASE( filter_supports_decoding ){

  unsigned filter_config = 0;
  herr_t status = H5Zget_filter_info (H5Z_FILTER_SQY, &filter_config);
  
  BOOST_CHECK(filter_config & H5Z_FILTER_CONFIG_DECODE_ENABLED);
  
}

BOOST_AUTO_TEST_CASE( h5_write_file ){

  boost::filesystem::path fname = "test_hdf5_interface.h5";
  std::string dname = "write_file";

  
  BOOST_CHECK_NO_THROW(h5_compress_arb_dataset(fname.string(),
			  dname,
			  constant_cube,
					       dims));
  
  BOOST_CHECK(boost::filesystem::exists(fname));
  
  BOOST_REQUIRE(!sqy_used_in_h5_file(fname.string(),dname));
  boost::filesystem::remove(fname);//remove test file if all is fine
  
}

BOOST_AUTO_TEST_CASE( h5_write_file_with_sqy ){

  boost::filesystem::path fname = "test_hdf5_interface_with_sqy.h5";
  std::string dname = "write_file_with_sqy";

  
  BOOST_CHECK_NO_THROW(h5_compress_arb_dataset(fname.string(),
					       dname,
					       constant_cube,
					       dims,
					       true));
  
  BOOST_CHECK(boost::filesystem::exists(fname));

  
  BOOST_REQUIRE(sqy_used_in_h5_file(fname.string(),dname));
  boost::filesystem::remove(fname);//remove test file if all is fine
}

 BOOST_AUTO_TEST_SUITE_END()
