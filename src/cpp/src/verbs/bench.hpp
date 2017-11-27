#ifndef _SQY_BENCH_H_
#define _SQY_BENCH_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <ctime>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "tiff_utils.hpp"
//#include "deprecated/static_pipeline_select.hpp"
#include "sqeazy_algorithms.hpp"
#include "sqeazy_pipelines.hpp"

#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "sqeazy_algorithms.hpp"
#include "hdf5_utils.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;
namespace sqy = sqeazy;

using timings_t = std::vector<std::chrono::duration<double, std::micro >>;

namespace sqeazy {

  namespace benchmark_detail {

    long int unix_style_timestamp()
    {
      time_t t = std::time(0);
      long int now = static_cast<long int> (t);
      return now;
    }

    struct series {

      timings_t times_;
      std::vector<std::size_t> bytes_written_;

      std::vector<std::size_t> shape_;
      int sizeof_pixel_;
      std::string name_;

      series():
        times_(),
        bytes_written_(),
        shape_(),
        sizeof_pixel_(0),
        name_(){

      }

      series(const std::vector<std::size_t>& _shape, int _sizeof, std::string _name = "", std::size_t _len = 0):
        times_(),
        bytes_written_(),
        shape_(_shape),
        sizeof_pixel_(_sizeof),
        name_(_name.begin(), _name.end()){

        times_.reserve(_len);
        bytes_written_.reserve(_len);

      }

      void add(const std::chrono::duration<double, std::micro >& _dur,
               const std::size_t& _bytes){

        times_.push_back(_dur);
        bytes_written_.push_back(_bytes);

      }

      void print(bool _as_csv, bool _no_header, const std::string& _comment = "") const {

        const std::string delim = (_as_csv ? "," : "");

        if(!_no_header){

          std::cout << (!_as_csv ? std::setw(2 ) : std::setw(0)) << "id"             << delim
                    << (!_as_csv ? std::setw(15) : std::setw(0)) << "shape"          << delim
                    << (!_as_csv ? std::setw(15) : std::setw(0)) << "time_mus"       << delim
                    << (!_as_csv ? std::setw(15) : std::setw(0)) << "final_bytes"    << delim
                    << (!_as_csv ? std::setw(18) : std::setw(0)) << "ingest_bw_mbps" << delim
                    << (!_as_csv ? std::setw(15) : std::setw(0)) << "sizeof_pixel"   << delim
                    << (!_as_csv ? std::setw(15) : std::setw(0)) << "n_elements"     << delim;
          if(_as_csv){
            std::cout << (!_as_csv ? std::setw(20) : std::setw(0)) << "filename"       << delim
                      << (!_as_csv ? std::setw(10) : std::setw(0)) << "comment";
          }
          else {
            std::cout << " filename+comment";
          }
                         std::cout << "\n";

        }
        std::stringstream shape_str;
        for ( std::size_t el : shape_ ){
          shape_str << el << "x";
        }
        auto tmp = shape_str.str();
        const std::string shape_string(tmp.begin(), tmp.size()>0 ? tmp.end()-1 : tmp.end());
        const std::size_t len = std::accumulate(shape_.begin(), shape_.end(),1,std::multiplies<std::size_t>());
        float ingest_bw = 0.f;
        constexpr float in_mb = 1.f/(1024*1024);
        const float ingest_size_in_mb = float(len*sizeof_pixel_)*in_mb;

        for(std::size_t i = 0;i < times_.size();++i){
          std::chrono::duration<double> time_in_sec = times_[i];
          ingest_bw = ingest_size_in_mb/( time_in_sec.count() );
          std::cout << (!_as_csv ? std::setw( 2               ) : std::setw(0))  << i                 << delim
                    << (!_as_csv ? std::setw(15               ) : std::setw(0))  << shape_string      << delim
                    << (!_as_csv ? std::setw(15               ) : std::setw(0))  << times_[i].count() << delim
                    << (!_as_csv ? std::setw(15               ) : std::setw(0))  << bytes_written_[i] << delim
                    << (!_as_csv ? std::setw(18               ) : std::setw(0))  << ingest_bw         << delim
                    << (!_as_csv ? std::setw(15               ) : std::setw(0))  << sizeof_pixel_     << delim
                    << (!_as_csv ? std::setw(15               ) : std::setw(0))  << len               << delim
                    << (_as_csv ? "\"" : " ") << name_ << (_as_csv ? "\"" : ",") << delim
                    << (_as_csv ? "\"" : " ") << _comment << (_as_csv ? "\"" : "")
                    << "\n";
        }

      }
    };

