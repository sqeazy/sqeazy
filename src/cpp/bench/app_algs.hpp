#ifndef _APP_ALGS_H_
#define _APP_ALGS_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include <boost/program_options.hpp>

//#include "bench_fixtures.hpp"
//#include "bench_utils.hpp"
//#include "bench_common.hpp"
#include "tiff_utils.hpp"
#include "pipeline_select.hpp"
#include "sqeazy_algorithms.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "quantiser_impl.hpp"
#include "sqeazy_algorithms.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;


void compare_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  if(_files.size()!=2){
    std::cerr << "number of files given is unequal 2 (received: "
	      << _files.size() <<")" << std::endl;
    return;
  }
      
  const bfs::path	src_file = _files[0];
  const bfs::path	src_file_extension = src_file.extension();
  
  const bfs::path	target_file = _files[1];
  const bfs::path	target_file_extension = target_file.extension();
  const bfs::path	target_file_stem = target_file.parent_path()/target_file.stem();//TODO

  
  std::vector<uint32_t>		src_input_dims;
  sqeazy_bench::tiff_facet	src_input;

  std::vector<uint32_t>		target_input_dims;
  sqeazy_bench::tiff_facet	target_input;

  std::map<std::string,float> results;
  
  if(!bfs::exists(src_file)){
    std::cerr << "[sqy-compare]\tunable to open " << src_file << "\tExiting.\n";
    return;
  }

  if(!bfs::exists(target_file)){
    std::cerr << "[sqy-compare]\tunable to open " << target_file << "\tExiting.\n";
    return;
  }

  if(!_config.count("metrics")){
    std::cerr << "[sqy-compare]\tno metrics received\tExiting.\n";
    return;
  }
    
  
  if(src_file_extension.generic_string().find("tif")!=std::string::npos &&
     target_file_extension.generic_string().find("tif")!=std::string::npos){
    
    //load tiff & extract the dimensions
    src_input.load(src_file.generic_string());
    src_input.dimensions(src_input_dims);

    target_input.load(target_file.generic_string());
    target_input.dimensions(target_input_dims);

    const uint32_t src_bits_per_sample = src_input.bits_per_sample();
    const uint32_t target_bits_per_sample = target_input.bits_per_sample();
    if(src_bits_per_sample!=target_bits_per_sample){
      std::cerr << "[SQY-compare]\t"
		<< src_file << " "<< src_bits_per_sample <<"-bit has different encoding as "
		<< target_file <<" " << target_bits_per_sample << "bit\n";
      return;
    }

    if(src_bits_per_sample==16){

      // std::vector<uint32_t> src_shape(src_input_dims.rbegin(), src_input_dims.rend());
      sqeazy::uint16_image_stack_cref src_stack_cref(reinterpret_cast<const uint16_t*>(src_input.data()),src_input_dims);

      // std::vector<uint32_t> target_shape(target_input_dims.rbegin(), target_input_dims.rend());
      sqeazy::uint16_image_stack_cref target_stack_cref(reinterpret_cast<const uint16_t*>(target_input.data()),target_input_dims);

      if(_config["metrics"].as<std::string>().find("all") != std::string::npos ){
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());
	results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				       target_stack_cref.data());
	results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
	results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }
	
      if(_config["metrics"].as<std::string>().find("mse") != std::string::npos )
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());

      if(_config["metrics"].as<std::string>().find("psnr") != std::string::npos )
	results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				       target_stack_cref.data());
      if(_config["metrics"].as<std::string>().find("drange") != std::string::npos ){
	results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
	results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }

    }

    if(src_bits_per_sample==8){

      // std::vector<uint32_t> src_shape(src_input_dims.rbegin(), src_input_dims.rend());
      sqeazy::uint8_image_stack_cref src_stack_cref((const uint8_t*)src_input.data(),src_input_dims);

      // std::vector<uint32_t> target_shape(target_input_dims.rbegin(), target_input_dims.rend());
      sqeazy::uint8_image_stack_cref target_stack_cref((const uint8_t*)target_input.data(),target_input_dims);

      if(_config["metrics"].as<std::string>().find("all") != std::string::npos ){
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());
	results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				       target_stack_cref.data());
	results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
	results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }

      if(_config["metrics"].as<std::string>().find("mse") != std::string::npos )
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());

      if(_config["metrics"].as<std::string>().find("psnr") != std::string::npos )
	results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				       target_stack_cref.data());

      if(_config["metrics"].as<std::string>().find("drange") != std::string::npos ){
	results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
	results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }
	
    }

  } else {
    std::cerr << "non-tif file format found, sqy-compare does not support anything else yet!\n";
    return;
  }

  
  for(const auto & k_v : results ){
    std::cout << std::setw(10) << k_v.first << "\t" << std::setw(15) << k_v.second << "\n";

  }
}


