#ifndef _SQY_COMPRESS_H_
#define _SQY_COMPRESS_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "tiff_utils.hpp"
//#include "deprecated/static_pipeline_select.hpp"
#include "sqeazy_algorithms.hpp"
#include "sqeazy_pipelines.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "sqeazy_algorithms.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace sqy = sqeazy;

void compress_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  
  
  bfs::path			current_file;
  bfs::path			output_file;
  std::fstream			sqyfile;
  std::vector<size_t>		input_shape;
  sqeazy::tiff_facet		input;
  
  sqy::dypeline<std::uint16_t>	pipe16;
  sqy::dypeline_from_uint8	pipe8;

  std::vector<char>		output;
  unsigned long			expected_size_byte = 0;

  const std::string pipeline_string = _config["pipeline"].as<std::string>();
  
  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline_string)){
    std::cerr << "unable to build pipeline from " << pipeline_string << "\nDoing nothing.\n";
    return;
  }

  pipe16 = sqy::dypeline<std::uint16_t>::from_string(pipeline_string);
  pipe8 = sqy::dypeline_from_uint8::from_string(pipeline_string);
  
  unsigned long compressed_length_byte = 0;
  char* enc_end = nullptr;
  
  for(const std::string& _file : _files) {

    enc_end = nullptr;
    current_file = _file;
    if(!bfs::exists(current_file)){
      std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
      continue;
    }

    ////////////////////////INPUT I/O///////////////////////////////
    //load tiff & extract the dimensions
    input.load(_file);
       
    //compute the maximum size of the output buffer
    input.dimensions(input_shape);

    if(input.bits_per_sample()==16)
      expected_size_byte = pipe16.max_encoded_size(input.size_in_byte());
    else{
      if(input.bits_per_sample()==8)
	expected_size_byte = pipe8.max_encoded_size( input.size_in_byte());
      else{
	std::cerr << "only 8 or 16-bit encoding support yet, skipping "<< _file<<"\n";
	continue;
      }
    }
    
    //create clean output buffer
    if(expected_size_byte!=output.size())
      output.resize(expected_size_byte);
    
    std::fill(output.begin(), output.end(),0);

    ////////////////////////COMPRESS///////////////////////////////
    //compress
    if(input.bits_per_sample()==16)
      enc_end = pipe16.encode(reinterpret_cast<const std::uint16_t*>(input.data()),
			      output.data(),
			      input_shape);

    if(input.bits_per_sample()==8)
      enc_end = pipe8.encode(reinterpret_cast<const std::uint8_t*>(input.data()),
			     output.data(),
			     input_shape);

    
    if(enc_end == nullptr && _config.count("verbose")) {
      std::cerr << "compression failed! Nothing to write to disk...\n";
      continue;
    }

    ////////////////////////OUTPUT I/O///////////////////////////////
    compressed_length_byte = enc_end - output.data();
      
    output_file = current_file.replace_extension(".sqy");
    sqyfile.open(output_file.generic_string(), std::ios_base::binary | std::ios_base::out );
    if(!sqyfile.is_open())
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	sqyfile.clear();
	continue;
      }
    
    
    sqyfile.write(output.data(),compressed_length_byte);
    sqyfile.close();

  }

}


#endif /* _SQY_COMPRESS_H_ */
