#ifndef _SQY_COMPRESS_H_
#define _SQY_COMPRESS_H_

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


void compress_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  
  std::vector<char>	output_data;
  unsigned long			expected_size_byte = 0;
  bfs::path	current_file;
  bfs::path	output_file;
  std::fstream			sqyfile;
  std::vector<unsigned>		input_dims;
  sqeazy::tiff_facet	input;
  sqeazy::pipeline_select<>	dynamic;
  
  unsigned long compressed_length_byte = 0;
  int enc_ret = 0;

  for(const std::string& _file : _files) {

    
    current_file = _file;
    if(!bfs::exists(current_file)){
      std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
      continue;
    }

    //load tiff & extract the dimensions
    input.load(_file);
       
    //compute the maximum size of the output buffer
    input.dimensions(input_dims);
    dynamic.set(input.bits_per_sample(),_config["pipeline"].as<std::string>());

    ///////////////////////////////////////////////////////////////
    // image type specific
    
    expected_size_byte = dynamic.max_compressed_size(input.size_in_byte());

    //create clean output buffer
    if(expected_size_byte!=output_data.size())
      output_data.resize(expected_size_byte);
    
    std::fill(output_data.begin(), output_data.end(),0);
    
    //compress
    enc_ret = dynamic.compress(input.data(),
			      &output_data[0],
			      input_dims, 
			      compressed_length_byte);
    
    if(enc_ret && _config.count("verbose")) {
      std::cerr << "compression failed! Nothing to write to disk...\n";
      continue;
    }
    ///////////////////////////////////////////////////////////////

    output_file = current_file.replace_extension(".sqy");
    sqyfile.open(output_file.generic_string(), std::ios_base::binary | std::ios_base::out );
    if(!sqyfile.is_open())
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	sqyfile.clear();
	continue;
      }
    
    
    sqyfile.write(&output_data[0],compressed_length_byte);
    sqyfile.close();

  }

}


#endif /* _SQY_COMPRESS_H_ */
