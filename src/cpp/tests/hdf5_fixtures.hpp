#ifndef _HDF5_FIXTURES_H_
#define _HDF5_FIXTURES_H_

#include <iostream>
#include "boost/filesystem.hpp"
namespace bfs = boost::filesystem;

struct helpers_fixture {

  const std::string     tfile_basename;
  std::string		tfile;
  const std::string	dname;
  const std::string	test_output_name;
  const bfs::path	test_output_path;

  helpers_fixture():
    tfile_basename("sample.h5"),
    tfile(""),
    dname("IntArray"),
    test_output_name("hdf5_helpers.h5"),
    test_output_path(test_output_name){

    const bfs::path here = bfs::absolute(bfs::current_path());
    bfs::path test_sample = here;
    test_sample /= tfile_basename;
    
    if(!bfs::exists(test_sample)){
      test_sample = here;
      test_sample /= "tests";
      test_sample /= tfile_basename;
    }
      
    if(!bfs::exists(test_sample))
      std::cerr << "[helpers_fixture] unable to find test_sample at " << test_sample << "\n";

    tfile = test_sample.string();
  }
  
};

#endif /* _HDF5_FIXTURES_H_ */
