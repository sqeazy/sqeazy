#ifndef _SQY_CONVERT_H_
#define _SQY_CONVERT_H_

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

void convert_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  if(_files.size()!=2){
    std::cerr << "number of files given is unequal 2 (received: "
	      << _files.size() <<")" << std::endl;
    return;
  }
      
  const  bfs::path	src_file = _files.front();
  const  bfs::path	src_file_extension = src_file.extension();
  
  const bfs::path	target_file = _files[1];
  const bfs::path	target_file_extension = target_file.extension();
  const bfs::path	target_file_stem = target_file.parent_path()/target_file.stem();
  std::fstream			sqyfile;
  std::vector<uint32_t>		input_dims;
  sqeazy::tiff_facet	input;
  
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
      if(_config["chroma_sampling"].as<std::string>() == sqeazy::yuv420formatter::y4m_code()){
      
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

    }

    if(input.bits_per_sample()==8)
      std::copy(input.data(), input.data() + input.size_in_byte(),converted_stack.data());
    
    
      //do the i/o

      if(target_file_extension.generic_string().find(".y4m")!=std::string::npos){
	if(_config["chroma_sampling"].as<std::string>() == sqeazy::yuv420formatter::y4m_code())
	  sqeazy::write_stack_as_y4m(converted_stack,target_file_stem.generic_string(),_config.count("verbose"));
	if(_config["chroma_sampling"].as<std::string>() == sqeazy::yuv444formatter::y4m_code()){

	  sqeazy::uint16_image_stack_cref stack_ref(reinterpret_cast<const uint16_t*>(input.data()),c_storage_order_shape);
	  
	  sqeazy::write_stack_as_y4m(stack_ref,
				     target_file_stem.generic_string(),
				     _config.count("verbose"),
				     sqeazy::yuv444formatter()
				     );}

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
    std::vector<uint16_t> decoded;
    
    if(sqeazy::is_y4m_file(src_file)){
      std::string y4m_header = sqeazy::y4m_header(src_file.generic_string());

      if(y4m_header.find("C420") != std::string::npos){
	shape = sqeazy::read_y4m_to_gray(buffer,
					 src_file.generic_string(),
					 _config.count("verbose"));}
      
      if(y4m_header.find("C444") != std::string::npos){
	shape = sqeazy::read_y4m_to_gray(decoded,
					 src_file.generic_string(),
					 _config.count("verbose"));
      }
      
    }
    else{
      if(sqeazy::is_yuv_file(src_file)){
	shape = sqeazy::read_yuv_to_gray8(buffer,
					  src_file.generic_string(),
					  _config["frame_shape"].as<std::string>(),
					  _config.count("verbose"));}
      else
	{
	  std::cerr << "source location is neither .y4m nor .yuv! Exiting ...\n";
	  return;
	}
    }

    if(buffer.empty() && decoded.empty()){
      std::cerr << "no data extracted from " << src_file << "! Exiting ...\n";
      return;
    }



    if(lut_found)//most likely a 16-bit stack, let's decode it
      {
	decoded.resize(buffer.size());

	sqeazy::quantiser<uint16_t, uint8_t> shrinker;
	shrinker.decode(lut_path.generic_string(),
			buffer.data(),
			buffer.size(),
			decoded.data());

      } 

    if(decoded.empty()){
      sqeazy::tiff_facet output(buffer.data(),buffer.data() + buffer.size(),shape);
      output.write(target_file.generic_string());
    }
    else {
      sqeazy::tiff_facet output(decoded.data(),decoded.data() + decoded.size(),shape);
      output.write(target_file.generic_string(),16);
    }



    if(_config.count("verbose")){
      if(bfs::exists(target_file))
	std::cout << target_file << " written successfully.\n";
      else
	std::cerr << target_file << " written with errors of unknown cause!.\n";
    }
  }

}


#endif /* _SQY_CONVERT_H_ */
