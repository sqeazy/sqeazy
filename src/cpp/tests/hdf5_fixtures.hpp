#ifndef _HDF5_FIXTURES_H_
#define _HDF5_FIXTURES_H_

#include "array_fixtures.hpp"
#include "bfs_helpers.hpp"
#include <algorithm>
#include <iostream>

namespace bfs = boost::filesystem;

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


struct helpers_fixture
{

  typedef unsigned short value_type;

  std::string tfile_basename;

  std::string tfile;
  bfs::path tpath;

  std::string dname;
  std::string test_output_name;
  bfs::path test_output_path;
  bfs::path no_filter_path;

  std::vector<unsigned short> retrieved;
  std::vector<size_t> dims;


  helpers_fixture()
      : tfile_basename("sample.h5")
      , tfile("")
      , tpath()
      , dname("IntArray")
      , test_output_name("hdf5_helpers.h5")
      , test_output_path(test_output_name)
      , no_filter_path("no_filter.h5")
      , retrieved()
      , dims(3, 8)
  {

    const bfs::path here = bfs::absolute(bfs::current_path());
    tpath = here;
    tpath /= tfile_basename;

    if(!bfs::exists(tpath))
    {
      tpath = here;
      tpath /= "tests";
      tpath /= tfile_basename;
    }

    if(!bfs::exists(tpath))
    {
      bfs::path search_path = here;
      search_path /= "tests";
      // bool found =
        helpers::find_file(search_path, tfile_basename, tpath);
    }

    if(!bfs::exists(tpath))
      std::cerr << "[helpers_fixture] unable to find tpath at " << tpath << "\n";

    tfile = tpath.string();

    dims[0] -= 2;
    dims[1] *= 16;  // to make SSE implementation happy
    dims[2] += 1;

    unsigned size = std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<unsigned>());
    retrieved.resize(size);

    for(unsigned i = 0; i < retrieved.size(); ++i)
      retrieved[i] = i;


    if(bfs::exists(test_output_path))
      bfs::remove(test_output_path);
    if(bfs::exists(no_filter_path))
      bfs::remove(no_filter_path);
  }

  void set_test_path(const std::string &_path)
  {
    test_output_path = bfs::path(_path);
    test_output_name = test_output_path.string();
  }
};

struct indexed_helpers : public helpers_fixture
{

  uint16_cube_of_8 data;
  const bfs::path index_file_path;
  const bfs::path level_path;
  std::vector<bfs::path> dataset_paths;
  std::vector<std::string> dataset_names;

  indexed_helpers()
      : helpers_fixture()
      , data()
      , index_file_path("index.h5")
      , level_path("intermediate")
      , dataset_paths(2)
      , dataset_names(2)
  {

    if(!bfs::exists(level_path))
      bfs::create_directory(level_path);

    if(bfs::exists(index_file_path))
      bfs::remove(index_file_path);

    for(unsigned i = 0; i < dataset_paths.size(); ++i)
    {


      std::stringstream fname;
      fname << "timepoint_" << i << ".h5";

      dataset_paths[i] = level_path;
      dataset_paths[i] /= fname.str();

      if(bfs::exists(dataset_paths[i]))
        bfs::remove(dataset_paths[i]);

      std::stringstream temp_dname;
      temp_dname << "/spim"
                 << "/" << dname;
      dataset_names[i] = temp_dname.str();
    }
  }

  void clean_up()
  {
    bfs::remove(index_file_path);
    bfs::remove_all(level_path);
    for(unsigned i = 0; i < dataset_paths.size(); ++i)
    {
      bfs::remove(dataset_paths[i]);
    }
  }
};

#endif /* _HDF5_FIXTURES_H_ */
