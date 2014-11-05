#ifndef _PIPELINE_SELECT_HPP_
#define _PIPELINE_SELECT_HPP_
#include <map>
#include <functional>
#include <utility>
#include <sstream>
#include <stdexcept>
#include "bench_common.hpp"

namespace sqeazy_bench {

  //
  //FIXME: this selection can be done through boost::variant 
  //
  struct compress_select {

    typedef std::pair<int, std::string> spec_t;
    typedef std::function<unsigned long(unsigned long)> max_encoded_size_ftype;
    typedef std::map<spec_t, max_encoded_size_ftype> max_map_t;
    
    typedef std::function<int(const unsigned char*, char*, std::vector<unsigned>&, unsigned long&)> compress8_ftype;
    typedef std::function<int(const unsigned short*, char*, std::vector<unsigned>&, unsigned long&)> compress16_ftype;
    typedef std::map<std::string, compress8_ftype> compress8_map_t;
    typedef std::map<std::string, compress16_ftype> compress16_map_t;

    static max_map_t max_encoded_size_map_;

    static compress8_map_t compress8_map_;
    static compress16_map_t compress16_map_;

    spec_t current_;

    static max_map_t fill_max_map(){
      
      max_map_t value;
      value[std::make_pair(8, char_rmbkg_bswap1_lz4_pipe::name())] = compress_select::max_encoded_size_ftype(char_rmbkg_bswap1_lz4_pipe::max_encoded_size<unsigned long>);
      value[std::make_pair(8, char_bswap1_lz4_pipe::name())] = compress_select::max_encoded_size_ftype(char_bswap1_lz4_pipe::max_encoded_size<unsigned long>);
      value[std::make_pair(16, rmbkg_bswap1_lz4_pipe::name())] = compress_select::max_encoded_size_ftype(rmbkg_bswap1_lz4_pipe::max_encoded_size<unsigned long>);
      value[std::make_pair(16, bswap1_lz4_pipe::name())] = compress_select::max_encoded_size_ftype(bswap1_lz4_pipe::max_encoded_size<unsigned>);
      return value;

    }

    static compress8_map_t fill_compress8(){
      
      compress8_map_t value;
      value[char_rmbkg_bswap1_lz4_pipe::name()] = compress_select::compress8_ftype(char_rmbkg_bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      return value;

    }

    static compress16_map_t fill_compress16(){
      
      compress16_map_t value;
      value[rmbkg_bswap1_lz4_pipe::name()] = compress_select::compress16_ftype(rmbkg_bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      return value;

    }


    compress_select(int _n_bits, std::string _pipe_name = ""):
      current_(std::make_pair(_n_bits, _pipe_name)){
            
    }


    compress_select(spec_t _spec = std::make_pair(0,"") ):
      current_(_spec){
            
    }
      
    max_encoded_size_ftype max_compressed_size()  {
      if(max_encoded_size_map_.find(current_)!=max_encoded_size_map_.end())
	return max_encoded_size_map_[current_];
      else{
	std::ostringstream msg;
	msg << "[compress_select]\t unable to retrieve functions for " << current_.second << " in " << current_.first << "-bit mode\n";
	throw std::runtime_error(msg.str().c_str());
      }
    }
    	           
    bool empty() const {
      
      return current_.first < 1 && current_.second.empty();
      
    }

    void set(spec_t _spec = std::make_pair(0,"")){
      current_ = _spec;
    }

    void set(int _n_bits, std::string _pipe_name = ""){
      current_ = std::make_pair(_n_bits, _pipe_name);
    }

    int compress(const char* _input, char* _output, std::vector<unsigned>& _dims, unsigned long& _num_encoded){
      
      std::ostringstream msg;
      int value = 0;
      switch(current_.first){

      case 8:
	if(compress8_map_.find(current_.second)!=compress8_map_.end()){
	  value = compress8_map_[current_.second](reinterpret_cast<const unsigned char*>(_input), _output, _dims, _num_encoded);
	} else {
	  msg << "[compress_select]\t unable to perform compress in for pipeline "<< current_.second <<"\n";
	  throw std::runtime_error(msg.str().c_str());
	}
	break;

      case 16:
	if(compress16_map_.find(current_.second)!=compress16_map_.end()){
	  value = compress16_map_[current_.second](reinterpret_cast<const unsigned short*>(_input), _output, _dims, _num_encoded);
	} else {
	  msg << "[compress_select]\t unable to perform compress in for pipeline "<< current_.second <<"\n";
	  throw std::runtime_error(msg.str().c_str());
	}
	break;
      default:
	msg << "[compress_select]\t unable to perform compress in " << current_.first << "-bit mode\n";
	throw std::runtime_error(msg.str().c_str());
      }

      return value;
    }
    
  };

  compress_select::max_map_t compress_select::max_encoded_size_map_ = compress_select::fill_max_map();
  compress_select::compress8_map_t compress_select::compress8_map_ = compress_select::fill_compress8();
  compress_select::compress16_map_t compress_select::compress16_map_ = compress_select::fill_compress16();
  // 


};
#endif /* _PIPELINE_SELECT_HPP_ */
