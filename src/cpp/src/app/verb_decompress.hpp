#ifndef _SQY_DECOMPRESS_H_
#define _SQY_DECOMPRESS_H_

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



void decompress_files(const std::vector<std::string>& _files,
		      const po::variables_map& _config) {


  std::string file_data;

  unsigned long long found_size_byte = 0;
  unsigned long long expected_size_byte = 0;
  bfs::path current_file;
  bfs::path output_file;
  std::fstream sqyfile;
  std::string found_pipeline;
  unsigned found_num_bits;
  sqeazy::pipeline_select<> dynamic;
  std::istringstream buf;
  sqeazy::tiff_facet	tiff;

  for(const std::string& _file : _files) {

    file_data.clear();
    found_pipeline = "unknown";
    tiff.shape_.clear();
    found_num_bits = 0;
    tiff.buffer_.clear();

    current_file = _file;

    //skip the rest if nothing was loaded
    if(!bfs::exists(current_file))
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	continue;
      }

    found_size_byte = bfs::file_size(current_file);
    sqyfile.open(_file, std::ios_base::binary | std::ios_base::in );

    file_data = std::string((std::istreambuf_iterator<char>(sqyfile)), 
			    std::istreambuf_iterator<char>());



    const char* file_ptr = &file_data[0];
    const char* end_header_ptr =  std::find(file_ptr, file_ptr + found_size_byte, 
					    sqeazy::image_header::header_end_delimeter());

    sqeazy::image_header sqy_header(file_ptr,
				    end_header_ptr+1
				    );
    tiff.shape_.resize(sqy_header.shape()->size());
    std::copy(sqy_header.shape()->begin(), sqy_header.shape()->end(),tiff.shape_.begin());
    found_pipeline = sqy_header.pipeline();
    found_num_bits = sqy_header.sizeof_header_type()*CHAR_BIT;
    dynamic.set(found_num_bits, found_pipeline);

    ///////////////////////////////////////////////////////////////
    // sqy file content specific

    //compute the maximum size of the output buffer
    expected_size_byte = dynamic.decoded_size_byte(file_ptr, sqy_header.size());

    if(expected_size_byte>tiff.buffer_.size())
      tiff.buffer_.resize(expected_size_byte);
        
    //create clean output buffer
    std::fill(tiff.buffer_.begin(), tiff.buffer_.end(),0);

    int dec_ret = dynamic.decompress(&file_data[0],&tiff.buffer_[0],
				     found_size_byte);
	
    if(dec_ret && _config.count("verbose")) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".tif");
    
    tiff.write(output_file.generic_string(), found_num_bits);

    sqyfile.close();
  }

}

#endif /* _SQY_DECOMPRESS_H_ */
