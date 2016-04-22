#define __SQY_CPP__
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

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "app/verbs.hpp"
#include "sqeazy_pipelines.hpp"


namespace po = boost::program_options;
namespace sqy = sqeazy;


int print_pairs(const std::vector<std::string>& _keys,
		const std::vector<std::string>& _values,
		const std::string& _prefix="  ",
		int max_value_width = 70){

  std::vector<int> key_sizes(_keys.size());
  std::vector<int> value_sizes(_values.size());
  auto sizes = [](const std::string& _str){
    return _str.size();
  };
  std::transform(_keys.begin(), _keys.end(),
		 key_sizes.begin(),
		 sizes);
  std::transform(_values.begin(), _values.end(),
		 value_sizes.begin(),
		 sizes);

  int max_str_size = *std::max_element(key_sizes.begin(), key_sizes.end());

  auto value_itr = _values.cbegin();
  auto value_sizes_itr = value_sizes.cbegin();
  for( const std::string& key : _keys ){
    std::cout << _prefix
	      << std::setw(max_str_size) << key
	      << "\t";
    auto current = (value_itr++);

    if(current->size()<max_value_width)
      std::cout << *(current) << "\n";
    else{
      int size = *value_sizes_itr;
      int chunks = (size + max_value_width -1)/max_value_width;
      std::cout << current->substr(0,max_value_width) << "\n";
      //FIXME: better would be to only split/substr if boundary is a space!
      for(int c = 1;c<chunks;++c){
	std::cout << _prefix
		  << std::setw(max_str_size) << " "
		  << "\t"
		  << current->substr(c*max_value_width,max_value_width)
		  << "\n";
	
      }
	
    }
    ++value_sizes_itr;
  }
}

template <typename modes_map_t, typename map_type >
int print_help(
	       const modes_map_t& _modes_args,
	       const map_type& _verb_aliases 
	       ) {

  std::string me = "sqy";
  std::cout << "usage: " << me << " <-h|optional> <verb> <files|..>\n"
	    << "\n"
	    << "available verbs:\n"
	    << "<help>" << "\n";


  if(_modes_args.size()){
      
    for( auto args : _modes_args ){

      auto fitr = _verb_aliases.find(args.first);
      if(fitr!=_verb_aliases.end())
	std::cout << "<" << fitr->second << ">\n" << args.second << "\n";
      else
	std::cout << "<" << args.first << ">\n" << args.second << "\n";
    }
  }

  std::cout << "pipeline builder\n"
	    << "	- pipelines may consist of any number of filters\n"
	    << "          (i.e. a function that ingests data of type T and emits it of type T again)\n"
	    << "	- each pipeline may have no or at least 1 sink\n"
    	    << "	  (i.e. a function that ingests data of type T and emits it of a type with smaller width than T,\n"
    	    << "	  for example quantiser or lz4)\n\n";

  std::vector<std::string> head_filter_factory_names = sqy::dypeline<std::uint16_t>::head_filter_factory_t::name_list();
  std::vector<std::string> head_filter_factory_descr = sqy::dypeline<std::uint16_t>::head_filter_factory_t::descriptions();
  
  std::vector<std::string> tail_filter_factory_names = sqy::dypeline<std::uint16_t>::tail_filter_factory_t::name_list();
  std::vector<std::string> tail_filter_factory_descr = sqy::dypeline<std::uint16_t>::tail_filter_factory_t::descriptions();
  
  std::vector<std::string> sink_factory_names        = sqy::dypeline<std::uint16_t>::sink_factory_t::name_list()   ;
  std::vector<std::string> sink_factory_descr	     = sqy::dypeline<std::uint16_t>::sink_factory_t::descriptions();

  std::cout << "available filters (before sink):\n";
  print_pairs(head_filter_factory_names,head_filter_factory_descr);
  
  std::cout << "\navailable sinks:\n";
  print_pairs(sink_factory_names,sink_factory_descr);
  
  std::cout << "\navailable filters (after sink):\n";
  print_pairs(tail_filter_factory_names,tail_filter_factory_descr);
  
  std::cout << "\n";

  return 1;
}


int main(int argc, char *argv[])
{


  typedef std::function<void(const std::vector<std::string>&,const po::variables_map&) > func_t;
    
  // static std::vector<std::string> modes{sqeazy::bswap1_lz4_pipe::static_name(),
  //     sqeazy::rmbkg_bswap1_lz4_pipe::static_name()};
  const static std::string default_compression = "bitswap1->lz4";

  static std::unordered_map<std::string,po::options_description> descriptions(4);

  descriptions["compress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used")
    ;

  descriptions["decompress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("output_name,o", po::value<std::string>(), "file location to write output to (if only 1 is given)")
    ("output_suffix,e", po::value<std::string>()->default_value(".tif"), "file extension to be used (must include period)")
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

  static     std::unordered_map<std::string, std::string> verb_aliases;
  verb_aliases["compress"] 	= std::string("compress|enc|encode|comp");
  verb_aliases["decompress"] 	= std::string("decompress|dec|decode|rec");
  verb_aliases["scan"]		= std::string("scan|info");                
  verb_aliases["convert"] 	= std::string("convert|transform|trf");
  verb_aliases["compare"] 	= std::string("compare|cmp");

  static     std::unordered_map<std::string, func_t> verb_functors;
  verb_functors["compress"]	= compress_files;      
  verb_functors["decompress"]	= decompress_files;    
  verb_functors["scan"]		= scan_files;          
  verb_functors["convert"]	= convert_files;       
  verb_functors["compare"]	= compare_files;       
  
  
  static     std::unordered_map<std::string, std::regex> verb_aliases_rex(verb_aliases.size());
  for ( auto& pair : verb_aliases ){
    std::ostringstream rex;
    rex << "(" << pair.second << ")";
    verb_aliases_rex[pair.first] = std::regex(rex.str());
  }
    
  // std::vector<std::string> args(argv+1, argv+argc);
  std::string joint_args;
  for( int i = 0;i<argc;++i ){
    joint_args.append(argv[i]);
  }
    
  int retcode = 0;
  if(argc<2 || joint_args.find("-h")!=std::string::npos || joint_args.find("help")!=std::string::npos) {
    retcode = print_help(descriptions,verb_aliases
			 );
  }
  else{
        
    po::options_description options_to_parse;
      
    func_t prog_flow;

    std::string target(argv[1]);
    std::string verb;
    
    std::vector<char*> new_argv(argv,argv+argc);
    new_argv.erase(new_argv.begin()+1);


    ///////////////////////////////////////////
    //REFACTOR THIS!
    for( auto& pair: verb_aliases_rex){
      if(std::regex_match(target,pair.second))
	{
	  prog_flow = verb_functors[pair.first];
	  verb = pair.first;
	  break;
	}
    }

    if(descriptions.find(verb)!=descriptions.end()){
      options_to_parse.add(descriptions[verb]);
    }
    
    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(new_argv.size(), &new_argv[0]// argc, argv
							).options(options_to_parse).allow_unregistered().run();
    po::store(parsed, vm); 
    po::notify(vm);
	
    std::vector<std::string> inputFiles = po::collect_unrecognized(parsed.options, po::include_positional);

    if(inputFiles.empty()){
      std::cerr << "[sqy] no input files given, exiting ...\n";
      return 1;
    } else {
      if(!verb.empty())
	prog_flow(inputFiles, vm);
      else{
	std::cerr << "unable to find matching verb for " << target << "\n";
	print_help(descriptions,verb_aliases);
	return 1;
      }
    }
  }

  return retcode;
}
