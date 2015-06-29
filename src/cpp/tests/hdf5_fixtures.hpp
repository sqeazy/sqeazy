#ifndef _HDF5_FIXTURES_H_
#define _HDF5_FIXTURES_H_

#include <iostream>
#include "boost/filesystem.hpp"
namespace bfs = boost::filesystem;

struct helpers_fixture {

  std::string     tfile_basename;
  std::string		tfile;
  std::string	dname;
  std::string	test_output_name;
  bfs::path	test_output_path;

  std::vector<unsigned short> retrieved;
  std::vector<unsigned int> dims;

  
  helpers_fixture():
    tfile_basename("sample.h5"),
    tfile(""),
    dname("IntArray"),
    test_output_name("hdf5_helpers.h5"),
    test_output_path(test_output_name),
    retrieved(),
    dims(3,8)
  {

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

    dims[0] -= 2;
    dims[2] += 1;

    unsigned size = std::accumulate(dims.begin(), dims.end(),1,std::multiplies<unsigned>());
    retrieved.resize(size);
    
    for(unsigned i = 0;i<retrieved.size();++i)
      retrieved[i] = i;

  }

  void set_test_path(const std::string& _path){
    test_output_path = bfs::path(_path);
    test_output_name = test_output_path.string();
  }
};

struct indexed_helpers : public helpers_fixture {


  const bfs::path	index_path;
  
  indexed_helpers():
    helpers_fixture(),
    index_path("index.h5")
    
  {
    set_test_path("intermediate/timepoint_n.h5");
  }

  
};

#endif /* _HDF5_FIXTURES_H_ */
