#ifndef _SQY_DIFF_H_
#define _SQY_DIFF_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include <boost/program_options.hpp>

#include "tiff_utils.hpp"
#include "sqeazy_algorithms.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "sqeazy_algorithms.hpp"
#include "hist_impl.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

struct diff_info
{


  static void header(const std::string& _reffile,
		     const po::variables_map& _config,
		     const int& _space_left = 30) {

    std::string first = "ref=";
      first += _reffile;

    std::cout << std::setw(_space_left) << first
      	      << _config["delimiter"].as<std::string>() << "mse"
	      << _config["delimiter"].as<std::string>() << "max"
      	      << _config["delimiter"].as<std::string>() << "min"
      	      << _config["delimiter"].as<std::string>() << "refmax"
      	      << _config["delimiter"].as<std::string>() << "refmin"
      ;
    
    std::cout << "\n";

  };

  template <typename T>
  static void print(const std::vector<T>& _left,
		    const std::vector<T>& _ref,
		    const po::variables_map& _config) {

    float mse = 0.f;
    for(std::size_t id = 0;id<_left.size();++id)
      {
	float temp = float(_left[id]) - _ref[id];
	mse += temp*temp;
      }

    T left_max  = *std::max_element(_left.begin(), _left.end());
    T left_min  = *std::min_element(_left.begin(), _left.end());

    T ref_max = *std::max_element(_ref.begin(), _ref.end());
    T ref_min = *std::min_element(_ref.begin(), _ref.end());
    
    std::cout.precision(4);
    std::cout << _config["delimiter"].as<std::string>() << mse/(_left.size()-1)
	      << _config["delimiter"].as<std::string>() << left_max
      	      << _config["delimiter"].as<std::string>() << left_min
      	      << _config["delimiter"].as<std::string>() << ref_max
      	      << _config["delimiter"].as<std::string>() << ref_min
      ;
    
    std::cout << "\n";
    std::cout.precision(6);

  };

};


int diff_files(const std::vector<std::string>& _files,
	       const po::variables_map& _config) {


  int value = 1;
  bfs::path	current_file = _files.front();
  bfs::path	output_file;
  std::vector<unsigned>		input_dims;
  sqeazy::tiff_facet	input;
  int bits_per_sample = 0;
  std::vector<uint16_t> buffer16;


  //load reference
  sqeazy::tiff_facet	ref;
  ref.load(_files.front());
  std::vector<unsigned> ref_dims;
  ref.dimensions(ref_dims);

  unsigned ref_bits_per_sample = ref.bits_per_sample();

  std::vector<std::uint16_t> refbuffer16(ref.size_in_byte()/sizeof(uint16_t));
  if(ref_bits_per_sample == 16){
    std::copy(ref.buffer_.data(),
	      ref.buffer_.data() + ref.size_in_byte(),
	      (char*)refbuffer16.data());
  }
      
  
  //checkout longest filename
  //TODO: refactor this
  int max_file_name_size = 0;
  for(const std::string& _file : _files) {
    current_file = _file;
    int current_size = current_file.filename().generic_string().size();
    if(current_size > max_file_name_size)
      max_file_name_size = current_size;
  }

  max_file_name_size *= 1.1;
  max_file_name_size += 4;
  
  diff_info::header(_files.front(),_config,max_file_name_size );
  
  for(std::size_t f = 1; f < _files.size();++f) {

    
    current_file = _files[f];

    std::cout << std::setw(max_file_name_size) << current_file.filename().c_str();//basename only
    
    if(!bfs::exists(current_file)){
      std::cerr << " cannot be opened as it does not exist\n";
      continue;
    }

    //load tiff & extract the dimensions
    input.load(_files[f]);
       
    //compute the maximum size of the output buffer
    input.dimensions(input_dims);

    bits_per_sample = input.bits_per_sample();

    if(bits_per_sample == 8){
      diff_info::print(input.buffer_, ref.buffer_,
		       _config);
    }
    else if(bits_per_sample == 16){
      buffer16.resize(input.size_in_byte()/sizeof(uint16_t));

      const uint16_t* begin = (const uint16_t*)&input.buffer_[0];
      const uint16_t* end = begin + buffer16.size();
      std::copy(begin,end,buffer16.begin());
      diff_info::print(buffer16, refbuffer16, _config);
    }
    else {
      std::cerr << " yields incompatible sampling (reference: "<< ref_bits_per_sample <<", found: "<< bits_per_sample<<"), skipping ...\n";
      continue;
    }
      
  }

  value = 0;//FIXME
  return value;

}

#endif /* _SQY_DIFF_H_ */
