#ifndef _HDF5_FIXTURES_H_
#define _HDF5_FIXTURES_H_

#include "boost/filesystem.hpp"
namespace bfs = boost::filesystem;

struct helpers_fixture {

  const std::string     tfile;
  const std::string	dname;
  const std::string	test_output_name;
  const bfs::path	test_output_path;

  helpers_fixture():
    tfile("sample.h5"),
    dname("IntArray"),
    test_output_name("hdf5_helpers.h5"),
    test_output_path(test_output_name){}
  
};

#endif /* _HDF5_FIXTURES_H_ */
