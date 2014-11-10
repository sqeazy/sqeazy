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
#include "tiff_utils.hpp"
#include "pipeline_select.hpp"

#include "boost/filesystem.hpp"
#include <boost/program_options.hpp>
namespace po = boost::program_options;


template <typename ModesContainer, typename ModesMap>
int print_help(const ModesContainer& _av_pipelines ,
	       const ModesMap& _modes_args
	       ) {

  std::string me = "sqy";
  std::cout << "usage: " << me << " <-h|optional> <target> <files|..>\n"
	    << "\n"
	    << "available targets:\n"
	    << "<help>" << "\n";


  if(_modes_args.size()){
      
    for( auto args : _modes_args ){
	
      std::cout << "<" << args.first << ">\n" << args.second << "\n";
    }
  }

  std::cout << "available pipelines:\n";
  std::set<std::string> pipeline_names;
  for( auto it : _av_pipelines )
    pipeline_names.insert(it);

  for(const std::string& pipeline : pipeline_names ) {
    std::cout << "\t" << pipeline << "\n";
  }

  std::cout << "\n";

  return 1;
}




void compress_files(const std::vector<std::string>& _files,
		    const po::variables_map& _config) {


  
  std::vector<char>	output_data;
  unsigned long			expected_size_byte = 0;
  boost::filesystem::path	current_file;
  boost::filesystem::path	output_file;
  std::fstream			sqyfile;
  std::vector<unsigned>		input_dims;
  sqeazy_bench::tiff_facet	input;
  sqeazy_bench::pipeline_select	dynamic;

  unsigned long compressed_length_byte = 0;
  int enc_ret = 0;

  for(const std::string& _file : _files) {

    
    current_file = _file;
    if(!boost::filesystem::exists(current_file)){
      std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
      continue;
    }

    //load tiff & extract the dimensions
    input.reload(_file);
       
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



void decompress_files(const std::vector<std::string>& _files,
		      const po::variables_map& _config) {


  std::vector<char> file_data;
  std::vector<char> output_data;

  std::vector<unsigned> found_shape;
  unsigned long long found_size_byte = 0;
  unsigned long long expected_size_byte = 0;
  boost::filesystem::path current_file;
  boost::filesystem::path output_file;
  std::fstream sqyfile;
  std::string found_pipeline;
  unsigned found_num_bites;
  sqeazy_bench::pipeline_select dynamic;

  for(const std::string& _file : _files) {

    file_data.clear();
    found_pipeline = "unknown";
    found_shape.clear();
    found_num_bites = 0;
    output_data.clear();

    current_file = _file;

    //skip the rest if nothing was loaded
    if(!boost::filesystem::exists(current_file))
      {
	std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
	continue;
      }

    found_size_byte = boost::filesystem::file_size(current_file);
    sqyfile.open(_file, std::ios_base::binary | std::ios_base::in );

    //read the contents of the file
    if(file_data.size()!=found_size_byte)
      file_data.resize(found_size_byte);

    sqyfile.get(&file_data[0],found_size_byte);

    const char* file_ptr =  &file_data[0];
    const char* end_header_ptr =  std::find(file_ptr, file_ptr + found_size_byte);
    sqeazy::image_header<sqeazy::unknown> sqy_header(file_ptr,
						     end_header_ptr
						     );

    found_shape = sqy_header.shape();
    found_pipeline = sqy_header.pipeline();
    found_num_bits = sqy_header.sizeof_header_type()*CHAR_BIT;
    dynamic.set(found_num_bits, found_pipeline);

    ///////////////////////////////////////////////////////////////
    // sqy file content specific

    //compute the maximum size of the output buffer
    expected_size = dynamic.decoded_size_byte(file_ptr, file_ptr + sqy_header.size());

    if(expected_size>output_data.size())
      output_data.resize(expected_size);
        
    //create clean output buffer
    std::fill(output_data.begin(), output_data.end(),0);

    int dec_ret = dynamic.decompress(&file_data[0],&output_data[0],
    					   found_size_byte);
	
    if(dec_ret && _config.count("verbose")) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".tif");
    
    // sqeazy_bench::write_tiff_from_vector(output_data, found_shape , output_file.string());
    // ///////////////////////////////////////////////////////////////

    sqyfile.close();
  }

}

int main(int argc, char *argv[])
{


    typedef std::function<void(const std::vector<std::string>&,const po::variables_map&) > func_t;
    
    //FIXME:: just a placeholder to make the whole thing compile

    static std::vector<std::string> modes{bswap1_lz4_pipe::name(),
       rmbkg_bswap1_lz4_pipe::name()};
    const static std::string default_compression = bswap1_lz4_pipe::name();

    static std::unordered_map<std::string,po::options_description> descriptions(2);

      descriptions["compress"].add_options()
	("help", "produce help message")
	("verbose,v", po::value<bool>()->default_value(false), "enable verbose output")
	("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used")
      ;

      descriptions["decompress"].add_options()
	("help", "produce help message")
	("verbose,v", po::value<bool>()->default_value(false), "enable verbose output")
      ;


    // std::vector<std::string> args(argv+1, argv+argc);
    std::string joint_args;
    for( int i = 0;i<argc;++i ){
      joint_args.append(argv[i]);
    }
    
    int retcode = 0;
    if(argc<2 || joint_args.find("-h")!=std::string::npos || joint_args.find("help")!=std::string::npos) {
      retcode = print_help(modes,descriptions);
    }
    else{
        
      po::options_description options_to_parse;
      
      func_t prog_flow;

      std::string target(argv[1]);

      std::vector<char*> new_argv(argv,argv+argc);
      new_argv.erase(new_argv.begin()+1);
      
      if(descriptions.find(target)!=descriptions.end()){
	options_to_parse.add(descriptions[target]);
      }
	


      //REFACTOR THIS!
      if(target == "compress"){
	prog_flow = compress_files;
      }

      if(target == "decompress"){
	prog_flow = decompress_files;
      }


      po::variables_map vm;

      // try{
      po::parsed_options parsed = po::command_line_parser(new_argv.size(), &new_argv[0]// argc, argv
							  ).options(options_to_parse).allow_unregistered().run();
      po::store(parsed, vm); 
      // }
      // catch(std::exception& e){
      // 	std::cerr << "[sqy]\tparsing options failed\t" << e.what() << "\n";
      // 	return print_help(modes,descriptions);
      // }
	
      
      po::notify(vm);
	
      std::vector<std::string> inputFiles;// = vm["files"].as<std::vector<std::string> >();
	
      // std::cout << target << " received files \n";
      // for( auto ffile : inputFiles ){
      // 	std::cout << "\t"<< ffile << "\n";
      // }

      std::vector<std::string> inputFiles = po::collect_unrecognized(parsed.options, po::include_positional);
      for( auto to_p : inputFiles ){
	std::cout << target << " to_p "<< to_p << "\n";
      }
      
      if(target == "compress"){
	compress_files(inputFiles, vm);
      }
    }

    return retcode;
}