    void print_results(const std::vector<series>& _data,
                       const po::variables_map& _config,
                       const std::string& _comment = ""){

      if(_data.empty())
        return;

      _data.front().print(_config.count("as-csv"), _config.count("noheader"), _comment);

      if(_data.size() == 1)
        return;

      for( std::size_t i = 1;i<_data.size();++i )
        _data[i].print(_config.count("as-csv"), _config.count("noheader"), _comment);

    }




    template <typename pipe_t>
    series native_bench_write(const sqeazy::tiff_facet& _input,
                              pipe_t& _pipeline,
                              bfs::path& _output_file,
                              int _n_repetitions= 10,
                              bool _verbose = false){

      typedef typename pipe_t::incoming_t raw_t;

      std::vector<size_t>	input_shape;
      std::vector<char>	output;
      size_t		expected_size_byte = _pipeline.max_encoded_size(_input.size_in_byte());
      // size_t                output_file_size = bfs::exists(_output_file) ? bfs::file_size(_output_file) : 0;

      //create clean output buffer
      if(expected_size_byte!=output.size())
        output.resize(expected_size_byte);

      std::fill(output.begin(), output.end(),0);

      //retrieve the size of the loaded buffer
      _input.dimensions(input_shape);

      series timings{input_shape, sizeof(raw_t), _input.location_.generic_string(),static_cast<std::size_t>(_n_repetitions)};

      //bench
      char* enc_end = nullptr;
      auto start = std::chrono::high_resolution_clock::now();
      auto end = start;

      for(int i = 0;i<_n_repetitions;++i){

        std::fill(output.begin(),output.end(),0);
        enc_end = nullptr;

        start = std::chrono::high_resolution_clock::now();
        enc_end = _pipeline.encode(reinterpret_cast<const raw_t*>(_input.data()),
                                   output.data(),
                                   input_shape);
        end = std::chrono::high_resolution_clock::now();
        timings.add(end - start, enc_end - output.data());

        if(enc_end == nullptr && _verbose) {
          std::cerr << "[SQY]\tnative benchmark of compression at iteration "<< i <<" failed! Exiting.\n";
          return series();
        }

      }


      ////////////////////////OUTPUT I/O///////////////////////////////
      const size_t compressed_length_byte = enc_end - output.data();

      std::fstream sqyfile;
      sqyfile.open(_output_file.generic_string(), std::ios_base::binary | std::ios_base::out );
      if(!sqyfile.is_open())
      {
        std::cerr << "[SQY]\tunable to open " << _output_file << "as output file. Skipping it!\n";
        sqyfile.clear();
        return series();
      }


      sqyfile.write(output.data(),compressed_length_byte);
      sqyfile.close();



      return timings;
    }

