#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <string>
#include <sstream>
#include <numeric>
#include <vector>
#include <unordered_map>
#include "bench_fixtures.hpp"
#include "../src/hist_impl.hpp"

#include "TH1F.h"

int help_string(){
  
  std::string me = "histo";
  std::cout << "usage: " << me << " <target>\n"
	    << "availble targets:\n"
	    << "\t help\n"
	    << "\t scan <file_to_encode>\n"
	    << "\n";
  return 1;
}

int help(const std::vector<std::string>& _args){
  std::cerr << "unknown argument provided:\n";
  if(_args.empty())
    std::cerr << "<none>";
  else
    for( const std::string& el : _args )
      std::cerr << el << " ";
  std::cerr << "\n\n";

  return help_string();
}


int scan(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  
  if(reference.empty())
    return 1;

  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  
  std::string no_suffix(_args[1],0,_args[1].find_last_of("."));
  size_t dash_pos = _args[1].find_last_of("/");
  std::string basename(_args[1],dash_pos+1, no_suffix.size() - dash_pos-1);
  
  std::ostringstream str("");
  str << basename << sizeof(unsigned short)*8 << "bit_intensities";
  TH1F values(str.str().c_str(),";intensity;frequency",128,0.,1 << (sizeof(unsigned short)*8));

  for(unsigned long i = 0;i<ref_hist.num_bins;++i){
    values.Fill(i,ref_hist.bins[i]);
  }

  std::ostringstream plots("");
  plots << basename << ".root";
  
  values.SaveAs(plots.str().c_str());

  return 0;

}

int main(int argc, char *argv[])
{

  typedef std::function<int(std::vector<std::string>) > func_t;
  std::unordered_map<std::string, func_t> prog_flow;
  prog_flow["help"] = func_t(help);
  prog_flow["-h"] = func_t(help);
  prog_flow["scan"] = func_t(scan);

  std::vector<std::string> arguments(argv+1,argv + argc);
  
  int retcode = 0;
  if(argc>1 && prog_flow.find(argv[1])!=prog_flow.end()){
    retcode = prog_flow[argv[1]](arguments);
  }
  else{
    retcode = help(arguments);
  }
  
  return retcode;
}
