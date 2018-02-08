#ifndef _LZ4_HPP_
#define _LZ4_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>
#include <future>
#include <algorithm>
#include <cmath>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "string_parsers.hpp"
#include "dynamic_stage.hpp"

//TODO: what if lz4 is not available??
#ifndef LZ4_VERSION_MAJOR
#include "lz4frame.h"
#include "lz4_utils.hpp"
#endif

namespace sqeazy {


  //FIXME: lz4 can also be a filter for char->char conversions
  template <typename in_type>
  using lz4_scheme_base_type = typename binary_select_type<filter<in_type>,//true
                                                           sink<in_type>,//false
                                                           std::is_same<in_type,char>::value
                                                           >::type;

  template < typename T , typename S = std::size_t>
  struct lz4_scheme :  public lz4_scheme_base_type<T> {

    typedef lz4_scheme_base_type<T> sink_type;
    typedef T raw_type;
    typedef typename sink_type::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[lz4_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("compress input with lz4, <accel|default = 1> improves compression speed at the price of compression ratio, <blocksize_kb|default = 256> ... the lz4 blocksize in kByte (possible values: 64/265/1024/4048), <framestep_kb|default = 32 > ... the atomic amount of input data to submit to lz4 compression, <n_chunks_of_input|default = 0 > ... divide input data into n_encode_chunks partitions (consider each parition independent of the other for encoding; this overrides framestep_kb)"); };

    std::string lz4_config;
    int acceleration;
    std::uint32_t blocksize_kb;
    std::uint32_t framestep_kb;
    std::uint32_t n_chunks_of_input;


    LZ4F_preferences_t lz4_prefs;

