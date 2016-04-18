#ifndef _SQY_SCAN_H_
#define _SQY_SCAN_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include <boost/program_options.hpp>

#include "tiff_utils.hpp"
#include "deprecated/static_pipeline_select.hpp"
#include "sqeazy_algorithms.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "sqeazy_algorithms.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

struct info{


  static void header(const int& _space_left = 30) {

    std::cout << std::setw(_space_left) << "filename"
             << std::setw(8) << "min"
             << std::setw(8) << "max"
             << std::setw(8) << "drange"
             << std::setw(8) << "lsb"
             << "\n";

  };

  template <typename T>
  static void print(const std::vector<T>& _payload,
                   const po::variables_map& _config) {


    T min = *std::min_element(_payload.begin(), _payload.end());
    T max = *std::max_element(_payload.begin(), _payload.end());

    static const double base2 = 1/std::log(2);
    
    double drange = 0;
    if(min!=max)
      drange = std::ceil(std::log(max - min + 1)*base2);

    int low_bit = sqeazy::lowest_set_bit(_payload.begin(), _payload.end());
    
    std::cout << std::setw(8) << min
             << std::setw(8) << max
             << std::setw(8) << drange
             << std::setw(8) << low_bit
             << "\n";

  };

};


void scan_files(const std::vector<std::string>& _files,
		const po::variables_map& _config) {


  bfs::path	current_file;
  bfs::path	output_file;

  std::vector<unsigned>		input_dims;
  sqeazy::tiff_facet	input;
  
  int bits_per_sample = 0;


  //TODO: refactor this
  int max_file_name_size = 0;
  for(const std::string& _file : _files) {
    current_file = _file;
    int current_size = current_file.filename().generic_string().size();
    if(current_size > max_file_name_size)
      max_file_name_size = current_size;
  }

  max_file_name_size *= 1.1;
  
  info::header(max_file_name_size);
    
  
  for(const std::string& _file : _files) {

    
    current_file = _file;

    std::cout << std::setw(max_file_name_size) << current_file.filename().c_str();//basename only
    
    if(!bfs::exists(current_file)){
      std::cerr << " cannot be opened as it does not exist\n";
      continue;
    }


    //load tiff & extract the dimensions
    input.load(_file);
       
    //compute the maximum size of the output buffer
    input.dimensions(input_dims);

    bits_per_sample = input.bits_per_sample();

    if(bits_per_sample == 8){
      info::print(input.buffer_,  _config);
    }
    else if(bits_per_sample == 16){
      std::vector<uint16_t> buffer(input.size_in_byte()/sizeof(uint16_t));

      const uint16_t* begin = (const uint16_t*)&input.buffer_[0];
      const uint16_t* end = begin + buffer.size();
      std::copy(begin,end,buffer.begin());
      info::print(buffer, _config);
    }
    else {
      std::cerr << " yields unknown sampling, skipping ...\n";
      continue;
    }
      
  }

}

#endif /* _SQY_SCAN_H_ */
