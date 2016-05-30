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
  std::vector<char> intermediate_buffer;
  std::vector<size_t> shape;
  
  std::uintmax_t file_size_byte = 0;
  std::vector<size_t> file_shape;
  std::uintmax_t expected_size_byte = 0;
  
  bfs::path current_file;
  bfs::path output_file;
  std::fstream sqyfile;
  std::string found_pipeline;
  
  sqy::dypeline<std::uint16_t>	pipe16;
  sqy::dypeline_from_uint8	pipe8;
  // sqeazy::pipeline_select<> dynamic;
  
  std::istringstream buf;
  size_t found_num_bits;

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
    file_shape.clear();
    
    
    if(current_file.extension()==".sqy"){
      sqyfile.open(_file, std::ios_base::binary | std::ios_base::in );

      file_content_buffer = std::string((std::istreambuf_iterator<char>(sqyfile)), 
					std::istreambuf_iterator<char>());


      

      ////////////////////////EXTRACT HEADER///////////////////////////////
      const char* file_ptr = &file_content_buffer[0];
      
      sqeazy::image_header sqy_header(file_ptr,
				      file_ptr+file_size_byte
				      );
      file_shape.resize(sqy_header.shape()->size(),1);
      file_shape[sqy::row_major::x] = file_size_byte;
    
      std::string found_pipeline = sqy_header.pipeline();
      found_num_bits = sqy_header.sizeof_header_type()*CHAR_BIT;
      if(!(found_num_bits==16 || found_num_bits==8))
	{
	  std::cerr << "[SQY]\tonly 8 or 16-bit encoding support yet, skipping "<< _file<<"\n";
	  continue;
	}

      //prepare for tiff output
      expected_size_byte = sqy_header.raw_size_byte();

    
      if(intermediate_buffer.size()<expected_size_byte)
	intermediate_buffer.resize(expected_size_byte);
      ////////////////////////DECODE///////////////////////////////
      shape = *sqy_header.shape();
    
      int dec_ret = 1;
      if(found_num_bits == 16){
	if(!sqy::dypeline<std::uint16_t>::can_be_built_from(sqy_header.pipeline())){
	  std::cerr << "[SQY]\tunable to build pipeline from " << sqy_header.pipeline() << "\nDoing nothing on "<< _file <<".\n";
	  continue;
	}
	
	pipe16 = sqy::dypeline<std::uint16_t>::from_string(sqy_header.pipeline());
      
	dec_ret = pipe16.decode(file_ptr,
				reinterpret_cast<std::uint16_t*>(intermediate_buffer.data()),
				file_shape,
				*sqy_header.shape());
      }

      if(found_num_bits == 8){
	if(!sqy::dypeline_from_uint8::can_be_built_from(sqy_header.pipeline())){
	  std::cerr << "[SQY]\tunable to build pipeline from " << sqy_header.pipeline() << "\nDoing nothing on "<< _file <<".\n";
	  continue;
	}
	
	pipe8 = sqy::dypeline_from_uint8::from_string(sqy_header.pipeline());
      
	dec_ret = pipe8.decode(file_ptr,
			       reinterpret_cast<std::uint8_t*>(intermediate_buffer.data()),
			       file_shape,
			       *sqy_header.shape());
      } 


      
      if(dec_ret) {
	std::cerr << "[SQY]\tdecompressing "<< _file <<" failed! Nothing to write to disk...\n";
	continue;
      }

      sqyfile.close();
      
    } else {
      
      sqy::h5_file loaded(current_file.c_str());
      if(!loaded.ready()){
	std::cerr << "[SQY]\tunable to load " << _file << "\n";
	continue;
      }
      
      const std::string dname = _config["dataset_name"].as<std::string>();

      //FIXME: add flag for dataset name
      if(!loaded.has_h5_item(dname)){
	std::cerr << "[SQY]\tunable to load " << _file << ":"<< dname <<"\n";
	continue;
      }

      
      loaded.shape(shape, dname);

      int sizeoftype = loaded.type_size_in_byte(dname);
      found_num_bits = sizeoftype*CHAR_BIT;
      expected_size_byte = std::accumulate(shape.begin(), shape.end(),
					   sizeoftype,
					   std::multiplies<size_t>());

      if(intermediate_buffer.size()<expected_size_byte)
	intermediate_buffer.resize(expected_size_byte);
      
      int rvalue = 1;

      if(found_num_bits == 8)
	rvalue = loaded.read_nd_dataset(dname,
					intermediate_buffer.data(),
					shape);
      else if(found_num_bits == 16){
	rvalue = loaded.read_nd_dataset(dname,
					reinterpret_cast<std::uint16_t*>(intermediate_buffer.data()),
					shape);
      }
	
      
      if(rvalue) {
	std::cerr << "[SQY]\tdecompressing "<< _file<<" failed! Nothing to write to disk...\n";
	continue;
      }
    }
      
    
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

    sqeazy::tiff_facet	tiff(output_file.string());
    tiff.shape_.resize(shape.size());
    std::copy(shape.begin(), shape.end(),
	      tiff.shape_.begin());
    
    if(expected_size_byte>tiff.buffer_.size())
      tiff.buffer_.resize(expected_size_byte);
    
    std::copy(intermediate_buffer.begin(),intermediate_buffer.end(),tiff.buffer_.begin());
    
    ////////////////////////OUTPUT I/O///////////////////////////////
    tiff.write(output_file.generic_string(),
	       found_num_bits);

    
  }

}

#endif /* _SQY_DECOMPRESS_H_ */