void convert_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  if(_files.size()!=2){
    std::cerr << "number of files given is unequal 2 (received: "
	      << _files.size() <<")" << std::endl;
    return;
  }
      
  const  bfs::path	src_file = _files[0];
  const  bfs::path	src_file_extension = src_file.extension();
  
  const bfs::path	target_file = _files[1];
  const bfs::path	target_file_extension = target_file.extension();
  const bfs::path	target_file_stem = target_file.parent_path()/target_file.stem();
  std::fstream			sqyfile;
  std::vector<uint32_t>		input_dims;
  sqeazy_bench::tiff_facet	input;
  
  if(!bfs::exists(src_file)){
    std::cerr << "[SQY]\tunable to open " << src_file << "\t Exit.\n";
    return;
  }


  if(src_file_extension.generic_string().find("tif")!=std::string::npos){
  
    //load tiff & extract the dimensions
    input.load(src_file.generic_string());
       
    //compute the maximum size of the output buffer, x,y,z
    input.dimensions(input_dims);

    const uint32_t bits_per_sample = input.bits_per_sample();
    if(bits_per_sample>16 || bits_per_sample<8){
      std::cerr << "[SQY]\t" << src_file << "contains unsupported encoding (16-bit, 8-bit) found: " <<bits_per_sample <<"bits\t Exit.\n";
      return;
    }

    std::vector<uint32_t> c_storage_order_shape(input_dims.begin(), input_dims.end());
    sqeazy::uint8_image_stack converted_stack(c_storage_order_shape);
  
    //convert
    if(input.bits_per_sample()==16){
    
      sqeazy::uint16_image_stack_cref loaded_stack(reinterpret_cast<const uint16_t*>(input.data()),c_storage_order_shape);

      //apply quantiser (dropping LUT)
      sqeazy::quantiser<uint16_t,uint8_t> shrinker;
      shrinker.encode(loaded_stack.data(),loaded_stack.num_elements(),converted_stack.data());
	
      bfs::path lut_path = target_file_stem;
      lut_path+=".lut";
      shrinker.lut_to_file(lut_path.generic_string(), shrinker.lut_decode_);
      
      if(_config.count("dump_quantiser")){
	bfs::path dump_path = lut_path;
	shrinker.dump(dump_path.replace_extension(".qdump").generic_string());
      }

    }

    if(input.bits_per_sample()==8){
      std::copy(input.data(), input.data() + input.size_in_byte(),converted_stack.data());
    }

    //do the i/o 
    if(target_file_extension.generic_string().find(".y4m")!=std::string::npos){
      sqeazy::write_stack_as_y4m(converted_stack,target_file_stem.generic_string(),_config.count("verbose"));
    }
    else
      sqeazy::write_stack_as_yuv(converted_stack,target_file_stem.generic_string(),_config.count("verbose"));

  } else {

    bfs::path lut_path = src_file.parent_path()/src_file.stem();
    lut_path+=".lut";
    
    const bool lut_found = bfs::exists(lut_path) && bfs::file_size(lut_path);

    if(_config.count("verbose"))
      std::cout << "lut" << (lut_found ? " " : " not ") << "found at " << lut_path
		<< (lut_found ? " assuming 16-bit encoding " : " assuming 8-bit encoding ")
		<< "\n";
	
    std::vector<uint8_t> buffer;
    std::vector<uint32_t> shape;
    
    if(sqeazy::is_y4m_file(src_file)){
      shape = sqeazy::read_y4m_to_gray8(buffer,
					src_file.generic_string(),
					_config.count("verbose"));
    }
    else if(sqeazy::is_yuv_file(src_file))
      shape = sqeazy::read_yuv_to_gray8(buffer,
					src_file.generic_string(),
					_config["frame_shape"].as<std::string>(),
					_config.count("verbose"));
    else
      {
	std::cerr << "source location is neither .y4m nor .yuv! Exiting ...\n";
	return;
      }

    if(buffer.empty()){
      std::cerr << "not data extracted from " << src_file << "! Exiiting ...\n";
      return;
    }



    if(lut_found)//most likely a 16-bit stack, let's decode it
      {
	std::vector<uint16_t> decoded(buffer.size());

	sqeazy::quantiser<uint16_t, uint8_t> shrinker;
	shrinker.decode(lut_path.generic_string(),
			&buffer[0],
			buffer.size(),
			&decoded[0]);

	sqeazy_bench::tiff_facet output(&decoded[0],&decoded[0] + decoded.size(),shape);
	output.write(target_file.generic_string(),16);
	
      } else {

      sqeazy_bench::tiff_facet output(&buffer[0],&buffer[0] + buffer.size(),shape);
      output.write(target_file.generic_string());

    }

    if(_config.count("verbose"))
      if(bfs::exists(target_file))
	std::cout << target_file << " written successfully.\n";
      else
	std::cerr << target_file << " written with errors of unknown cause!.\n";
  }

}


void compress_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  
  std::vector<char>	output_data;
  unsigned long			expected_size_byte = 0;
  bfs::path	current_file;
  bfs::path	output_file;
  std::fstream			sqyfile;
  std::vector<unsigned>		input_dims;
  sqeazy_bench::tiff_facet	input;
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
  sqeazy_bench::tiff_facet	tiff;

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
  sqeazy_bench::tiff_facet	input;
  
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

#endif /* _APP_ALGS_H_ */
