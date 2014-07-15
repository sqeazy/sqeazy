#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <chrono>
#include <iterator>
#include <numeric>
#include <vector>
#include <unordered_map>
#include "bench_fixtures.hpp"
#include "bench_utils.hpp"



int help_string(){
  
  std::string me = "bench";
  std::cout << "usage: " << me << " <target>\n"
	    << "availble targets:\n"
	    << "\t help\n"
	    << "\t diff_encode <num_iterations|optional>\n"
	    << "\t diff_decode <num_iterations|optional>\n"
	    << "\t ref_encode  <num_iterations|optional>\n"
	    << "\t pipe_encode <file_to_encode>\n"
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

int diff_encode(const std::vector<std::string>& _args){

  synthetic_fixture<> reference;
  std::cout << "SQY_RasterDiffEncode_3D_UI16 of "
  	    << reference << "\n";
  perform(std::function<int(int,int,int,const char*,char*)>(SQY_RasterDiffEncode_3D_UI16),reference);
  return 0;
}

int diff_decode(const std::vector<std::string>& _args){

  synthetic_fixture<> reference;
  std::cout << "SQY_RasterDiffDecode_3D_UI16 of "
  	    << reference << "\n";
  perform(std::function<int(int,int,int,const char*,char*)>(SQY_RasterDiffDecode_3D_UI16),reference);
  return 0;
}

int pipe_encode_on_file(const std::vector<std::string>& _args, long& _input_size, long& _output_size){

  tiff_fixture<unsigned short> reference(_args[1]);
  
  if(reference.empty())
    return 1;

  _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
  int retcode = SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					     input_data,intermediate_buffer);
  
  retcode += 10*SQY_BitSwap4Encode_UI16(intermediate_buffer,input_data,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  unsigned expected_size = SQY_LZ4Length(intermediate_buffer, &input_length);
  char* final_buffer = new char[expected_size];
  _output_size = reference.data_in_byte();
  retcode += 100 * SQY_LZ4Encode(input_data,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;

  return 42;

}

int pipe_encode_on_synthetic(const std::vector<std::string>& _args, long& _input_size, long& _output_size){

  synthetic_fixture<unsigned short> reference;
  std::cout << "input data: "
      	    << reference <<"\n";

  _input_size = reference.data_in_byte();
  const char* input_data = reinterpret_cast<const char*>(&reference.sin_data[0]);
  
  char first_intermediate[sizeof(short)*synthetic_fixture<unsigned short>::size];
  char second_intermediate[sizeof(short)*synthetic_fixture<unsigned short>::size];
  std::fill(std::begin(first_intermediate), std::end(first_intermediate),0);    
  std::fill(std::begin(second_intermediate), std::end(second_intermediate),0);    
  
  int retcode = SQY_RasterDiffEncode_3D_UI16(reference.axis_length(), reference.axis_length(), reference.axis_length(), 
			       input_data,first_intermediate);
  
  retcode += 10*SQY_BitSwap4Encode_UI16(first_intermediate,second_intermediate,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  unsigned expected_size = SQY_LZ4Length(second_intermediate, &input_length);
  char* final_buffer = new char[expected_size];
  _output_size = reference.data_in_byte();
  retcode += 100* SQY_LZ4Encode(second_intermediate,
			      synthetic_fixture<unsigned short>::size_in_byte,
			      final_buffer,
			      &_output_size
			      );
  delete [] final_buffer;
  
  return retcode;
}


int pipe_encode(const std::vector<std::string>& _args){


  long output_buffer_size_in_byte = 0;
  long input_buffer_size_in_byte = 0;
  int retcode = 0;
  if(_args.size()>1){
    retcode = pipe_encode_on_file(_args, input_buffer_size_in_byte, output_buffer_size_in_byte);
  } else {
    retcode = pipe_encode_on_synthetic(_args, input_buffer_size_in_byte, output_buffer_size_in_byte);
  }

  std::cout << "file = " << _args[1] << ", "
	    << ", output buffer size = " << output_buffer_size_in_byte << " ("<< output_buffer_size_in_byte/(1<<20) <<" MB)"
	    << ", compression ratio output/input = " << double(output_buffer_size_in_byte)/double(input_buffer_size_in_byte)
	    <<"\n";
  
  return retcode;
}

int main(int argc, char *argv[])
{

  typedef std::function<int(std::vector<std::string>) > func_t;
  std::unordered_map<std::string, func_t> prog_flow;
  prog_flow["help"] = func_t(help);
  prog_flow["-h"] = func_t(help);
  prog_flow["diff_encode"] = func_t(diff_encode);
  prog_flow["pipe_encode"] = func_t(pipe_encode);
  prog_flow["diff_decode"] = func_t(diff_decode);


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
