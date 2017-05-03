#ifndef _SQY_COMPARE_H_
#define _SQY_COMPARE_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "tiff_utils.hpp"
#include "deprecated/static_pipeline_select.hpp"
#include "sqeazy_algorithms.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "sqeazy_algorithms.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

static void add_compare_options_to(po::options_description& _desc){

  _desc.add_options()
    ("metrics,m", po::value<std::string>()->default_value("nrmse"), "comma-separated list of metrics (possible values: mse, nrmse, psnr, drange, all)")
    ("as-csv,c", "output as csv including header")
    ("noheader", "skip output header")
    ;

}

static void display_results(const std::map<std::string,float>& _results,
                            const po::variables_map& _config){


  std::vector<std::string> keys;
  std::vector<int> key_lengths;
  std::vector<float> values;
  for(const auto & k_v : _results ){
    keys.push_back(k_v.first);
    key_lengths.push_back(k_v.first.size());
    values.push_back(k_v.second);
  }

  auto max_width = *std::max_element(key_lengths.begin(), key_lengths.end());

  if(!_config.count("noheader")){
    for(std::size_t i = 0;i<keys.size();i++){

      if(!_config.count("as-csv"))
        std::cout << std::setw(max_width);

      std::cout << keys[i];

      if(i!=(keys.size()-1)){
        if(_config.count("as-csv"))
          std::cout << ",";
      }
      else
        std::cout << "\n";
    }
  }

  for(std::size_t i = 0;i<values.size();i++){
    if(!_config.count("as-csv"))
      std::cout << std::setw(max_width);

    std::cout << values[i];

    if(i!=(values.size()-1)){
      if(_config.count("as-csv"))
        std::cout << ",";
    }
    else
      std::cout << "\n";
  }

}

