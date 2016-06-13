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

#include "verbs/detail.hpp"
#include "sqeazy_pipelines.hpp"
#include "string_shapers.hpp"

namespace po = boost::program_options;
namespace sqy = sqeazy;


void print_pairs(const std::vector<std::string>& _keys,
		 const std::vector<std::string>& _values,
		 const std::string& _prefix="  ",
		 std::size_t max_value_width = 70){

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

  std::size_t max_str_size = *std::max_element(key_sizes.begin(), key_sizes.end());

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
      auto lines_to_print = sqy::break_lines(current->cbegin(),current->cend(),max_value_width);
      std::cout << lines_to_print.front() << "\n";
      
      for(size_t i =1;i<lines_to_print.size();i++){
	std::cout << _prefix
		  << std::setw(max_str_size) << " "
		  << "\t"
		  << lines_to_print[i]
		  << "\n";
	
      }
	
    }
    ++value_sizes_itr;
  }
}


template <typename options_type, typename map_type >
int brief_help(const options_type& _sqy_options ,
	       const map_type& _verb_descr, 
	       const map_type& _verb_aliases 
	       ) {

  std::string me = "sqy";
  std::cout << "usage: " << me << " <-h|optional> <verb> <files|..>\n"
	    << "\n"
	    << "available verbs (their description and aliases):\n"
	    // << "\t" <<  "help" << "\n"
    ;

  std::vector<std::string> keys;
  for(auto& pair : _verb_descr)
    keys.push_back(pair.first);
  
  std::ostringstream alias;
  for( auto& key  : keys  ){
    
    std::cout << std::setw(4) << " " << std::left << std::setw(15) << key ;
    auto descr = _verb_descr.find(key);

    if(descr!=_verb_descr.end())
      std::cout << std::setw(4) << " " << std::left << std::setw(60) << descr->second ;
    
    if(_verb_aliases.find(key) != _verb_aliases.end()){
      auto value = _verb_aliases.find(key);
      std::cout << "(" << value->second << ")\n";
    }
    else
      std::cout << "\n";
  }
  
  std::cout << "\n"
	    << "available flags to sqy only:\n"
	    << _sqy_options << "\n";
  
  
}

static void print_pipeline_descriptions(){
  std::cout << "pipeline builder\n"
	    << "----------------\n"
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

}

template <typename modes_map_t, typename map_type >
int full_help(
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

  print_pipeline_descriptions();
  
  return 1;
}