    template <typename pipe_t>
    series tiff_bench_write(const sqeazy::tiff_facet& _input,
                            pipe_t& _pipeline,
                            bfs::path& _output_file,
                            int _n_repetitions= 10,
                            bool _verbose = false){

      typedef typename pipe_t::incoming_t raw_t;

      std::vector<size_t>	input_shape;
      std::vector<char>	output;
      std::size_t		expected_size_byte = _pipeline.max_encoded_size(_input.size_in_byte());
      // size_t                output_file_size = bfs::exists(_output_file) ? bfs::file_size(_output_file) : 0;

      std::size_t		bytes_encoded =0;
      std::size_t        data_encoded =0;


      //create clean output buffer
      if(expected_size_byte!=output.size())
        output.resize(expected_size_byte);

      std::fill(output.begin(), output.end(),0);

      //retrieve the size of the loaded buffer
      _input.dimensions(input_shape);
      std::size_t		input_size = std::accumulate(input_shape.begin(), input_shape.end(),1,std::multiplies<std::size_t>());

      series timings{input_shape, sizeof(raw_t), _input.location_.generic_string(),static_cast<std::size_t>(_n_repetitions)};

//bench
      char* enc_end = nullptr;
      auto start = std::chrono::high_resolution_clock::now();
      auto end = start;

      for(int i = 0;i<_n_repetitions;++i){

        std::fill(output.begin(),output.end(),0);
        enc_end = nullptr;

        start = std::chrono::high_resolution_clock::now();
        enc_end = _pipeline.encode(reinterpret_cast<const raw_t*>(_input.data()),
                                   output.data(),
                                   input_shape);
        end = std::chrono::high_resolution_clock::now();
        timings.add(end - start, enc_end - output.data());

        if(enc_end == nullptr && _verbose) {
          std::cerr << "[SQY]\tnative benchmark of compression at iteration "<< i <<" failed! Exiting.\n";
          return series();
        }

      }

      // //bench
      // char* enc_end = _pipeline.encode(reinterpret_cast<const raw_t*>(_input.data()),
      //                                  output.data(),
      //                                  input_shape);

      // if(enc_end == nullptr && _verbose) {
      //   std::cerr << "[SQY::bench to tiff]\tnative benchion failed! Nothing to write to disk...\n";
      //   return 1;
      // }

      const std::string hdr = _pipeline.header(input_shape);
      bytes_encoded = enc_end - output.data();
      data_encoded = bytes_encoded - hdr.size();

      if(_pipeline.is_compressor() && (data_encoded % input_size) != 0) {
        std::cerr << "[SQY::bench to tiff]\t pipelines that performed compression which doesn't preserve dimensionality cannot be written\n";
        return series();
      }

      ////////////////////////OUTPUT I/O///////////////////////////////

      sqeazy::tiff_facet otiff(_output_file.generic_string());
      otiff.shape_ = input_shape;
      otiff.buffer_.resize(data_encoded);

      std::copy(output.begin()+hdr.size(),
                output.end(),
                otiff.buffer_.begin());

      otiff.write(_output_file.generic_string(),
                  sizeof(typename pipe_t::outgoing_t)*CHAR_BIT);



      return timings;
    }

    template <typename pipe_t>
    series h5_bench_write(const sqeazy::tiff_facet& _input,
                          pipe_t& _pipeline,
                          bfs::path& _output_file,
                          const std::string& _dname,
                          int _n_repetitions= 10,
                          bool _verbose = false){

      typedef typename pipe_t::incoming_t raw_t;

      std::vector<size_t>	input_shape;
      std::vector<char>	output;


      //retrieve the size of the loaded buffer
      _input.dimensions(input_shape);


      series timings{input_shape, sizeof(raw_t), _input.location_.generic_string(),static_cast<std::size_t>(_n_repetitions)};
      int rvalue = 0;

      auto start = std::chrono::high_resolution_clock::now();
      auto end = start;

      for(int i = 0;i<_n_repetitions;++i){
        rvalue = 0;
        bfs::remove(_output_file);

        start = std::chrono::high_resolution_clock::now();
        sqy::h5_file loaded(_output_file.generic_string(), H5F_ACC_TRUNC);
        if(!loaded.ready())
          return series();
        else{

          rvalue = loaded.write_nd_dataset(_dname,
                                           reinterpret_cast<const raw_t*>(_input.data()),
                                           input_shape.data(),
                                           input_shape.size(),
                                           _pipeline);

        }
        end = std::chrono::high_resolution_clock::now();
        timings.add(end - start, bfs::file_size(_output_file));
      }

      if(rvalue)
        return series();

      return timings;
    }

  };

};



namespace sqyb = sqeazy::benchmark_detail;


