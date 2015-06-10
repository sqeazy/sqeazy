#ifndef _SQEAZY_HEADER_H_
#define _SQEAZY_HEADER_H_

#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <typeinfo>

#include "sqeazy_common.hpp"

namespace sqeazy {


  template <char delimiter, typename T>
  void split_string_to_vector(const std::string& _buffer, std::vector<T>& _v) {

    int num_del = std::count(_buffer.begin(), _buffer.end(), delimiter);
    if(_v.size() != size_t(num_del + 1))
      _v.resize(num_del+1);

    size_t begin = 0;
    size_t end = _buffer.find(delimiter);

    std::string token;

    for(size_t id = 0; id<_v.size(); ++id) {
      std::istringstream converter(_buffer.substr(begin,end - begin));
      converter >> _v[id];
      begin = end +1;
      end = _buffer.find(delimiter,begin);
    }

  }

  static unsigned sizeof_typeinfo(const std::string& _lit) {

    static std::map<std::string,int> type_size_map;
    type_size_map[typeid(short).name()] = sizeof(short);
    type_size_map[typeid(unsigned short).name()] = sizeof(unsigned short);
    type_size_map[typeid(char).name()] = sizeof(char);
    type_size_map[typeid(unsigned char).name()] = sizeof(unsigned char);
    type_size_map[typeid(int).name()] = sizeof(int);
    type_size_map[typeid(unsigned int).name()] = sizeof(unsigned int);
    type_size_map[typeid(long).name()] = sizeof(long);
    type_size_map[typeid(unsigned long).name()] = sizeof(unsigned long);

    std::map<std::string,int>::const_iterator found = type_size_map.find(_lit);
    if(found!=type_size_map.end())
      return found->second;
    else
      return 0;
  }

  template <typename T,char major_delim = ',', char minor_delim='x', char header_end_delim = '|'>
  struct image_header {

    static const char major_delimeter() { return major_delim; }
    static const char minor_delimeter() { return minor_delim; }
    static const char header_end_delimeter() { return header_end_delim; }
    
    typedef T value_type;

    std::string header;
    std::vector<unsigned> dims;
    std::string pipeline_;

    template <typename S>
    image_header(const std::vector<S>& _dims, const std::string& _pipe_name = "no_pipeline"):
      header(""),
      dims(_dims.begin(), _dims.end()),
      pipeline_(_pipe_name)
    {
      try{
	header = pack(dims,_pipe_name);
      }
      catch(...){
	std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
      }
    }


    image_header(unsigned long _size_in_byte, const std::string& _pipe_name = "no_pipeline"):
      header(""),
      dims(1),
      pipeline_(_pipe_name)
    {
      dims[0] = _size_in_byte;
      try{
	header = pack(dims,_pipe_name);
      }
      catch(...){
	std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
      }
    }


    image_header(const std::string& _str):
      header(),
      dims(),
      pipeline_() {

      std::string::const_iterator header_end = std::find(_str.begin(), _str.end(), header_end_delim);
      header = std::string(_str.begin(), header_end + 1);

      try{
	unpack(header,dims, pipeline_);
      }
      catch(...){
	std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to unpack header (" << _str <<")!\n";
header = "";
}
    }

    template <typename Iter>
    image_header(Iter _begin, Iter _end):
      header(),
      dims(),
      pipeline_() {

      Iter header_end = std::find(_begin, _end, header_end_delim);
      header = std::string(_begin, header_end + 1);
      
      try{
	unpack(header,dims, pipeline_);
      }
      catch(...){
	std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to unpack header (" << header <<")!\n";
      }

    }

    ~image_header(){}
    
    unsigned size() const {

      return header.size();
    }

     std::vector<unsigned> const * shape() const {
      return &dims;
    }

    template <typename U>
    void shape(std::vector<U>& _dims) const {
      _dims = dims;
    }
    
    std::string str() const {

      return header;

    }

    std::string pipeline() const {

      return this->pipeline_;

    }


    unsigned long payload_size() const {

      unsigned long value = std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<unsigned>());
      return value;
    }

    unsigned long payload_size_byte() const {

      unsigned long size_of_type = sizeof(T);
      if(!header.empty()) {
	unsigned found_size_byte = sizeof_header_type();
	if(found_size_byte && found_size_byte!=size_of_type)
	  return found_size_byte*payload_size();
      }
      return payload_size()*size_of_type;
    }

    unsigned sizeof_header_type() const {
      unsigned value = 0;
      if(header.empty())
	return value;
      
      std::string type_literal = header.substr(header.find(major_delim)+1,1);
      value = sizeof_typeinfo(type_literal);
      return value;
    }

