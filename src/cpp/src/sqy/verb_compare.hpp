#ifndef _SQY_COMPARE_H_
#define _SQY_COMPARE_H_

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
  sqeazy::tiff_facet	src_input;

  std::vector<uint32_t>		target_input_dims;
  sqeazy::tiff_facet	target_input;

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
	results["nrmse"] = sqeazy::nrmse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
					 target_stack_cref.data());
	results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				       target_stack_cref.data());
	results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
	results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }
      
      if(_config["metrics"].as<std::string>().find("nrmse") !=std::string::npos){
	results["nrmse"] = sqeazy::nrmse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
					 target_stack_cref.data());}

      if(_config["metrics"].as<std::string>().find("mse") != std::string::npos){
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());
      }
      
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

      if(_config["metrics"].as<std::string>().find("nrmse") !=std::string::npos){
	results["nrmse"] = sqeazy::nrmse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
					 target_stack_cref.data());}

      if(_config["metrics"].as<std::string>().find("mse") != std::string::npos){
	results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
				     target_stack_cref.data());
      }
      
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
    if(_config.count("verbose"))
      std::cout << std::setw(10) << k_v.first << "\t" << std::setw(15) << k_v.second << "\n";
    else
      std::cout << std::setw(15) << k_v.second << "\t";

  }

  if(!_config.count("verbose"))
    std::cout << "\n";
}

#endif /* _SQY_COMPARE_H_ */
