#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <vector>
#include <cmath>
//#include <fstream>
#include <unordered_map>
#include <regex>

#include "app_algs.hpp"
#include "sqeazy_predef_pipelines.hpp"

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


int main(int argc, char *argv[])
{


  typedef std::function<void(const std::vector<std::string>&,const po::variables_map&) > func_t;
    
  //FIXME:: just a placeholder to make the whole thing compile

  static std::vector<std::string> modes{sqeazy::bswap1_lz4_pipe::name(),
      sqeazy::rmbkg_bswap1_lz4_pipe::name()};
  const static std::string default_compression = sqeazy::bswap1_lz4_pipe::name();

  static std::unordered_map<std::string,po::options_description> descriptions(4);

  descriptions["compress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used")
    ;

  descriptions["decompress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ;

  descriptions["scan"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ;

  descriptions["convert"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("chroma_sampling,c", po::value<std::string>()->default_value("C420"),  "from .tif to .y4m (possible values: C420, C444)")
    //("method,m", po::value<std::string>()->default_value("internal"), "method for conversion (possible: sws_scale, internal)")
    ("frame_shape,s", po::value<std::string>()->default_value(""), "shape of each frame to expect (only needed for .yuv sources), e.g. 1920x1020")
    ("dump_quantiser,d", "dump LUT for quantiser")
    ;

  descriptions["compare"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("metrics,m", po::value<std::string>()->default_value("nrmse"), "comma-separated list of metrics (possible values: mse, psnr)")
    ;

  static     std::unordered_map<std::string, std::regex> aliases(4);
  aliases["compress"] = std::regex("(compress|enc|encode|comp)");
  aliases["decompress"] = std::regex("(decompress|dec|decode|rec)");
  aliases["scan"] = std::regex      ("(scan|info)");                
  aliases["convert"] = std::regex   ("(convert|transform|trf)");
  aliases["compare"] = std::regex("(compare|cmp)");
    
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


    ///////////////////////////////////////////
    //REFACTOR THIS!
    if(std::regex_match(target,aliases["compress"]))
      prog_flow = compress_files;

    if(std::regex_match(target,aliases["decompress"]))
      prog_flow = decompress_files;

    if(std::regex_match(target,aliases["scan"]))
      prog_flow = scan_files;

    if(std::regex_match(target,aliases["convert"]))
      prog_flow = convert_files;

    if(std::regex_match(target,aliases["compare"]))
      prog_flow = compare_files;

    
    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(new_argv.size(), &new_argv[0]// argc, argv
							).options(options_to_parse).allow_unregistered().run();
    po::store(parsed, vm); 
    po::notify(vm);
	
    std::vector<std::string> inputFiles = po::collect_unrecognized(parsed.options, po::include_positional);

    if(inputFiles.empty()){
      std::cout << "[sqy] no input files given, exiting ...\n";
    } else {
      prog_flow(inputFiles, vm);
    }
  }

  return retcode;
}
