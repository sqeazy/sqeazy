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
#include "hist_impl.hpp"
#include "pipeline.hpp"
#include "sqeazy_impl.hpp"
#include "external_encoders.hpp"

int help_string(){
  
  std::string me = "bench";
  std::cout << "usage: " << me << " <target>\n"
	    << "availble targets:\n"
	    << "\t help\n"
	    << "\t diff_bswap4_lz4_compress <files_to_encode>\n"
    	    << "\t bswap4_lz4_compress      <files_to_encode>\n"
    	    << "\t diff_bswap1_lz4_compress <files_to_encode>\n"
    	    << "\t bswap1_lz4_compress      <files_to_encode>\n"
	    << "\t diff_lz4_compress        <files_to_encode>\n"
	    << "\n";

  return 1;
}


int bswap1_lz4_compress(const std::vector<std::string>& _args) {

    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<unsigned short>, sqeazy::lz4_scheme<unsigned short> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;
    int ret_code = 0;
    unsigned num_files = _args.size() - 1;
    std::vector<char> output;

    for(unsigned i = 0; i<num_files; ++i) {

        tiff_fixture<unsigned short> reference(_args[i]);

        if(reference.empty())
            return 1;

        sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());

        ret_code += current_pipe::encode(&reference.tiff_data[0],&output[0],reference.axis_lengths);

        unsigned long _output_size = current_pipe::last_num_encoded_bytes;

        std::cout << "bswap1_lz4\t"
                  << _args[i] << "\t"
                  << reference.data_in_byte() << "\t"
                  << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
                  << ref_hist.smallest_populated_bin() << "\t"
                  << ref_hist.largest_populated_bin() << "\t"
                  << ref_hist.mode() << "\t"
                  << ref_hist.mean() << "\t"
                  << double(_output_size)/double(reference.data_in_byte())
                  << "\n";
    }
    
    return ret_code;

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

