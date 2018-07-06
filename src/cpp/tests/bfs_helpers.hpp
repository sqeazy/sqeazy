#ifndef _BFS_HELPERS_H_
#define _BFS_HELPERS_H_

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3

#include "boost/filesystem.hpp"


namespace bfs = boost::filesystem;

namespace helpers {

	bool find_file(const bfs::path &dir_path, const bfs::path &file_name, bfs::path &path_found)
  {
    const bfs::recursive_directory_iterator end;
    const auto it = std::find_if(bfs::recursive_directory_iterator(dir_path), end, [&file_name](const bfs::directory_entry &e) { return e.path().filename() == file_name; });
    if(it == end)
    {
      return false;
    }
    else
    {
      path_found = it->path();
      return true;
    }
  }
	
};

#endif

