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
#include "hist_impl.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

struct info
{


  static void csv_header(const std::string& _del, bool _bit_details = false) {

    std::cout << "filename"
              << _del << "bits_per_pixel"
              << _del << "shape"
              << _del << "min"
              << _del << "max"
              << _del << "mean"
              << _del << "stddev"
              << _del << ".05*quant"
              << _del << ".25*quant"
              << _del << "median"
              << _del << ".75*quant"
              << _del << ".95*quant"
      ;

    if(_bit_details){
      std::cout << _del << "drange"
                << _del << "lsb";
    }

    std::cout << "\n";

  };

  template <typename T, typename S>
  static void csv_print(const std::vector<T>& _payload,
                        const std::vector<S>& _shape,
                        const std::string& _del, bool _bit_details = false) {


    float min = (*std::min_element(_payload.begin(), _payload.end()));
    float max = (*std::max_element(_payload.begin(), _payload.end()));

    std::ostringstream shape_str;
    int cnt = 0;
    for( const S& el : _shape ){
      cnt++;
      shape_str << el;
      if(cnt!=_shape.size())
        shape_str << "x";
    }

    std::cout << _del << sizeof(T)*CHAR_BIT
              << _del << shape_str.str()
              << _del << min
              << _del << max;

    sqeazy::histogram<T> hist(_payload.cbegin(), _payload.cend());
    std::cout.precision(4);

    std::cout << _del << hist.mean()
              << _del << hist.mean_variation()
              << _del << hist.calc_support(.05)
              << _del << hist.calc_support(.25)
              << _del << hist.median()
              << _del << hist.calc_support(.75)
              << _del << hist.calc_support(.95);

    std::cout.precision(6);

    if(_bit_details){

      static const double base2 = 1/std::log(2);

      double drange = 0;
      if(min!=max)
        drange = std::ceil(std::log(max - min + 1)*base2);

      int low_bit = sqeazy::lowest_set_bit(_payload.begin(), _payload.end());

      std::cout << _del << drange
                << _del << low_bit;
    }

    std::cout << "\n";

  };

};


int scan_files(const std::vector<std::string>& _files,
           const po::variables_map& _config) {


  int value = 1;
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

  info::csv_header(_config["delimiter"].as<std::string>(),_config.count("bit_details") );


  for(const std::string& _file : _files) {


    current_file = _file;

    std::cout << current_file.filename().c_str();//basename only

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
      info::csv_print(input.buffer_, input_dims,
                  _config["delimiter"].as<std::string>(),_config.count("bit_details"));
    }
    else if(bits_per_sample == 16){
      std::vector<uint16_t> buffer(input.size_in_byte()/sizeof(uint16_t));

      const uint16_t* begin = (const uint16_t*)&input.buffer_[0];
      const uint16_t* end = begin + buffer.size();
      std::copy(begin,end,buffer.begin());
      info::csv_print(buffer, input_dims,
                      _config["delimiter"].as<std::string>(),_config.count("bit_details"));
    }
    else {
      std::cerr << " yields unknown sampling, skipping ...\n";
      continue;
    }

  }

  value = 0;//FIXME
  return value;

}

#endif /* _SQY_SCAN_H_ */