    //TODO: check syntax of lz4 configuration at runtime
    lz4_scheme(const std::string& _payload=""):
      lz4_config(_payload),
      acceleration(1),
      blocksize_kb(256),
      framestep_kb(16),
      n_chunks_of_input(0),
      lz4_prefs(){

      pipeline_parser p;
      auto config_map = p.minors(_payload.begin(),_payload.end());

      if(config_map.size()){
        auto f_itr = config_map.find("accel");
        if(f_itr!=config_map.end())
          acceleration = std::stof(f_itr->second);

        f_itr = config_map.find("blocksize_kb");
        if(f_itr!=config_map.end())
          blocksize_kb = std::stof(f_itr->second);

        f_itr = config_map.find("framestep_kb");
        if(f_itr!=config_map.end())
          framestep_kb = std::stof(f_itr->second);

        f_itr = config_map.find("n_chunks_of_input");
        if(f_itr!=config_map.end()){
          n_chunks_of_input = std::stof(f_itr->second);

        }
      }

      if(n_chunks_of_input!=0)
        framestep_kb = 0;

      lz4_prefs = {
        { lz4::closest_blocksize::of(blocksize_kb), //commonly L2 size on Intel platforms
          LZ4F_blockLinked,
          LZ4F_noContentChecksum,
          LZ4F_frame,
          0 /* content size unknown */,
          0 /* no dictID */ ,
          LZ4F_noBlockChecksum },
        acceleration,   /* compression level */
        0,   /* autoflush */
        { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
      };

    }




    std::string name() const override final {

      return std::string("lz4");

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      std::ostringstream msg;
      msg << "accel=" << std::to_string(acceleration) << ",";
      msg << "blocksize_kb=" << blocksize_kb << ",";
      msg << "framestep_kb=" << framestep_kb << ",";
      msg << "n_chunks_of_input=" << n_chunks_of_input ;
      return msg.str();


    }


/**
       \brief calculate the maximum size in bytes of the encoded buffer given an input of size _size_bytes

       \return
       \retval intmax_t .. maximum size in bytes of the encoded buffer given an input of size _size_bytes

    */
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      //if framestep_kb is set, use it; divide the payload into chunks otherwise
      std::intmax_t input_chunk_bytes = framestep_kb ? framestep_kb << 10 : std::ceil(_size_bytes/n_chunks_of_input);
      if(input_chunk_bytes >= _size_bytes || n_chunks_of_input >= _size_bytes){
        input_chunk_bytes = _size_bytes;
      }

      std::intmax_t lz4_bound_per_chunk = LZ4F_compressBound(input_chunk_bytes, &lz4_prefs);

      if(input_chunk_bytes >= _size_bytes)
        return lz4_bound_per_chunk;
      else{
        std::size_t n_steps = (_size_bytes + input_chunk_bytes - 1 )/ input_chunk_bytes;

        return lz4_bound_per_chunk*n_steps;
      }
    }

/**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length number of char items in _in
     * @return pointer to last_element_written+1 in _out
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::size_t _length) override final {
      std::vector<std::size_t> shape{1,_length};
      return encode(_in,_out,shape);
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _shape mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return pointer to end of payload
     *
     * frame footer and multithreading information https://github.com/lz4/lz4/issues/445
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, const std::vector<std::size_t>& _shape) override final {

      typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;

      const local_size_type total_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());
      const local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      const compressed_type* input = reinterpret_cast<const compressed_type*>(_in);
      const compressed_type* input_end = input + total_length_in_byte;

      compressed_type* _out_end = _out + max_encoded_size(total_length_in_byte);

      local_size_type framestep_byte = framestep_kb ? framestep_kb << 10 : std::ceil(total_length_in_byte/n_chunks_of_input);
      if(framestep_byte >= total_length_in_byte || n_chunks_of_input >= total_length_in_byte){
        framestep_byte = total_length_in_byte;
      }


      const int nthreads = this->n_threads();

      compressed_type* value = nullptr;

      if(nthreads==1){
        //lz4_prefs.frameInfo.contentSize = total_length_in_byte;
        value = lz4::encode_serial(input,input_end,
                                   _out,_out_end,
                                   framestep_byte,
                                   lz4_prefs
          );
        //lz4_prefs.frameInfo.contentSize = 0;

      } else {
        value = lz4::encode_parallel(input,input_end,
                                     _out,_out_end,
                                     framestep_byte,
                                     lz4_prefs,
                                     nthreads);
      }

      return value;
    }



    int decode( const compressed_type* _in, raw_type* _out,
                const std::vector<std::size_t>& _inshape,
                std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
        _outshape = _inshape;

      size_type _len_in = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<size_type>());
      size_type _len_out = std::accumulate(_outshape.begin(), _outshape.end(),1,std::multiplies<size_type>());

      return decode(_in,_out,_len_in,_len_out);
    }

    int decode( const compressed_type* _in, raw_type* _out,
                std::size_t _inlen,
                std::size_t _outlen=0)  const override final {

      int value = 1;//failing

      if(!_outlen)
        _outlen = _inlen;

      compressed_type* dst = reinterpret_cast<compressed_type*>(_out);
      compressed_type* dstEnd = reinterpret_cast<compressed_type*>(_out+_outlen);

      const compressed_type* src = _in;
      const compressed_type* srcEnd = _in + _inlen;

      const std::size_t expected_bytes_decoded = _outlen*sizeof(raw_type);

      //if(this->n_threads()==1){
      LZ4F_dctx *dctx = nullptr;
      LZ4F_frameInfo_t info;

      std::size_t dstSize = std::distance(dst,dstEnd);
      std::size_t srcSize = std::distance(src,srcEnd);
      std::size_t ret = 1;
      while (src < srcEnd ) {
        /* INVARIANT: Any data left in dst has already been written */
        dstSize = std::distance(dst,dstEnd);

        if(dctx == nullptr){
          ret = LZ4F_createDecompressionContext(&dctx, 100);
          if (LZ4F_isError(ret)) {
            std::cerr << "[sqy::lz4::decode] LZ4F_dctx creation error: " << LZ4F_getErrorName(ret) << "\n";
            return value;
          }
        }

        ret = LZ4F_getFrameInfo(dctx, &info, src, &srcSize);
        if (LZ4F_isError(ret)) {
          break;
        }

        src += srcSize;
        srcSize = std::distance(src,srcEnd);

        if (std::ptrdiff_t(info.contentSize) > std::distance(dst,dstEnd)) {
          std::cerr << "[sqy::lz4::decode] decompressed input yields " << info.contentSize
                  << " Bytes, only "<< std::distance(dst,dstEnd) << " Bytes left in destination\n";
          dst = reinterpret_cast<compressed_type*>(_out);
          break;
        }