int main(int argc, char *argv[])
{

  int retcode = 1;
  typedef std::function<int(const std::vector<std::string>&,const po::variables_map&) > func_t;

  po::options_description sqy_options;
  sqy_options.add_options()
    ("help,h", "produce this help message")
    ("fullhelp", "produce exhaustive help message with all verbs documented")
    ("version,v", "print version");

  po::variables_map sqy_vm;
  po::parsed_options sqy_parsed = po::command_line_parser(argc, argv
						      ).options(sqy_options).allow_unregistered().run();
  po::store(sqy_parsed, sqy_vm); 
  po::notify(sqy_vm);
    
  static     std::map<std::string, std::string> verb_aliases;
  verb_aliases["help"] 	= std::string("");
  verb_aliases["compress"] 	= std::string("compress|enc|encode|comp");
  verb_aliases["decompress"] 	= std::string("decompress|dec|decode|rec");
  verb_aliases["scan"]		= std::string("scan|info");                
  verb_aliases["convert"] 	= std::string("convert|transform|trf");
  verb_aliases["compare"] 	= std::string("compare|cmp");

  static     std::map<std::string, std::string> verb_descriptions;
  verb_descriptions["help"] 	= std::string("print a help message");
  verb_descriptions["compress"]	= std::string("compress a tiff stack to native sqy format or hdf5");
  verb_descriptions["decompress"]= std::string("decompress a .sqy/.h5 file to tiff");
  verb_descriptions["scan"]	= std::string("print statistics about the stack in a tiff file");
  verb_descriptions["convert"] 	= std::string("convert a tiff file to another format (.yuv, .y4m)");
  verb_descriptions["compare"] 	= std::string("compare two tiff stacks and see if they are equal");

  const static std::string default_compression = "bitswap1->lz4";
  static std::unordered_map<std::string,po::options_description> descriptions(4);

  
  descriptions["compress"].add_options()
    ("help,h", "produce help message")
    ("verbose,v", "enable verbose output")
    ("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used (see 'pipeline builder' documentation below)")
    ("dataset_name,d", po::value<std::string>()->default_value("sqy_stack"), "name of the HDF5 dataset to appear inside any of .h5 encoded files (ignored for native .sqy compression)")
    ("output_name,o", po::value<std::string>(), "file location to write output to (if only 1 is given)")
    ("output_suffix,e", po::value<std::string>()->default_value(".sqy"), "file extension to be used (must include period)")
    ;

  descriptions["decompress"].add_options()
    ("help,h", "produce help message")
    ("verbose,v", "enable verbose output")
    ("dataset_name,d", po::value<std::string>()->default_value("sqy_stack"), "name of the HDF5 dataset to load from input h5 file(s) (ignored for native .sqy compression)")
    ("output_name,o", po::value<std::string>(), "file location to write output to (if only 1 is given)")
    ("output_suffix,e", po::value<std::string>()->default_value(".tif"), "file extension to be used")
    ;

  descriptions["scan"].add_options()
    ("help,h", "produce help message")
    ("verbose,v", "enable verbose output")
    ;

  descriptions["convert"].add_options()
    ("help,h", "produce help message")
    ("verbose,v", "enable verbose output")
    ("chroma_sampling,c", po::value<std::string>()->default_value("C420"),  "how to subsample chroma channels (possible values: C420, C444)")
    ("quantiser,q", po::value<std::string>()->default_value("quantiser"), "method for conversion")
    ("frame_shape,s", po::value<std::string>()->default_value(""), "WidthxHeight of each frame  to expect (only needed for .yuv input sources), e.g. 1920x1020")
    ("lut_suffix,l", po::value<std::string>()->default_value(".lut"), "suffix for the LUT, i.e. stack_16bit.tif will generate stack_16bit.y4m and stack_16bit.lut) ")
    ;

  add_compare_options_to(descriptions["compare"]);
  
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
    if(!pair.second.empty())
      verb_aliases_rex[pair.first] = std::regex(rex.str());
  }


  if(argc<2) {
    retcode = brief_help(sqy_options,
			 verb_descriptions,
			 verb_aliases
			 );
    return retcode;
  }

  if(sqy_vm.count("fullhelp")) {
    retcode = full_help(descriptions,verb_aliases);
    return retcode;
  }

  
  std::string joint_args;
  for( int i = 0;i<argc;++i ){
    joint_args.append(argv[i]);
  }
    


  
  po::options_description options_to_parse;
      
  func_t prog_flow;

  std::string target(argv[1]);
  std::string verb;

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

  if(verb.empty()){

    
    if(sqy_vm.count("version")) {
      //FIXME: introduce versioing infrastructure
      std::cout << "sqy 0.0-alpha\n";
      return 1;
    }

    
    std::cerr << "unable to find matching verb for " << target << "\n";
    brief_help(sqy_options,
	       verb_descriptions,
	       verb_aliases
	       );
    return retcode;
  }

  
  
  if(descriptions.find(verb)!=descriptions.end()){
    options_to_parse.add(descriptions[verb]);
  }
    
  po::variables_map vm;

  std::vector<char*> argv_without_verb(argv,argv+argc);
  argv_without_verb.erase(argv_without_verb.begin()+1);

  po::parsed_options parsed = po::command_line_parser(argv_without_verb.size(), &argv_without_verb[0]// argc, argv
						      ).options(options_to_parse).allow_unregistered().run();
  po::store(parsed, vm); 
  po::notify(vm);


  if(vm.count("help")){
    std::cout << descriptions[verb] << "\n";
    if(vm.count("pipeline"))
      print_pipeline_descriptions();
    return retcode;
  }
  
  std::vector<std::string> inputFiles = po::collect_unrecognized(parsed.options, po::include_positional);

  if(inputFiles.empty()){
    std::cerr << "[sqy] no input files given, exiting ...\n";
    return 1;
  } else {
    if(!verb.empty()){
      retcode = prog_flow(inputFiles, vm);
    }
    else{
      std::cerr << "unable to find matching verb for " << target << "\n";
      brief_help(sqy_options,
			 verb_descriptions,
			 verb_aliases
			 );
      return retcode;
    }
  }
  

  return retcode;
}
