#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <iomanip>
#include <numeric>
#include <vector>
#include <cmath>
#include <fstream>
#include <unordered_map>


#include "bench_fixtures.hpp"
#include "bench_utils.hpp"
#include "bench_common.hpp"

#include "boost/filesystem.hpp"

struct sqy_config
{
  bool verbose;
};

template <typename MapT>
int print_help(const MapT& _av_targets) {

    auto mbegin = _av_targets.cbegin();
    auto mend = _av_targets.cend();
    auto max_el_itr = std::max_element(mbegin,mend,
                                       [&](const typename MapT::value_type& a,
    const typename MapT::value_type& b) {
        return a.first.size() < b.first.size();

    }
                                      );
    const unsigned field_width = max_el_itr->first.size()+1;

    std::string me = "sqy";
    std::cout << "usage: " << me << " <flags> <target>\n"
              << "flags:\n\t -v \t\t print benchmark stats for every file\n"
              << "\t -r <path>\t perform round trip and save output to <path>/<basefilename>-<targetname>.tif\n"
              << "\t -e <path>\t save result of encoding <path>/<basefilename>-<targetname>.tif\n"
              << "\n"
              << "availble targets:\n"
              << std::setw(field_width) << "help" << "\n";


    std::set<std::string> target_names;
    for( auto it : _av_targets )
        target_names.insert(it.first);

    for(const std::string& target : target_names ) {
        std::cout << std::setw(field_width) << target <<"\t<files_to_encode>\n";
    }

    std::cout << "\n";

    return 1;
}


template <typename T, typename PipeType>
void decompress_files(const std::vector<std::string>& _files,
		      const sqy_config& _config) {

  typedef T value_type;
  typedef PipeType current_pipe;

  if(_config.verbose)
    std::cerr << "fill_suite :: " << PipeType::name() << "\n";

  std::vector<char> file_data;
  std::vector<value_type> output_data;
  std::vector<size_t> output_dims;
  unsigned long filesize = 0;
  boost::filesystem::path current_file;
  boost::filesystem::path output_file;
  std::fstream inputfile;

  for(const std::string& _file : _files) {

    inputfile.open(_file.c_str());
    current_file = _file;
    filesize = boost::filesystem::file_size(current_file);
    
    //skip the rest if nothing was loaded
    if(!inputfile.is_open())
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	inputfile.clear();
	continue;
      }
       
    //compute the maximum size of the output buffer
    long expected_size = 0;
    sqeazy::lz4_scheme<char>::decoded_size_byte(&file_data[0],&expected_size);
    expected_size /= sizeof(value_type);

    if(expected_size>output_data.size())
      output_data.resize(expected_size);
        
    //create clean output buffer
    std::fill(output_data.begin(), output_data.end(),0);

    int dec_ret = current_pipe::decompress(&file_data[0],&output_data[0],
					   filesize);
	
    if(dec_ret & _config.verbose) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".tif");
    
    sqeazy::write_tiff_from_vector(output_data, output_dims , output_file.string());
  }

}

int main(int argc, char *argv[])
{


    // typedef std::function<void(const std::vector<std::string>&,const sqy_config&) > func_t;
    // std::unordered_map<std::string, func_t> prog_flow;

    // prog_flow["compress_rmbkg_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    // prog_flow["compress_rmbkg_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    // prog_flow["compress_rmbkg_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_bswap1_lz4_pipe>);
    // prog_flow["compress_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    // prog_flow["compress_diffonrow_bswap1_lz4"] = func_t(fill_suite<unsigned short, diffonrow_bswap1_lz4_pipe>);
    // prog_flow["compress_bswap1_lz4"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    // prog_flow["compress_diff_lz4"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    // prog_flow["compress_lz4"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    // prog_flow["experim_rmbkg_lz4"] = func_t(fill_suite<unsigned short, rmbkg_lz4_pipe>);
    // prog_flow["experim_diffonrow_lz4"] = func_t(fill_suite<unsigned short, diffonrow_lz4_pipe>);
    // prog_flow["experim_huff_lz4"] = func_t(fill_suite<unsigned short, huff_lz4_pipe>);
    // prog_flow["experim_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, huff_bswap1_lz4_pipe>);

    // prog_flow["speed_rmbkg_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    // prog_flow["speed_rmbkg_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    // prog_flow["speed_rmbkg_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_bswap1_lz4_pipe>);
    // prog_flow["speed_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    // prog_flow["speed_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, huff_bswap1_lz4_pipe>);
    // prog_flow["speed_bswap1_lz4"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    // prog_flow["speed_diff_lz4"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    // prog_flow["speed_lz4"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    int retcode = 0;
    if(argc>1) {
        unsigned int f_index = 1;

        // std::unordered_map<std::string, func_t>::const_iterator f_func = prog_flow.end();

        std::vector<std::string> args(argv+1, argv+argc);
        // fill_suite_config config(args);

        // for(f_index = 0; f_index<args.size(); ++f_index) {
        //     if((f_func = prog_flow.find(args[f_index])) != prog_flow.end())
        //         break;
        // }

        // std::vector<std::string> filenames(args.begin()+1, args.end());

	std::cerr << "NOTHING IMPLEMENTED SO FAR\n";
	//print_help(prog_flow);
        
    }
    else {
      retcode = 1;//print_help(prog_flow);
    }

    return retcode;
}
