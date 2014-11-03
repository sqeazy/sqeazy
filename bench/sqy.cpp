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

template <typename ModesContainer, typename ModesArgs>
int print_help(const ModesContainer& _av_pipelines ,
	       const ModesArgs& _modes_args
	       ) {

    std::string me = "sqy";
    std::cout << "usage: " << me << " <-h|optional> <mode> <files|..>\n"
              << "\n"
              << "availble modes:\n"
              << "\thelp" << "\n";


    std::set<std::string> target_names;
    for( auto it : _av_pipelines )
        target_names.insert(it);

    for(const std::string& target : target_names ) {
        std::cout << "\t" << target << "\n";
    }

    std::cout << "\n";
    
    if(_modes_args.size()){
      for( auto args : _modes_args ){
	std::cout << "\n";
      }
    }

    return 1;
}

template <typename T, typename PipeType>
void compress_files(const std::vector<std::string>& _files,
		      const sqy_config& _config) {

  typedef T value_type;
  typedef PipeType current_pipe;
  typedef tiff_fixture<T> tiff_image;

  if(_config.verbose)
    std::cerr << "fill_suite :: " << PipeType::name() << "\n";

  
  std::vector<char> output_data;

  tiff_image input_file;

  unsigned long filesize = 0;
  boost::filesystem::path current_file;
  boost::filesystem::path output_file;
  std::fstream sqyfile;

  for(const std::string& _file : _files) {

    
    current_file = _file;
    if(!boost::filesystem::exists(current_file)){
      std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
      continue;
    }

    //load tiff & extract the dimensions
    input_file.fill_from(_file);
       
    //compute the maximum size of the output buffer
    filesize = current_pipe::max_encoded_size(input_file.axis_lengths);

    if(filesize!=output_data.size())
      output_data.resize(filesize);

    //create clean output buffer
    std::fill(output_data.begin(), output_data.end(),0);
    unsigned long compressed_length_byte = 0;

    //compress
    int enc_ret = current_pipe::compress(input_file.data(),
					 &output_data[0],
					 input_file.axis_lengths, 
					 compressed_length_byte);
	
    if(enc_ret && _config.verbose) {
      std::cerr << "compression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".sqy");
    sqyfile.open(output_file.string(), std::ios_base::binary | std::ios_base::out );
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


template <typename T, typename PipeType>
void decompress_files(const std::vector<std::string>& _files,
		      const sqy_config& _config) {

  typedef T value_type;
  typedef PipeType current_pipe;

  if(_config.verbose)
    std::cerr << "fill_suite :: " << PipeType::name() << "\n";

  std::vector<char> file_data;
  std::vector<value_type> output_data;
  std::vector<unsigned> output_dims;
  unsigned long filesize = 0;
  boost::filesystem::path current_file;
  boost::filesystem::path output_file;
  std::fstream sqyfile;

  for(const std::string& _file : _files) {

    
    current_file = _file;

    //skip the rest if nothing was loaded
    if(!boost::filesystem::exists(current_file))
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	continue;
      }

    filesize = boost::filesystem::file_size(current_file);
    sqyfile.open(_file, std::ios_base::binary | std::ios_base::in );

    //read the contents of the file
    if(file_data.size()!=filesize)
      file_data.resize(filesize);

    sqyfile.get(&file_data[0],filesize);

    //compute the maximum size of the output buffer
    long expected_size = current_pipe::decoded_size_byte(&file_data[0],file_data.size());
    expected_size /= sizeof(value_type);

    if(expected_size>output_data.size())
      output_data.resize(expected_size);

    //extract the dimensions to be expected
    output_dims = current_pipe::decode_dimensions(&file_data[0],filesize);
        
    //create clean output buffer
    std::fill(output_data.begin(), output_data.end(),0);

    int dec_ret = current_pipe::decompress(&file_data[0],&output_data[0],
					   filesize);
	
    if(dec_ret && _config.verbose) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".tif");
    
    sqeazy::write_tiff_from_vector(output_data, output_dims , output_file.string());
    
    sqyfile.close();
  }

}

int main(int argc, char *argv[])
{


    typedef std::function<void(const std::vector<std::string>&,const sqy_config&) > func_t;
    
    static std::vector<std::string> modes{bswap1_lz4_pipe::name(),
	rmbkg_bswap1_lz4_pipe::name()
	};
    
    std::unordered_map<std::string, func_t> encode_flow;
    encode_flow[modes[0]] = func_t(compress_files<unsigned short, bswap1_lz4_pipe>);
    encode_flow[modes[1]] = func_t(compress_files<unsigned short, rmbkg_bswap1_lz4_pipe>);

    std::unordered_map<std::string, func_t> decode_flow;
    decode_flow[modes[0]] = func_t(decompress_files<unsigned short, bswap1_lz4_pipe>);
    decode_flow[modes[1]] = func_t(decompress_files<unsigned short, rmbkg_bswap1_lz4_pipe>);

    std::vector<std::string> args(argv+1, argv+argc);
    
    int retcode = 0;
    if(argc<2 || args[0].find("-h")!=std::string::npos || args[0].find("help")!=std::string::npos) {
      retcode = print_help(modes,args);
    }
    else{
        unsigned int f_index = 1;

        // std::unordered_map<std::string, func_t>::const_iterator f_func = prog_flow.end();

        
        // fill_suite_config config(args);

        // for(f_index = 0; f_index<args.size(); ++f_index) {
        //     if((f_func = prog_flow.find(args[f_index])) != prog_flow.end())
        //         break;
        // }

        // std::vector<std::string> filenames(args.begin()+1, args.end());

	std::cerr << "NOTHING IMPLEMENTED SO FAR\n";
	//print_help(prog_flow);
        
    }

    return retcode;
}