int bench_files(const std::vector<std::string>& _files,
                const po::variables_map& _config) {

  int value = 1;

  bfs::path			current_file;
  bfs::path			output_file;
  std::fstream			sqyfile;
  std::vector<size_t>		input_shape;
  sqeazy::tiff_facet		input;

  sqy::dypeline<std::uint16_t>	pipe16;
  sqy::dypeline_from_uint8	pipe8;

  std::vector<char>		output;


  const std::string pipeline_string = _config["pipeline"].as<std::string>();

  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline_string)){
    std::cerr << "[SQY]\tunable to build pipeline from " << pipeline_string << "\nDoing nothing.\n";
    return value;
  }

  pipe16 = sqy::dypeline<std::uint16_t>::from_string(pipeline_string);
  pipe8 = sqy::dypeline_from_uint8::from_string(pipeline_string);

  int nthreads_to_use = sqy::clean_number_of_threads(_config["nthreads"].as<int>());
  pipe16.set_n_threads(nthreads_to_use);
  pipe8.set_n_threads(nthreads_to_use);

  const int n_reps = _config["repetitions"].as<int>();

  if(_files.size()>1)
    std::cout << "[SQY]\tmultiple input files detected, ignoring --output_name flag\n";

  std::vector<sqyb::series> file_series(_files.size());
  int index = 0;
  for(const std::string& _file : _files) {


    current_file = _file;
    if(!bfs::exists(current_file)){
      std::cerr << "[SQY]\tunable to open " << _file << "\t skipping it\n";
      continue;
    }

    ////////////////////////INPUT I/O///////////////////////////////
    //load tiff & extract the dimensions
    input.load(_file);


    output_file = current_file.replace_extension(".sqy");

    ////////////////////////PRODUCE TARGET FILE NAME///////////////////////////////
    if(_config.count("output_suffix")){
      auto new_suffix = _config["output_suffix"].as<std::string>();

      if(new_suffix.front() == '.')
        output_file = current_file.replace_extension(_config["output_suffix"].as<std::string>());
      else{
        output_file = current_file.stem();
        output_file += new_suffix;
      }


    }

    if(_config.count("output_name") && _files.size()==1)
      output_file = _config["output_name"].as<std::string>();


    ////////////////////////BENCH & WRITE///////////////////////////////
    if(output_file.extension()==".sqy"){
      if(input.bits_per_sample()==16){
        file_series[index] = sqyb::native_bench_write(input,
                                                      pipe16,
                                                      output_file,
                                                      n_reps,
                                                      _config.count("verbose"));

      }
      else{
        file_series[index] = sqyb::native_bench_write(input,
                                                      pipe8,
                                                      output_file,
                                                      n_reps,
                                                      _config.count("verbose"));

      }
    }

    if(output_file.extension()==".tif"){
      if(input.bits_per_sample()==16){
        file_series[index] = sqyb::tiff_bench_write(input,
                                                    pipe16,
                                                    output_file,
                                                    n_reps,
                                                    _config.count("verbose"));
      }
      else{
        file_series[index] = sqyb::tiff_bench_write(input,
                                                    pipe8,
                                                    output_file,
                                                    n_reps,
                                                    _config.count("verbose"));

      }
    }


    if(output_file.extension()==".h5"){

      if(input.bits_per_sample()==16){
        file_series[index] = sqyb::h5_bench_write(input,
                                                  pipe16,
                                                  output_file,
                                                  _config["dataset_name"].as<std::string>(),
                                                  n_reps,
                                                  _config.count("verbose"));
      }
      else{
        file_series[index] = sqyb::h5_bench_write(input,
                                                  pipe8,
                                                  output_file,
                                                  _config["dataset_name"].as<std::string>(),
                                                  n_reps,
                                                  _config.count("verbose"));

      }
    }
    ++index;
  }

  std::stringstream cmt;
  cmt << _config["pipeline"].as<std::string>() << "|"
      << nthreads_to_use << "threads" << "|"
      << sqyb::unix_style_timestamp();

  std::string comment_string = cmt.str();
  if(_config["comment"].as<std::string>().size())
    comment_string = _config["comment"].as<std::string>();
  sqyb::print_results(file_series,_config,comment_string);


  if(!file_series.empty())
    return 0;
  else
    return value;

}


#endif /* _SQY_BENCH_H_ */