int diff_bswap4_lz4_compress(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  if(reference.empty())
    return 1;

  unsigned long _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
  int retcode = SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					     input_data,intermediate_buffer);
  
  retcode += 10*SQY_BitSwap4Encode_I16(intermediate_buffer,input_data,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  long expected_size = input_length;
  retcode += SQY_LZ4_Max_Compressed_Length(&expected_size);

  char* final_buffer = new char[expected_size];
  long _output_size = reference.data_in_byte();
  retcode += 100 * SQY_LZ4Encode(input_data,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;
  std::cout << "diff_bswap4_lz4\t" 
	    << _args[1] << "\t"
	    << reference.data_in_byte() << "\t" 
	    << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
	    << ref_hist.smallest_populated_bin() << "\t"
	    << ref_hist.largest_populated_bin() << "\t"
	    << ref_hist.mode() << "\t"
	    << ref_hist.mean() << "\t"
	    << double(_output_size)/double(reference.data_in_byte())
    	    << "\n";

  return retcode;

}

int diff_bswap1_lz4_compress(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  if(reference.empty())
    return 1;

  unsigned long _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
  int retcode = SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					     input_data,intermediate_buffer);
  
  retcode += 10*SQY_BitSwap1Encode_I16(intermediate_buffer,input_data,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  long expected_size = input_length;
  retcode += SQY_LZ4_Max_Compressed_Length(&expected_size);

  char* final_buffer = new char[expected_size];
  long _output_size = reference.data_in_byte();
  retcode += 100 * SQY_LZ4Encode(input_data,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;
  std::cout << "diff_bswap1_lz4\t" 
	    << _args[1] << "\t"
	    << reference.data_in_byte() << "\t" 
	    << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
	    << ref_hist.smallest_populated_bin() << "\t"
	    << ref_hist.largest_populated_bin() << "\t"
	    << ref_hist.mode() << "\t"
	    << ref_hist.mean() << "\t"
	    << double(_output_size)/double(reference.data_in_byte())
    	    << "\n";

  return retcode;

}

int rmbkg_diff_bswap1_lz4_compress(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  if(reference.empty())
    return 1;

  unsigned long _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
    int retcode = SQY_RmBackground_Estimated_UI16(		reference.axis_lengths[0],
                  reference.axis_lengths[1],
                  reference.axis_lengths[2],
                  input_data,0
                                                 );

  retcode += SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					  input_data,intermediate_buffer);
  
  retcode += 10*SQY_BitSwap1Encode_I16(intermediate_buffer,input_data,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  long expected_size = input_length;
  retcode += SQY_LZ4_Max_Compressed_Length(&expected_size);

  char* final_buffer = new char[expected_size];
  long _output_size = reference.data_in_byte();
  retcode += 100 * SQY_LZ4Encode(input_data,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;
  std::cout << "rmbkg_diff_bswap1_lz4\t" 
	    << _args[1] << "\t"
	    << reference.data_in_byte() << "\t" 
	    << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
	    << ref_hist.smallest_populated_bin() << "\t"
	    << ref_hist.largest_populated_bin() << "\t"
	    << ref_hist.mode() << "\t"
	    << ref_hist.mean() << "\t"
	    << double(_output_size)/double(reference.data_in_byte())
    	    << "\n";

  return retcode;

}

int rmbkg_diff_bswap4_lz4_compress(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  if(reference.empty())
    return 1;

  unsigned long _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
  int retcode = SQY_RmBackground_Estimated_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2),
						input_data,0);

  retcode += SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					  input_data,intermediate_buffer);
  
  retcode += 10*SQY_BitSwap4Encode_I16(intermediate_buffer,input_data,reference.data_in_byte());
  
  long input_length = reference.data_in_byte();
  long expected_size = input_length;
  retcode += SQY_LZ4_Max_Compressed_Length(&expected_size);

  char* final_buffer = new char[expected_size];
  long _output_size = reference.data_in_byte();
  retcode += 100 * SQY_LZ4Encode(input_data,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;
  std::cout << "rmbkg_diff_bswap4_lz4\t" 
	    << _args[1] << "\t"
	    << reference.data_in_byte() << "\t" 
	    << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
	    << ref_hist.smallest_populated_bin() << "\t"
	    << ref_hist.largest_populated_bin() << "\t"
	    << ref_hist.mode() << "\t"
	    << ref_hist.mean() << "\t"
	    << double(_output_size)/double(reference.data_in_byte())
    	    << "\n";

  return retcode;

}



int diff_lz4_compress(const std::vector<std::string>& _args){

  tiff_fixture<unsigned short> reference(_args[1]);
  
  if(reference.empty())
    return 1;

  sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());
  unsigned long _input_size = reference.data_in_byte();
  char* input_data = reinterpret_cast<char*>(&reference.tiff_data[0]);
  char* intermediate_buffer = new char[_input_size];
  
  std::fill(intermediate_buffer, intermediate_buffer + _input_size,0);    
  
  
  int retcode = SQY_RasterDiffEncode_3D_UI16(reference.axis_length(0), reference.axis_length(1), reference.axis_length(2), 
					input_data,intermediate_buffer);
  
  long input_length = reference.data_in_byte();

  long expected_size = input_length;
  retcode += SQY_LZ4_Max_Compressed_Length(&expected_size);
  
  char* final_buffer = new char[expected_size];
  long _output_size = reference.data_in_byte();
  retcode += 10 * SQY_LZ4Encode(intermediate_buffer,
				 _input_size,
				 final_buffer,
				 &_output_size
				 );
  delete [] final_buffer;
  delete [] intermediate_buffer;
  std::cout << "diff_lz4\t" 
	    << _args[1] << "\t"
	    << reference.data_in_byte() << "\t" 
	    << reference.axis_length(0)<< "x" << reference.axis_length(1)<< "x" << reference.axis_length(2) << "\t"
	    << ref_hist.smallest_populated_bin() << "\t"
	    << ref_hist.largest_populated_bin() << "\t"
	    << ref_hist.mode() << "\t"
	    << ref_hist.mean() << "\t"
	    << double(_output_size)/double(reference.data_in_byte())
    	    << "\n";

  return retcode;

}

int main(int argc, char *argv[])
{

  typedef std::function<int(std::vector<std::string>) > func_t;
  std::unordered_map<std::string, func_t> prog_flow;
  prog_flow["help"] = func_t(help);
  prog_flow["-h"] = func_t(help);

  //compression benches
  prog_flow["rmbkg_diff_bswap4_lz4_compress"] = func_t(rmbkg_diff_bswap4_lz4_compress);
  prog_flow["diff_bswap4_lz4_compress"] = func_t(diff_bswap4_lz4_compress);
  
  prog_flow["rmbkg_diff_bswap1_lz4_compress"] = func_t(rmbkg_diff_bswap1_lz4_compress);
  prog_flow["diff_bswap1_lz4_compress"] = func_t(diff_bswap1_lz4_compress);
  prog_flow["bswap1_lz4_compress"] = func_t(bswap1_lz4_compress);

  prog_flow["diff_lz4_compress"] = func_t(diff_lz4_compress);



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
