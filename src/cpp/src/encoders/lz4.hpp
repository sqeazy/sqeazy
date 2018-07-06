#ifndef _LZ4_HPP_
#define _LZ4_HPP_
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <future>
#include <string>
#include <typeinfo>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

// TODO: what if lz4 is not available??
#ifndef LZ4_VERSION_MAJOR
#include "lz4_utils.hpp"
#include "lz4frame.h"
#endif


namespace sqeazy
{



  // FIXME: lz4 can also be a filter for char->char conversions
  template <typename in_type>
  using lz4_scheme_base_type = typename binary_select_type<filter<in_type>,  // true
                                                           sink<in_type>,    // false
                                                           std::is_same<in_type, char>::value>::type;

  template <typename T, typename S = std::size_t> struct lz4_scheme : public lz4_scheme_base_type<T>
  {

    typedef S size_type;
    typedef lz4_scheme_base_type<T> sink_type;
    typedef T raw_type;
    typedef typename sink_type::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value == true, "[lz4_scheme] input type is non-arithmetic");
    static const std::string description()
      {
        return std::string("compress input with lz4, <accel|default = 1> improves compression speed at the price of compression ratio, <blocksize_kb|default = 256> ... the lz4 blocksize in kByte (possible values: 64/265/1024/4048), <framestep_kb|default = 256 > ... the atomic amount of input data to submit to lz4 "
                           "compression (required to be a multiple of the blocksize), <n_chunks_of_input|default = 0 > ... divide input data into n_encode_chunks partitions (consider each parition independent of the other for encoding; this overrides framestep_kb)");
      };

    std::string lz4_config;
    int acceleration;
    std::uint32_t blocksize_kb;
    std::uint32_t framestep_kb;
    std::uint32_t n_chunks_of_input;

    LZ4F_preferences_t lz4_prefs;

   // TODO: check syntax of lz4 configuration at runtime
    lz4_scheme(const std::string &_payload = "")
      : lz4_config(_payload)
      , acceleration(1)
      , blocksize_kb(256)
      , framestep_kb(256)
      , n_chunks_of_input(0)
      , lz4_prefs()
      {

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(), _payload.end());

        if(config_map.size())
        {
          auto f_itr = config_map.find("accel");
          if(f_itr != config_map.end())
            acceleration = std::stof(f_itr->second);

          f_itr = config_map.find("blocksize_kb");
          if(f_itr != config_map.end())
            blocksize_kb = std::stof(f_itr->second);

          f_itr = config_map.find("framestep_kb");
          if(f_itr != config_map.end())
            framestep_kb = std::stof(f_itr->second);

          f_itr = config_map.find("n_chunks_of_input");
          if(f_itr != config_map.end())
          {
            n_chunks_of_input = std::stof(f_itr->second);
          }
        }

        if(framestep_kb < blocksize_kb)
        {
          framestep_kb = blocksize_kb;
        }
        else
        {
          framestep_kb = std::round(framestep_kb / float(blocksize_kb)) * blocksize_kb;
        }

        if(n_chunks_of_input != 0)
          framestep_kb = 0;

        lz4_prefs.frameInfo =  {lz4::closest_blocksize::of(blocksize_kb),  // commonly L2 size on Intel platforms
                                LZ4F_blockLinked,
                                LZ4F_noContentChecksum,
                                LZ4F_frame,
                                0 /* content size unknown */,
                                0 /* no dictID */,
                                LZ4F_noBlockChecksum};
        lz4_prefs.compressionLevel = acceleration;
        lz4_prefs.autoFlush = 0;

