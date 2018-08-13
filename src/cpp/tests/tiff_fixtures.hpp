#ifndef _TIFF_FIXTURES_H_
#define _TIFF_FIXTURES_H_
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "boost/multi_array.hpp"
#include <boost/filesystem.hpp>

#include <string>
#include <sstream>

#include "tiff_utils.hpp"
#include "header_utils.hpp"

namespace bfs = boost::filesystem;


namespace sqeazy {

struct data_interface {

  virtual char* bytes() = 0;

  virtual unsigned long size_in_byte() const = 0;

  virtual void fill_from(const std::string& _path) = 0;

};

template <typename T = unsigned short, bool verbose = true>
struct tiff_fixture : public data_interface {

  std::string file_loc;
  std::vector<T> tiff_data;
  std::vector<unsigned> axis_lengths;

  unsigned long axis_length(const int& _axis_ref) const {
    return axis_lengths.at(_axis_ref);
  }

  T* data(){

    return &tiff_data[0];

  }


  T const * data() const {

    return &tiff_data[0];

  }


  char* bytes(){

    return reinterpret_cast<char*>(&tiff_data[0]);

  }



  void fill_from(const std::string& _path){

    bfs::path cpath = _path;

    if(!bfs::exists(cpath) || _path.empty()){
      std::stringstream msg("");
      msg << "unable to load file at path " << _path << "\n";
      throw std::runtime_error(msg.str().c_str());
    }

    TIFF* stack_tiff   = TIFFOpen( _path.c_str() , "r" );
    std::vector<tdir_t> stack_tdirs   ;
    sqeazy::get_tiff_dirs(stack_tiff,   stack_tdirs  );

    axis_lengths = sqeazy::extract_tiff_to_vector(stack_tiff,   stack_tdirs  , tiff_data     );

    TIFFClose(stack_tiff);

    if(tiff_data.size()>0 && verbose){

      std::cout << *this << "successfully loaded \n";
    }

  }

  void swap(tiff_fixture& _first, tiff_fixture& _second){
    std::swap(_first.file_loc, _second.file_loc);
    std::swap(_first.tiff_data, _second.tiff_data);
    std::swap(_first.axis_length, _second.axis_length);
  }

  tiff_fixture():
    file_loc(""),
    tiff_data(),
    axis_lengths()
  {

  }

  tiff_fixture(const std::string& _fileloc):
    file_loc(_fileloc),
    tiff_data(),
    axis_lengths()
  {
    fill_from(_fileloc);
  }


  //copy-constructor: reinit everything (don't copy)
  tiff_fixture(const tiff_fixture& _rhs):
    file_loc(_rhs.file_loc),
    tiff_data(_rhs.tiff_data),
    axis_lengths(_rhs.axis_lengths){


  }


  tiff_fixture& operator=(tiff_fixture _rhs){

    swap(*this,_rhs);
    return *this;

  }

  unsigned long size_in_byte() const {
    return tiff_data.size()*sizeof(T);
  }


  unsigned long size() const {

    return tiff_data.size();

  }


  friend std::ostream& operator<<(std::ostream& _cout, const tiff_fixture& _self){
    _cout << "loaded " << _self.file_loc << " as ";
    for(size_t axis_id = 0;axis_id<_self.axis_lengths.size();++axis_id){
      _cout << _self.axis_length(axis_id) << ((axis_id < _self.axis_lengths.size()-1) ? "x" : " ");
    }
    _cout <<" size("<< sqeazy::header_utils::represent<T>::as_string() <<") = " << _self.size_in_byte()/(1<<20) << " MB ";
    return _cout;
  }

  bool empty() const {
    return tiff_data.empty();
  }
};

};

#endif
