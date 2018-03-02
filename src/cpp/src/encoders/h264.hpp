#ifndef _H264_HPP_
#define _H264_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "image_stack.hpp"

#include "dynamic_stage.hpp"



//TODO: what if ffmpeg is not available??
#ifndef AV_CODEC_ID_H264
extern "C" {

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>

}

#endif


#include <type_traits>

//#include "h264_scheme_impl.hpp"
#include "ffmpeg_video_encode_impl.hpp"
#include "string_parsers.hpp"




namespace sqeazy {

  template <typename in_type>
  using h264_scheme_base_type = typename binary_select_type<filter<in_type>,//true
                                                            sink<in_type>,//false
                                                            std::is_same<in_type,char>::value
                                                            >::type;

  template < typename in_type , typename S = std::size_t>
  struct h264_scheme :  public h264_scheme_base_type<in_type>
  {

    typedef typename binary_select_type<filter<in_type>,//true
                                        sink<in_type>,//false
                                        std::is_same<in_type,char>::value>::type base_t;

    typedef in_type raw_type;
    typedef typename base_t::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[h264_scheme] input type is non-arithmetic");
    static_assert(sizeof(raw_type)<3,"[h264_scheme] input type is not 1- or 2-byte wide (bit-depths larger than 16 are not supported yet)");

    std::string h264_config;
    parsed_map_t  config_map;

    //TODO: check syntax of h264 configuration at runtime
    h264_scheme(const std::string& _payload=""):
      h264_config(_payload),
      config_map()
      {

        if(_payload.empty())
          h264_config = "preset=ultrafast,qp=0";

        if(h264_config.find("threads")== std::string::npos){
          h264_config += ",threads=";
          h264_config += std::to_string(this->n_threads());
        }

        pipeline_parser p;
        config_map = p.minors(h264_config.begin(), h264_config.end());

        //TODO: catch this possibly throug the _payload
#ifndef SQY_TRACE
        av_log_set_level(AV_LOG_ERROR);
#else
        av_log_set_level(AV_LOG_DEBUG);
#endif

        av_register_all();
      }

    static const std::string description() { return std::string("h264 encode gray8 buffer with h264, args can be anything that libavcodec can understand, e.g. h264(preset=ultrafast,qp=0) would match the ffmpeg flags --preset=ultrafast --qp=0; see ffmpeg -h encoder=h264 for details.;if the input data is of 16-bit type, it is quantized by a min_of_4 difference scheme"); };

    std::string name() const override final {

      return std::string("h264");

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      return h264_config;

    }

    /**
       \brief given the size of some input in bytes, how large can the encoded data maximally be
       I have not yet found a function inside ffmpeg that might do this for me,
       right now, I assume 25% memory extra if lossless encoding is configured (overhead)

       \param[in]

       \return
       \retval

    */
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      auto found_qp = config_map.find("qp");
      if(found_qp!=config_map.end() && std::stoi(found_qp->second)==0)
        return _size_bytes*1.25;
      else
        return _size_bytes;

    }


    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length shape of input buffer
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in,
                             compressed_type* _out,
                             const std::vector<std::size_t>& _shape) override final {

      typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;

      local_size_type total_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());

      int drange = sizeof(raw_type)*CHAR_BIT;

      std::vector<compressed_type> temp_out;

      stack<raw_type> temp_in(_shape);

      std::copy(_in,_in+temp_in.num_elements(),temp_in.data());

      size_t num_written_bytes = 0;

      //normalize data FIXME: is that needed at all?
      if(sizeof(raw_type)!=1){
        const int max_value = *std::max_element(_in,_in + total_length);
        const int min_value = *std::min_element(_in,_in + total_length);
        drange = std::log(max_value - min_value)/std::log(2);
      }

      if(drange<17){
        stack_cref<raw_type> temp_in_cref = temp_in;
        num_written_bytes = ffmpeg_encode_stack<raw_type, AV_CODEC_ID_H264>(temp_in_cref,
                                                                            temp_out,
                                                                            config_map);
      }
      else {
        std::cerr << "found dynamic range of more than 16 bits! this is not implemented yet\n";
        num_written_bytes = 0;
      }

      if(num_written_bytes>(temp_out.size()*sizeof(compressed_type))){
        throw std::runtime_error("video codec has exceeded the alotted temp space, if this exception doesn't kill the process, a sigsegv will do");
      }

      if(num_written_bytes > 0)
        std::copy(temp_out.begin(), temp_out.end(),_out);

      std::intmax_t compressed_elements = num_written_bytes*sizeof(compressed_type);
      return _out+compressed_elements;

    }

    //FIXME: the output buffer will most likely be larger than the input buffer
    int decode( const compressed_type* _in, raw_type* _out,
                const std::vector<std::size_t>& _inshape,
                std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
        _outshape = _inshape;

      size_t _len_in = std::accumulate(_inshape.begin(), _inshape.end(),sizeof(compressed_type),std::multiplies<size_t>());
      size_t _len_out = std::accumulate(_outshape.begin(), _outshape.end(),1,std::multiplies<size_t>());

      const char* input = reinterpret_cast<const char*>(_in);

      std::size_t num_bytes_decoded = decode_stack(input, _len_in, _out, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded <= (_len_out*sizeof(raw_type)) ) ? SUCCESS : FAILURE;


    }


    ~h264_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return sink<raw_type>::is_compressor;

    }



  };


};//sqy namespace

#endif /* _H264_H_ */
