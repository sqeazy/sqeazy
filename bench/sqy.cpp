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


  
  std::vector<char> output_data;
  
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
	
    if(enc_ret && _config.count("verbose")) {
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



void decompress_files(const std::vector<std::string>& _files,
		      const po::variables_map& _config) {


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
	
    if(dec_ret && _config.count("verbose")) {
      std::cerr << "decompression failed! Nothing to write to disk...\n";
      continue;
    }

    output_file = current_file.replace_extension(".tif");
    
    sqeazy_bench::write_tiff_from_vector(output_data, output_dims , output_file.string());
    
    sqyfile.close();
  }

}

int main(int argc, char *argv[])
{


    typedef std::function<void(const std::vector<std::string>&,const po::variables_map&) > func_t;
    
    //FIXME:: just a placeholder to make the whole thing compile

    static std::vector<std::string> modes{bswap1_lz4_pipe::name(),
       rmbkg_bswap1_lz4_pipe::name(),
       typeid(unsigned char).name() + bswap1_lz4_pipe::name(),
       typeid(unsigned char).name() + rmbkg_bswap1_lz4_pipe::name()
       };
    const static std::string default_compression = bswap1_lz4_pipe::name();

    static std::unordered_map<std::string,po::options_description> descriptions(2);

      descriptions["compress"].add_options()
      ("help", "produce help message")
	("verbose,v", po::value<bool>()->default_value(false), "enable verbose output")
	("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used")
	("files", po::value<std::vector<std::string> >()// ->composing()
	 , "")
      ;

      descriptions["decompress"].add_options()
      ("help", "produce help message")
	("verbose,v", po::value<bool>()->default_value(false), "enable verbose output")
      ("files", po::value<std::vector<std::string> >()// ->composing()
       , "")
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
        

	po::options_description* desc = 0;
	func_t prog_flow;

	std::string target(argv[1]);
	
	if(descriptions.find(target)!=descriptions.end()){
	  desc = &descriptions[target];
	}
	
	//REFACTOR THIS!
	if(target == "compress"){
	  prog_flow = compress_files;
	}

	if(target == "decompress"){
	  prog_flow = decompress_files;
	}

	if(prog_flow.empty() || !desc){
	  std::cerr << "unable to decipher target: " << argv[1] << "\n";
	  return print_help(modes,descriptions);
	}

	po::variables_map vm;
	po::positional_options_description pd;
	pd.add("files", -1);
	
	po::store(po::command_line_parser(argc-2, argv+2).options(*desc).positional(pd).allow_unregistered().run(), 
		  vm); 

	po::notify(vm);
	
	std::vector<std::string> inputFiles = vm["files"].as<std::vector<std::string> >();
	
	std::cout << target << " received files \n";
	for( auto ffile : inputFiles ){
	  std::cout << "\t"<< ffile << "\n";
	}
    }

    return retcode;
}
