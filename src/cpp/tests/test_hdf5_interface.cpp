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

//#include "hdf5_test_utils.hpp"
#include "hdf5_fixtures.hpp"

// static sqeazy::loaded_hdf5_plugin always_load;
/*
  as the loaded_hdf5_plugin class is instantiated as a static object, 
  the sqy h5 filter will be registered at the start of any program that 
  includes this (either by code or by linking against the library).
  The filter will be unregistered once the app/lib finishes running
  or being called. 
*/

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;



BOOST_FIXTURE_TEST_SUITE( hdf5_inference_methods, helpers_fixture )

BOOST_AUTO_TEST_CASE( sizeof_dataset ){

  unsigned size_in_byte = 0;
  int rvalue = SQY_h5_query_sizeof(tfile.c_str(), dname.c_str(), &size_in_byte);

  BOOST_CHECK_EQUAL(size_in_byte,4);
  
}

BOOST_AUTO_TEST_SUITE_END()