        sqeazy::lz4::wrap<decltype(lz4_prefs)>::favorDecSpeed_initialisation(lz4_prefs,0);
      }


    std::string
    name() const override final
      {

        return std::string("lz4");
      }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override
      {

        std::ostringstream msg;
        msg << "accel=" << std::to_string(acceleration) << ",";
        msg << "blocksize_kb=" << blocksize_kb << ",";
        msg << "framestep_kb=" << framestep_kb << ",";
        msg << "n_chunks_of_input=" << n_chunks_of_input;
        return msg.str();
      }

    /**
     * helper function to calculate the size of a chunk of work
     */
    std::size_t bytes_per_chunk(std::intmax_t _size_bytes) const
      {

        std::intmax_t value = framestep_kb ? framestep_kb << 10 : std::ceil(_size_bytes / n_chunks_of_input);
        if(value >= _size_bytes || n_chunks_of_input >= _size_bytes)
        {
          value = _size_bytes;
        }

        return value;
      }

    /**
       \brief calculate the maximum size in bytes of the encoded buffer given an input of size _size_bytes; this function depends on the current state of this object with respect to framestep_kb, blocksize_kb and n_chunks_of_input. it over estimates the expected size of the encoded stream given the assumption of data
       that is incrompressible

       \return
       \retval intmax_t .. maximum size in bytes of the encoded buffer given an input of size _size_bytes

    */
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final
      {

        // if framestep_kb is set, use it; divide the payload into chunks otherwise
        std::intmax_t nbytes_per_chunk = bytes_per_chunk(_size_bytes);

        std::intmax_t value = LZ4F_HEADER_SIZE_MAX;  // for 1.8.0, test_lz4_sandbox.cpp:max_encoded_size/null_prefs

        if(nbytes_per_chunk >= _size_bytes)
          value += LZ4F_compressBound(nbytes_per_chunk, &lz4_prefs);
        else
        {

          std::size_t nchunks = (_size_bytes + nbytes_per_chunk - 1) / nbytes_per_chunk;
          std::size_t nchunks_per_thread = (nchunks + this->n_threads() - 1) / this->n_threads();  // overestimation on purpose

          std::size_t maxoutbytes_per_thread = nchunks_per_thread * (LZ4F_compressBound(nbytes_per_chunk, &lz4_prefs) + LZ4F_HEADER_SIZE_MAX);

          value = maxoutbytes_per_thread * this->n_threads();
        }

        return value;
      }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length number of char items in _in
     * @return pointer to last_element_written+1 in _out
     */
    compressed_type *encode(const raw_type *_in, compressed_type *_out, std::size_t _length) override final
      {
        std::vector<std::size_t> shape{1, _length};
        return encode(_in, _out, shape);
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
    compressed_type *encode(const raw_type *_in, compressed_type *_out, const std::vector<std::size_t> &_shape) override final
      {

        typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;

        const local_size_type len = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());
        const local_size_type bytes = len * sizeof(raw_type);

        const compressed_type *input = reinterpret_cast<const compressed_type *>(_in);
        const compressed_type *input_end = input + bytes;

        compressed_type *_out_end = _out + max_encoded_size(bytes);

        const local_size_type framestep_byte = bytes_per_chunk(bytes);
        const int nthreads = this->n_threads();

        compressed_type *value = nullptr;

        if(nthreads == 1)
        {
          value = lz4::encode_serial(input, input_end, _out, _out_end, framestep_byte, lz4_prefs);
        }
        else
        {
          value = lz4::encode_parallel(input, input_end, _out, _out_end, framestep_byte, lz4_prefs, nthreads);
        }

        return value;
      }


    int decode(const compressed_type *_in, raw_type *_out, const std::vector<std::size_t> &_inshape, std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final
      {

        if(_outshape.empty())
          _outshape = _inshape;

        size_type _len_in = std::accumulate(_inshape.begin(), _inshape.end(), 1, std::multiplies<size_type>());
        size_type _len_out = std::accumulate(_outshape.begin(), _outshape.end(), 1, std::multiplies<size_type>());

        return decode(_in, _out, _len_in, _len_out);
      }

    int decode(const compressed_type *_in, raw_type *_out, std::size_t _inlen, std::size_t _outlen = 0) const override final
      {

        int value = 1;  // failing

        if(!_outlen)
          _outlen = _inlen;

        compressed_type *dst = reinterpret_cast<compressed_type *>(_out);
        compressed_type *dstEnd = reinterpret_cast<compressed_type *>(_out + _outlen);

        const compressed_type *src = _in;
        const compressed_type *srcEnd = _in + _inlen;

        const std::size_t expected_bytes_decoded = _outlen * sizeof(raw_type);

        // if(this->n_threads()==1){
        LZ4F_dctx *dctx = nullptr;
        LZ4F_frameInfo_t info;

        std::size_t dstSize = std::distance(dst, dstEnd);
        std::size_t srcSize = std::distance(src, srcEnd);
        std::size_t ret = 1;
        while(src < srcEnd)
        {
          /* INVARIANT: Any data left in dst has already been written */
          dstSize = std::distance(dst, dstEnd);

          if(dctx == nullptr)
          {
            ret = LZ4F_createDecompressionContext(&dctx, 100);
            if(LZ4F_isError(ret))
            {
              std::cerr << "[sqy::lz4::decode] LZ4F_dctx creation error: " << LZ4F_getErrorName(ret) << "\n";
              return value;
            }
          }

          ret = LZ4F_getFrameInfo(dctx, &info, src, &srcSize);
          if(LZ4F_isError(ret))
          {
            break;
          }

          src += srcSize;
          srcSize = std::distance(src, srcEnd);

          if(std::ptrdiff_t(info.contentSize) > std::distance(dst, dstEnd))
          {
            std::cerr << "[sqy::lz4::decode] decompressed input yields " << info.contentSize << " Bytes, only " << std::distance(dst, dstEnd) << " Bytes left in destination\n";
            dst = reinterpret_cast<compressed_type *>(_out);
            break;
          }

          ret = LZ4F_decompress(dctx, dst, &dstSize, src, &srcSize,
                                /* LZ4F_decompressOptions_t */ NULL);
          if(LZ4F_isError(ret))
          {
            std::cerr << "[sqy::lz4::decode] Decompression error: " << LZ4F_getErrorName(ret) << "\n";
            dst = reinterpret_cast<compressed_type *>(_out);
            break;
          }

          src += srcSize;
          srcSize = std::distance(src, srcEnd);
          dst += dstSize;

          // lz4 frame API thinks the decompression is over
          if(ret == 0)
          {
            LZ4F_freeDecompressionContext(dctx);
            dctx = nullptr;
          }
        }

        //}

        std::size_t num_bytes_decoded = std::distance(reinterpret_cast<compressed_type *>(_out), dst) * sizeof(compressed_type);
        if(num_bytes_decoded > 0 && num_bytes_decoded <= expected_bytes_decoded)
          return 0;
        else
          return 1;
      }


    ~lz4_scheme(){};

    std::string output_type() const final override { return typeid(compressed_type).name(); }

    bool is_compressor() const final override { return sink<T>::is_compressor; }
  };
}
;  // sqy namespace


#endif /* _LZ4_H_ */