        ret = LZ4F_decompress(dctx, dst, &dstSize,
                              src, &srcSize,
                              /* LZ4F_decompressOptions_t */ NULL);
        if (LZ4F_isError(ret)) {
          std::cerr << "[sqy::lz4::decode] Decompression error: " << LZ4F_getErrorName(ret) << "\n";
          dst = reinterpret_cast<compressed_type*>(_out);
          break;
        }

        src += srcSize;
        srcSize = std::distance(src,srcEnd);
        dst += dstSize;

        //lz4 frame API thinks the decompression is over
        if(ret == 0){
          LZ4F_freeDecompressionContext(dctx);
          dctx = nullptr;
        }

      }

      //}

      std::size_t num_bytes_decoded = std::distance(reinterpret_cast<compressed_type*>(_out),dst)*sizeof(compressed_type);
      if(num_bytes_decoded > 0 && num_bytes_decoded<=expected_bytes_decoded)
        return 0;
      else
        return 1;

    }



    ~lz4_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return sink<T>::is_compressor;

    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    typedef S size_type;

    static S last_num_encoded_bytes;

    static const bool is_sink = true;

    static const std::string static_name() {

      return std::string("lz4");

    }


    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    template <typename SizeType>
    static const error_code static_encode(const raw_type* _input,
                                          compressed_type* _output,
                                          std::vector<SizeType>& _dims,//size of _input
                                          size_type& _bytes_written = last_num_encoded_bytes
      ) {

      typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
      local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
      local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      local_size_type max_payload_length_in_byte = LZ4_compressBound(total_length_in_byte);

      const compressed_type* input = reinterpret_cast<const compressed_type*>(_input);

#if LZ4_VERSION_NUMBER > 10701
      size_type num_written_bytes = LZ4_compress_default(input,
                                                         _output,
                                                         total_length_in_byte,
                                                         max_payload_length_in_byte);
#else
      size_type num_written_bytes = LZ4_compress_limitedOutput(input,
                                                               _output,
                                                               total_length_in_byte,
                                                               max_payload_length_in_byte);
#endif
      if(num_written_bytes > 0) {
        _bytes_written = num_written_bytes;
        last_num_encoded_bytes = _bytes_written;
        return SUCCESS;
      }
      else {
        _bytes_written = 0;
        last_num_encoded_bytes = 0;
        return FAILURE;
      }

    }


    /**
     * @brief encode _input using the lz4 library, the output contains the number of input elements encoded as 64bit signed integer (long) and the encoded data
     *
     * @param _input array/buffer that contains data of raw_type
     * @param _output the char buffer that contains the compressed data (not truncated, not owned)
     * @param _length size of _input (unmutable)
     * @param _bytes_written number of bytes/elements in _output that actually contain data
     * @return sqeazy::error_code SUCCESS if non-zero number of bytes have been encoded by lz4
     */
    static const error_code static_encode(const raw_type* _input,
                                          compressed_type* _output,
                                          const size_type& _length,//size of _input
                                          size_type& _bytes_written = last_num_encoded_bytes
      ) {

      std::vector<size_type> artificial_dims(1);
      artificial_dims.front() = _length;

      return static_encode(_input, _output, artificial_dims, _bytes_written);

    }

    /**
     * @brief decode the input data stream _input with lz4 (the number of output elements is extracted from the first 64
     * bit of the input stream)
     *
     * @param _input input buffer as char
     * @param _output output buffer (needs to be allocated outside this function)
     * @param _length number of elements in _input
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const compressed_type* _input,
                                          raw_type* _output,
                                          const size_type& _len_in,
                                          const size_type& _len_out
      ) {

      char* output = reinterpret_cast<char*>(_output);
      const char* input = reinterpret_cast<const char*>(_input);

      size_type num_bytes_decoded = LZ4_decompress_safe(input,output,_len_in, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long static_max_encoded_size(U _src_length_in_bytes) {

      unsigned long lz4_bound = LZ4_compressBound(_src_length_in_bytes);

      return lz4_bound;

    }


  };

  template < typename T , typename S>
  S lz4_scheme<T,S>::last_num_encoded_bytes = 0;


};//sqy namespace


#endif /* _LZ4_H_ */