    // template <typename vtype>
    // static const std::string pack(const std::vector<vtype>& _dims) {

    //   std::stringstream header("");
      
    //   header << typeid(T).name() << major_delim
    // 	     << _dims.size() << major_delim;
    //   for(unsigned i = 0; i<_dims.size(); ++i) {
    // 	if(i!=(_dims.size()-1))
    // 	  header << _dims[i] << minor_delim;
    // 	else
    // 	  header << _dims[i];
    //   }
      
    //   header << header_end_delim;
    //   return header.str();
    // }


    template <typename vtype>
    static const std::string pack(const std::vector<vtype>& _dims, const std::string& _pipe_name = "empty_pipe") {

      std::stringstream header("");
      header << _pipe_name << major_delim ;
      header << typeid(T).name() << major_delim
    	     << _dims.size() << major_delim;

      if(_dims.size()>1){
      for(unsigned i = 0; i<_dims.size(); ++i) {
    	if(i!=(_dims.size()-1))
    	  header << _dims[i] << minor_delim;
    	else
    	  header << _dims[i];
      }
      } else {
	header << _dims[0] ;
      }
      
      int final_size_in_byte = header.str().size() + 1;
      if(final_size_in_byte % sizeof(T) != 0){
	int n_spaces_to_fill = final_size_in_byte % sizeof(T);
	for(int i = 0;i<n_spaces_to_fill;++i)
	  header << " ";
      }
	
      header << header_end_delim;

      return header.str();
    }

    static const bool valid_header(const std::string& _hdr){

      return std::count(_hdr.begin(),_hdr.end(), major_delim) == 3;

    }

    template <typename Iter>
    static const bool valid_header(Iter _begin, Iter _end){

      Iter header_end = std::find(_begin, _end, header_end_delim);
      
      if(header_end != _end){
	std::string hdr = std::string(_begin, header_end + 1);
	return std::count(hdr.begin(),hdr.end(), major_delim) == 3;}
      else
	return false;

    }

    static const void unpack(const std::string& _buffer, std::vector<unsigned>& _shape, std::string& _pipe_name ) {

      return unpack(&_buffer[0], _buffer.size(), _shape, _pipe_name);
    }

    static const void unpack(const char* _buffer, const unsigned& _size, std::vector<unsigned>& _shape, std::string& _pipe_name) {

      const char* header_end_ptr = std::find(_buffer, _buffer + _size, header_end_delim);

      //let's omit the header_end_ptr to make splitting easier
      std::string header(_buffer, header_end_ptr);
      
      if(!valid_header(header)){
	std::ostringstream msg;
	msg << "[image_header::unpack]\t received header ("<< header <<") does not comply expected format\n";
	throw std::runtime_error(msg.str().c_str());
      }
      
      std::vector<std::string> fields;
      split_string_to_vector<major_delim>(header, fields);
      _pipe_name = fields.front();
      split_string_to_vector<minor_delim>(fields.back(), _shape);

    }

    static const std::vector<unsigned> unpack_shape(const char* _buffer, const unsigned& _size) {

      const char* header_end_ptr = std::find(_buffer, _buffer + _size, header_end_delim);
      //let's omit the header_end_ptr to make splitting easier
      std::string header(_buffer, header_end_ptr);
      
      if(!valid_header(header)){
	std::ostringstream msg;
	msg << "[image_header::unpack_shape]\t received header ("<< header <<") does not comply expected format\n";
	throw std::runtime_error(msg.str().c_str());
      }

      std::vector<std::string> fields;
      split_string_to_vector<major_delim>(header, fields);
      
      std::vector<unsigned> value;
      split_string_to_vector<minor_delim>(fields.back(), value);
      return value;
    }


    static const int unpack_num_dims(const char* _buffer, const unsigned& _size) {

      const char* header_end_ptr = std::find(_buffer, _buffer + _size, header_end_delim);
      //let's omit the header_end_ptr to make splitting easier
      std::string in_buffer(_buffer, header_end_ptr);

      if(!valid_header(in_buffer)){
	std::ostringstream msg;
	msg << "[image_header::unpack_shape]\t received header ("<< in_buffer <<") does not comply expected format\n";
	throw std::runtime_error(msg.str().c_str());
      }

      int value = std::count(in_buffer.begin(),in_buffer.end(), minor_delim) + 1;
      return value;
    }
  };

};

#endif /* _SQEAZY_HEADER_H_ */
