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
#include "sqeazy_pipelines.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

/**
   \brief method to load data from file at _src and convert it back to _dst
   this method is meant for y4m/yuv to tif conversions
   
   \param[in] _src path of input file
   \param[in] _dst path of output file
   \param[in] _config command-line parameters
   
   \return 
   \retval 
   
*/
void backward_conversion(const bfs::path& _src,
			 const bfs::path& _dst,
			 const po::variables_map& _config) {


  
  const bfs::path	target_file_extension = _dst.extension();
  const bfs::path	target_file_stem = _dst.parent_path()/_dst.stem();
  
  std::fstream			sqyfile;
  std::vector<uint32_t>		input_dims;
  sqeazy::tiff_facet	input;
  if(!bfs::exists(_src)){
    std::cerr << "[SQY]\tunable to open " << _src << "\t Exit.\n";
    return;
  }

  bfs::path lut_path = _src.parent_path()/_src.stem();
  lut_path+=".lut";
    
  const bool lut_found = bfs::exists(lut_path) && bfs::file_size(lut_path);

  if(_config.count("verbose"))
    std::cout << "lut" << (lut_found ? " " : " not ") << "found at " << lut_path
	      << (lut_found ? " assuming 16-bit encoding " : " assuming 8-bit encoding ")
	      << "\n";
	
  std::vector<uint8_t> buffer;
  std::vector<uint32_t> shape;
  std::vector<uint16_t> decoded;
    
  if(sqeazy::is_y4m_file(_src)){
    std::string y4m_header = sqeazy::y4m_header(_src.generic_string());

    if(y4m_header.find("C420") != std::string::npos){
      shape = sqeazy::read_y4m_to_gray(buffer,
				       _src.generic_string(),
				       _config.count("verbose"));}
      
    if(y4m_header.find("C444") != std::string::npos){
      shape = sqeazy::read_y4m_to_gray(decoded,
				       _src.generic_string(),
				       _config.count("verbose"));
    }
      
  }
  else{
    if(sqeazy::is_yuv_file(_src)){
      shape = sqeazy::read_yuv_to_gray8(buffer,
					_src.generic_string(),
					_config["frame_shape"].as<std::string>(),
					_config.count("verbose"));}
    else
      {
	std::cerr << "source location is neither .y4m nor .yuv! Exiting ...\n";
	return;
      }
  }

  if(buffer.empty() && decoded.empty()){
    std::cerr << "no data extracted from " << _src << "! Exiting ...\n";
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
    output.write(_dst.generic_string());
  }
  else {
    sqeazy::tiff_facet output(decoded.data(),decoded.data() + decoded.size(),shape);
    output.write(_dst.generic_string(),16);
  }



  if(_config.count("verbose")){
    if(bfs::exists(_dst))
      std::cout << _dst << " written successfully.\n";
    else
      std::cerr << _dst << " written with errors of unknown cause!.\n";
  }
}

void insert_lut_path_if_not_present(std::string& quantiser_definition,
				    const bfs::path& lut_path){

  if(quantiser_definition.find("(")==std::string::npos){

    std::ostringstream args;
    args << "(decode_lut_path=" << lut_path << ")";
    quantiser_definition += args.str();
    return ;
    
  }

  if(quantiser_definition.find("lut_path")!=std::string::npos){
    return ;
  }

  std::ostringstream value;
  value << quantiser_definition.substr(0,quantiser_definition.size()-1);
  value << ",decode_lut_path=" << lut_path << ")"; 

  quantiser_definition = value.str();
}

/**
   \brief method to load data from file at _src and convert it back to _dst
   this method is meant for tif to y4m/yuv conversions
   
   \param[in] _src path of input file
   \param[in] _dst path of output file
   \param[in] _config command-line parameters
   
   \return 
   \retval 
   
*/
void forward_conversion(const bfs::path& _src,
			const bfs::path& _dst,
			const po::variables_map& _config) {

  const bfs::path	target_file_extension = _dst.extension();
  const bfs::path	target_file_stem = _dst.parent_path()/_dst.stem();


  
  std::fstream			sqyfile;
  std::vector<uint32_t>		input_dims;
  sqeazy::tiff_facet	input;
  
  if(!bfs::exists(_src)){
    std::cerr << "[SQY]\tunable to open " << _src << "\t Exit.\n";
    return;
  }

  if(!(target_file_extension == ".y4m" || target_file_extension == ".yuv")){
    std::cerr << "[SQY]\tunable to convert to anything else than y4m/yuv (received  " << target_file_extension << ")!\n";
    return;
  }

  //load tiff & extract the dimensions
  input.load(_src.generic_string());
       
  //compute the maximum size of the output buffer, x,y,z
  input.dimensions(input_dims);

  const std::uint32_t bits_per_sample = input.bits_per_sample();
  if(bits_per_sample>16 || bits_per_sample<8){
    std::cerr << "[SQY]\t" << _src << "contains unsupported encoding (16-bit, 8-bit) found: " <<bits_per_sample <<"bits\t Exit.\n";
    return;
  }

  std::vector<std::size_t> c_storage_order_shape(input_dims.begin(), input_dims.end());
  sqeazy::uint8_image_stack converted_stack(c_storage_order_shape);
    
  //convert using quantiser
  if(input.bits_per_sample()==16){
    
    bfs::path lut_path = _dst;
    lut_path.replace_extension(_config["lut_suffix"].as<std::string>());

    
    std::string quantiser_definition = _config["quantiser"].as<std::string>();
    insert_lut_path_if_not_present(quantiser_definition,lut_path);
    
    if(_config.count("verbose"))
      std::cout << "[SQY]\tUsing quantiser " << quantiser_definition << "\n";

    auto pipe16 = sqy::dypeline<std::uint16_t>::from_string(quantiser_definition);
    //apply quantiser (dropping LUT)
    if(_config["chroma_sampling"].as<std::string>() == sqeazy::yuv420formatter::y4m_code()){

      sqeazy::uint16_image_stack_cref loaded_stack(reinterpret_cast<const uint16_t*>(input.data()),c_storage_order_shape);
      auto encoded_end = pipe16.detail_encode(loaded_stack.data(),
					      (char*)converted_stack.data(),
					      c_storage_order_shape,
					      converted_stack.num_elements()*sizeof(std::uint8_t));

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
  
  if(!bfs::exists(src_file)){
    std::cerr << "[SQY]\tunable to open " << src_file << "\t Exit.\n";
    return;
  }


  if(src_file_extension.generic_string().find("tif")!=std::string::npos){

    forward_conversion(src_file,target_file,_config);

  } else {
    backward_conversion(src_file,target_file,_config);
  }
}


#endif /* _SQY_CONVERT_H_ */