int compare_files(const std::vector<std::string>& _files,
                  const po::variables_map& _config) {

  int value = 1;

  if(_files.size()!=2){
    std::cerr << "number of files given is unequal 2 (received: "
              << _files.size() <<")" << std::endl;
    return value;
  }

  const bfs::path	src_file = _files.front();
  const bfs::path	src_file_extension = src_file.extension();

  const bfs::path	target_file = _files.back();
  const bfs::path	target_file_extension = target_file.extension();
  const bfs::path	target_file_stem = target_file.parent_path()/target_file.stem();//TODO


  std::vector<uint32_t>		src_input_dims;
  sqeazy::tiff_facet	src_input;

  std::vector<uint32_t>		target_input_dims;
  sqeazy::tiff_facet	target_input;

  std::map<std::string,float> results;

  if(!bfs::exists(src_file)){
    std::cerr << "[sqy-compare]\tunable to open " << src_file << "\tExiting.\n";
    return value;
  }

  if(!bfs::exists(target_file)){
    std::cerr << "[sqy-compare]\tunable to open " << target_file << "\tExiting.\n";
    return value;
  }

  if(!_config.count("metrics")){
    std::cerr << "[sqy-compare]\tno metrics received\tExiting.\n";
    return value;
  }

  std::string metrics = _config["metrics"].as<std::string>();
  bool do_nrmse = metrics.find("nrmse") != std::string::npos ? true : false;
  bool do_mse = (metrics.find("mse") != std::string::npos && !do_nrmse) ? true : false;
  bool do_psnr = metrics.find("psnr") != std::string::npos ? true : false;
  bool do_drange = metrics.find("drange") != std::string::npos ? true : false;
  bool do_bitwidth = metrics.find("bitwidth") != std::string::npos ? true : false;

  if(metrics.find("all") != std::string::npos)
  {
    do_nrmse = do_mse = do_psnr = do_drange = do_bitwidth = true;
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
      return value;
    }

    if(src_bits_per_sample==16){

      // std::vector<uint32_t> src_shape(src_input_dims.rbegin(), src_input_dims.rend());
      sqeazy::uint16_image_stack_cref src_stack_cref(reinterpret_cast<const uint16_t*>(src_input.data()),src_input_dims);

      // std::vector<uint32_t> target_shape(target_input_dims.rbegin(), target_input_dims.rend());
      sqeazy::uint16_image_stack_cref target_stack_cref(reinterpret_cast<const uint16_t*>(target_input.data()),target_input_dims);

      if(do_nrmse)
        results["nrmse"] = sqeazy::nrmse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                         target_stack_cref.data());

      if(do_mse)
        results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                     target_stack_cref.data());

      if(do_psnr)
        results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                       target_stack_cref.data());
      if(do_drange){
        results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
        results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }

      if(do_bitwidth)
        results["bits_p_sample"]  = 16;
    }

    if(src_bits_per_sample==8){

      // std::vector<uint32_t> src_shape(src_input_dims.rbegin(), src_input_dims.rend());
      sqeazy::uint8_image_stack_cref src_stack_cref((const uint8_t*)src_input.data(),src_input_dims);

      // std::vector<uint32_t> target_shape(target_input_dims.rbegin(), target_input_dims.rend());
      sqeazy::uint8_image_stack_cref target_stack_cref((const uint8_t*)target_input.data(),target_input_dims);

      if(do_nrmse)
        results["nrmse"] = sqeazy::nrmse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                         target_stack_cref.data());

      if(do_mse)
        results["mse"] = sqeazy::mse(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                     target_stack_cref.data());

      if(do_psnr)
        results["psnr"] = sqeazy::psnr(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements(),
                                       target_stack_cref.data());
      if(do_drange){
        results["left_drange"] = sqeazy::dyn_range(src_stack_cref.data(), src_stack_cref.data() + src_stack_cref.num_elements());
        results["right_drange"] = sqeazy::dyn_range(target_stack_cref.data(), target_stack_cref.data() + target_stack_cref.num_elements());
      }

      if(do_bitwidth)
        results["bits_p_sample"]  = 8;
    }

  }

  if(src_file_extension.generic_string().find("y4m")!=std::string::npos &&
     target_file_extension.generic_string().find("y4m")!=std::string::npos){

    std::vector<std::uint8_t> src_stack;
    std::vector<std::uint8_t> tgt_stack;

    auto src_shape = sqeazy::read_y4m_to_gray(src_stack,src_file.generic_string()	,_config.count("verbose"));
    auto tgt_shape = sqeazy::read_y4m_to_gray(tgt_stack,target_file.generic_string()	,_config.count("verbose"));

    if(!std::equal(src_shape.begin(), src_shape.end(),
                   tgt_shape.begin())){

      std::cerr << "unable to compare " <<  src_file << " vs " << target_file << ", shapes mismatch\n"
                <<  "width = "  << src_shape[sqeazy::row_major::w] << " =? " << tgt_shape[sqeazy::row_major::w] << ", "
                <<  "height = " << src_shape[sqeazy::row_major::h] << " =? " << tgt_shape[sqeazy::row_major::h] << ", "
                <<  "depth = "  << src_shape[sqeazy::row_major::d] << " =? " << tgt_shape[sqeazy::row_major::d] << "\n"
        ;
      return value;
    }


    if(do_nrmse)
      results["nrmse"] = sqeazy::nrmse(src_stack.data(), src_stack.data() + src_stack.size(),
                                       tgt_stack.data());

    if(do_mse)
      results["mse"] = sqeazy::mse(src_stack.data(), src_stack.data() + src_stack.size(),
                                   tgt_stack.data());

    if(do_psnr)
      results["psnr"] = sqeazy::psnr(src_stack.data(), src_stack.data() + src_stack.size(),
                                     tgt_stack.data());
    if(do_drange){
      results["left_drange"] = sqeazy::dyn_range(src_stack.data(), src_stack.data() + src_stack.size());
      results["right_drange"] = sqeazy::dyn_range(tgt_stack.data(), tgt_stack.data() + tgt_stack.size());
    }

    if(do_bitwidth)
      results["bits_p_sample"]  = 8;
  }

  if(results.empty()){

    std::cerr << "file format(s) unknown, sqeazy-compare does not support anything else than tif-to-tif or y4m-to-y4m yet!\n";
    return value;
  }

  display_results(results, _config);
  value = 0;

  return value;
}

#endif /* _SQEAZY_COMPARE_H_ */
