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
#include "hdf5_utils.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace sqy = sqeazy;

template <typename pipe_t>
size_t native_compress_write(const sqeazy::tiff_facet& _input,
			  pipe_t& _pipeline,
			  bfs::path& _output_file,
			  bool _verbose = false){

  typedef typename pipe_t::incoming_t raw_t;

  std::vector<size_t>	input_shape;
  std::vector<char>	output;
  size_t		expected_size_byte = _pipeline.max_encoded_size(_input.size_in_byte());
  size_t                output_file_size = bfs::exists(_output_file) ? bfs::file_size(_output_file) : 0;
  size_t		bytes_written =0;
    
  //create clean output buffer
  if(expected_size_byte!=output.size())
    output.resize(expected_size_byte);
  
  std::fill(output.begin(), output.end(),0);

  //retrieve the size of the loaded buffer
  _input.dimensions(input_shape);

  //compress  
  char* enc_end = _pipeline.encode(reinterpret_cast<const raw_t*>(_input.data()),
				   output.data(),
				   input_shape);

  if(enc_end == nullptr && _verbose) {
    std::cerr << "native compression failed! Nothing to write to disk...\n";
    return 1;
  }

  ////////////////////////OUTPUT I/O///////////////////////////////
  const size_t compressed_length_byte = enc_end - output.data();
      
  std::fstream sqyfile;
  sqyfile.open(_output_file.generic_string(), std::ios_base::binary | std::ios_base::out );
  if(!sqyfile.is_open())
    {
      std::cerr << "[SQY]\tunable to open " << _output_file << "as output file. Skipping it!\n";
      sqyfile.clear();
      return 0;
    }
  
    
  sqyfile.write(output.data(),compressed_length_byte);
  sqyfile.close();

  bytes_written = bfs::file_size(_output_file) - output_file_size;

  if(bfs::file_size(_output_file)==output_file_size)
    return output_file_size;
  else
    return bytes_written;
}

template <typename pipe_t>
size_t h5_compress_write(const sqeazy::tiff_facet& _input,
			  pipe_t& _pipeline,
			  bfs::path& _output_file,
			  bool _verbose = false){

  typedef typename pipe_t::incoming_t raw_t;

  std::vector<size_t>	input_shape;
  std::vector<char>	output;
  size_t		expected_size_byte = _pipeline.max_encoded_size(_input.size_in_byte());
  size_t                output_file_size = bfs::exists(_output_file) ? bfs::file_size(_output_file) : 0;
  size_t		bytes_written =0;

  //retrieve the size of the loaded buffer
  _input.dimensions(input_shape);
  sqy::h5_file loaded(_output_file.generic_string(), bfs::exists(_output_file) ? H5F_ACC_RDWR : H5F_ACC_TRUNC);
  std::string dname = _output_file.stem().generic_string();
  
  int rvalue = 0;
  if(!loaded.ready())
    return rvalue;
  else{
    rvalue = loaded.write_nd_dataset(dname,
				     reinterpret_cast<const raw_t*>(_input.data()),
				     input_shape.data(),
				     input_shape.size(),
				     _pipeline);
      
  }

  if(!rvalue)
    return 0;
  
  bytes_written = bfs::file_size(_output_file) - output_file_size;//applies if created from scratch or appended

  if(bfs::file_size(_output_file)==output_file_size)
    return output_file_size;//rewrite
  else
    return bytes_written;
}

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
    size_t bytes_written = 0;
    
    output_file = current_file.replace_extension(".sqy");

    ////////////////////////PRODUCE TARGET FILE NAME///////////////////////////////
    if(_config.count("output_suffix")){
      auto new_suffix = _config["output_suffix"].as<std::string>();

      if(new_suffix.front() == '.')
	output_file = current_file.replace_extension(_config["output_suffix"].as<std::string>());
      else{
	output_file = current_file.stem();
	output_file += new_suffix;
      }
	

    }

    if(_config.count("output_name") && _files.size()==1)
      output_file = _config["output_name"].as<std::string>();


    ////////////////////////COMPRESS & WRITE///////////////////////////////
    if(output_file.extension()==".sqy"){
      if(input.bits_per_sample()==16){
	bytes_written = native_compress_write(input,
					      pipe16,
					      output_file,
					      _config.count("verbose"));
      }
      else{
	bytes_written = native_compress_write(input,
					      pipe8,
					      output_file,
					      _config.count("verbose"));
    
      }
    }

    if(output_file.extension()==".h5"){
      
	if(input.bits_per_sample()==16){
	  bytes_written = h5_compress_write(input,
					    pipe16,
					    output_file,
					    _config.count("verbose"));
	}
	else{
	  bytes_written = h5_compress_write(input,
					    pipe8,
					    output_file,
					    _config.count("verbose"));
	  
	}
    }
    
    if(!bytes_written){
      std::cerr << "[SQY]\terrors occurred while processing " << _file << "\n";
    }

  }

}


#endif /* _SQY_COMPRESS_H_ */
