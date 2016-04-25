#ifndef _SQY_DECOMPRESS_H_
#define _SQY_DECOMPRESS_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "tiff_utils.hpp"
// #include "deprecated/static_pipeline_select.hpp"

#include "sqeazy_pipelines.hpp"
#include "sqeazy_algorithms.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"


namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace sqy = sqeazy;


void decompress_files(const std::vector<std::string>& _files,
		      const po::variables_map& _config) {


  std::string file_content_buffer;

  unsigned long long file_size_byte = 0;
  unsigned long long expected_size_byte = 0;
  bfs::path current_file;
  bfs::path output_file;
  std::fstream sqyfile;
  std::string found_pipeline;
  
  sqy::dypeline<std::uint16_t>	pipe16;
  sqy::dypeline_from_uint8	pipe8;
  // sqeazy::pipeline_select<> dynamic;
  
  std::istringstream buf;


  for(const std::string& _file : _files) {

    file_content_buffer.clear();
    current_file = _file;

    //skip the rest if nothing was loaded
    if(!bfs::exists(current_file))
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	continue;
      }

    ////////////////////////INPUT I/O///////////////////////////////
    file_size_byte = bfs::file_size(current_file);
    sqyfile.open(_file, std::ios_base::binary | std::ios_base::in );

    file_content_buffer = std::string((std::istreambuf_iterator<char>(sqyfile)), 
			    std::istreambuf_iterator<char>());


    ////////////////////////EXTRACT HEADER///////////////////////////////
    const char* file_ptr = &file_content_buffer[0];

    sqeazy::image_header sqy_header(file_ptr,
				    file_ptr+file_size_byte
				    );

    std::string found_pipeline = sqy_header.pipeline();
    size_t found_num_bits = sqy_header.sizeof_header_type()*CHAR_BIT;
    if(!(found_num_bits==16 || found_num_bits==8))
      {
	std::cerr << "only 8 or 16-bit encoding support yet, skipping "<< _file<<"\n";
	continue;
      }

    //prepare for tiff output
    expected_size_byte = sqy_header.raw_size_byte();

    if(_config.count("output_suffix"))
      output_file = current_file.replace_extension(_config["output_suffix"].as<std::string>());

    if(_config.count("output_name") && _files.size()==1)
      output_file = _config["output_name"].as<std::string>();

    
    sqeazy::tiff_facet	tiff(output_file.string());
    tiff.shape_.resize(sqy_header.shape()->size());
    std::copy(sqy_header.shape()->begin(), sqy_header.shape()->end(),tiff.shape_.begin());
    
    if(expected_size_byte>tiff.buffer_.size())
      tiff.buffer_.resize(expected_size_byte);
    
    std::fill(tiff.buffer_.begin(), tiff.buffer_.end(),0);

    ////////////////////////DECODE///////////////////////////////
    
    int dec_ret = 1;
    if(found_num_bits == 16){
      if(!sqy::dypeline<std::uint16_t>::can_be_built_from(sqy_header.pipeline())){
	std::cerr << "unable to build pipeline from " << sqy_header.pipeline() << "\nDoing nothing on "<< _file <<".\n";
	continue;
      }
	
      pipe16 = sqy::dypeline<std::uint16_t>::from_string(sqy_header.pipeline());
      
      dec_ret = pipe16.decode(file_ptr,
			      reinterpret_cast<std::uint16_t*>(&tiff.buffer_[0]),
			      tiff.shape_);
    }

    if(found_num_bits == 8){
      if(!sqy::dypeline_from_uint8::can_be_built_from(sqy_header.pipeline())){
	std::cerr << "unable to build pipeline from " << sqy_header.pipeline() << "\nDoing nothing on "<< _file <<".\n";
	continue;
      }
	
      pipe8 = sqy::dypeline_from_uint8::from_string(sqy_header.pipeline());
      
      dec_ret = pipe8.decode(file_ptr,
			     reinterpret_cast<std::uint8_t*>(&tiff.buffer_[0]),
			     tiff.shape_);
    } 


    
    if(dec_ret && _config.count("verbose")) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    ////////////////////////OUTPUT I/O///////////////////////////////
    tiff.write(output_file.generic_string(), found_num_bits);

    sqyfile.close();
  }

}

#endif /* _SQY_DECOMPRESS_H_ */
