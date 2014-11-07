#ifndef _PIPELINE_SELECT_HPP_
#define _PIPELINE_SELECT_HPP_
#include <map>
#include <functional>
#include <utility>
#include <sstream>
#include <stdexcept>
#include "bench_common.hpp"

#include "boost/variant.hpp"
#include "boost/utility/enable_if.hpp"
#include <boost/type_traits.hpp>

namespace sqeazy_bench {

  struct pipeline_select {

    typedef std::pair<int, std::string> spec_t;

    spec_t current_;


    pipeline_select(int _n_bits, std::string _pipe_name = ""):
      current_(std::make_pair(_n_bits, _pipe_name)){
            
    }


    pipeline_select(spec_t _spec = std::make_pair(0,"") ):
      current_(_spec){
            
    }
    	           
    bool empty() const {
      
      return current_.first < 1 && current_.second.empty();
      
    }

    void set(spec_t _spec = std::make_pair(0,"")){
      current_ = _spec;
    }

    void set(int _n_bits, std::string _pipe_name = ""){
      current_ = std::make_pair(_n_bits, _pipe_name);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //MAX COMPRESSED SIZE
    
    typedef boost::variant<boost::blank, char_rmbkg_bswap1_lz4_pipe, char_bswap1_lz4_pipe, rmbkg_bswap1_lz4_pipe, bswap1_lz4_pipe> supported_pipes_t;
    supported_pipes_t pipeholder_;


    struct  give_max_compressed_size : public boost::static_visitor<unsigned long> {
    
      unsigned long len_in_byte;
      
      explicit give_max_compressed_size(unsigned long _in):
	len_in_byte(_in){}

      template <typename T>
      unsigned long operator()(T){
	return T::template max_encoded_size<unsigned long>(len_in_byte);
      }
      
      unsigned long operator()(boost::blank){
	return 0;
      }

    };

    unsigned long max_compressed_size(unsigned long _in_byte){
      
      give_max_compressed_size visitor(_in_byte);
      
      if(current_.second == char_rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = char_rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == char_bswap1_lz4_pipe::name())
	pipeholder_ = char_bswap1_lz4_pipe();

      if(current_.second == rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == bswap1_lz4_pipe::name())
	pipeholder_ = bswap1_lz4_pipe();

      if(!pipeholder_.which()){
	std::ostringstream msg;
	msg << "[pipeline_select]\t unknown pipeline "<< current_.second <<" queried in max_compressed_size\n";
	throw std::runtime_error(msg.str().c_str());
      }

      unsigned long value = boost::apply_visitor(visitor, pipeholder_);
      return value;
      
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////
    //COMPRESS

    typedef boost::variant< const boost::blank*, 
			   const unsigned char*,const unsigned short*> supported_types_t;
    supported_types_t typeholder_;
    
    struct  perform_compress : public boost::static_visitor<int> {

      // const char* input_buffer_;
      char* output_buffer_;
      std::vector<unsigned>* shape_;
      unsigned long* bytes_encoded_;
      
      explicit perform_compress(// const char* _in,
				char* _out,
				std::vector<unsigned>* _shape,
				unsigned long* _bytes_enc
				):
	// input_buffer_(_in),
	output_buffer_(_out),
	shape_(_shape),
	bytes_encoded_(_bytes_enc)
      {}


      template <typename T>
      int operator()(boost::blank, const T*){
      	return -1;

      }

      template <typename T>
      int operator()(T, const boost::blank*){
      	return -1;

      }

      int operator()(boost::blank, const boost::blank*){
      	return -1;

      }

      template <typename FirstT, typename SecondT>
      typename boost::enable_if_c<boost::is_same<FirstT,boost::blank>::value == false && 
				  boost::is_same<SecondT,typename FirstT::raw_type>::value == false,int>::type 
	operator()(FirstT, const SecondT*){
      	return -1;

      }      


      template <typename FirstT>
      typename boost::enable_if_c<boost::is_same<FirstT,boost::blank>::value == false,int>::type operator()(FirstT, const typename FirstT::raw_type* _input){
      	return FirstT::template compress<std::vector<unsigned>, unsigned long>(_input, output_buffer_, *shape_, *bytes_encoded_);

      }      
    };

    int compress(const char* _input, char* _output, std::vector<unsigned>& _dims, unsigned long& _num_encoded){
      
      perform_compress visitor(_output, &_dims, &_num_encoded);
      
      if(current_.second == char_rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = char_rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == char_bswap1_lz4_pipe::name())
	pipeholder_ = char_bswap1_lz4_pipe();

      if(current_.second == rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == bswap1_lz4_pipe::name())
	pipeholder_ = bswap1_lz4_pipe();

      if(!pipeholder_.which()){
	std::ostringstream msg;
	msg << "[pipeline_select]\t unknown pipeline "<< current_.second <<" queried in max_compressed_size\n";
	throw std::runtime_error(msg.str().c_str());
      }

      if(current_.first == sizeof(unsigned char)*CHAR_BIT)
	typeholder_ = reinterpret_cast<const unsigned char*>(_input);
      
      if(current_.first == sizeof(unsigned short)*CHAR_BIT)
	typeholder_ = reinterpret_cast<const unsigned short*>(_input);

      if(!typeholder_.which()){
	std::ostringstream msg;
	msg << "[pipeline_select]\t unknown bits "<< current_.first <<" queried in max_compressed_size\n";
	throw std::runtime_error(msg.str().c_str());
      }

      
      int value = boost::apply_visitor(visitor, pipeholder_, typeholder_);
      return value;
    }
    
  };




};
#endif /* _PIPELINE_SELECT_HPP_ */
