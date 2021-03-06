#ifndef _HEVC_SCHEME_IMPL_H_
#define _HEVC_SCHEME_IMPL_H_

#include <sstream>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>

extern "C" {

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

}

//#include "hevc_scheme_utils.hpp"
#include "sqeazy_algorithms.hpp"
#include "video_io.hpp"
#include "video_utils.hpp"

namespace sqeazy {

  namespace detail {

    static std::map<std::string,std::string> default_hevc_config = {
      {"preset", "ultrafast"},
      {"profile", "main"},
      {"lossless", "1"},
      {"min-keyint", "1"},
#ifdef DEBUG_HEVC
#ifndef TRACE_HEVC
      {"log-level","info"}
#else
      {"log-level","full"}
#endif
#else
      {"log-level","warning"}
#endif
    };

    /**
       \brief function that uses

       \param[in] _volume stack that is to be encoded
       \param[in] _shape stack shape as in [z,y,x] dimensions
       \param[out] _buffer stack that will contain the encoded stack

       \return
       \retval number of bytes the encoded _buffer contains

    */
    template <typename raw_type, AVCodecID codec_id =  AV_CODEC_ID_HEVC>
    static uint32_t hevc_encode_stack(const raw_type* _volume,
                      const std::vector<uint32_t>& _shape,
                      std::vector<char>& _buffer ,
                      const std::map<std::string,std::string>& _config = default_hevc_config,
                      const std::string& _debug_filename = ""){


      uint32_t bytes_written = 0;

      sqeazy::av_codec_t codec(codec_id);
      sqeazy::av_codec_context_t ctx(codec);

      /* resolution must be a multiple of two due to YUV420p format*/
      ctx.get()->width = _shape[row_major::w];
      ctx.get()->height = _shape[row_major::h];

      /* frames per second */
      ctx.get()->time_base = (AVRational){1,25};
      /* emit one intra frame every ten frames
       * check frame pict_type before passing frame
       * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
       * then gop_size is ignored and the output of encoder
       * will always be I frame irrespective to gop_size
       */
      // ctx.get()->gop_size = 10;
      // ctx.get()->max_b_frames = 1;
      ctx.get()->pix_fmt = AV_PIX_FMT_YUV420P;



      if (codec_id == AV_CODEC_ID_HEVC){

    //http://x265.readthedocs.org/en/default/lossless.html#lossless-encoding
    std::stringstream params("");
    int count = 0;
    for( const auto & kv : _config )
      {
        if(kv.first == "preset" || kv.first == "profile"){
          av_opt_set(ctx.get()->priv_data, kv.first.c_str(), kv.second.c_str(), 0);
        }
        else {
          params << kv.first << "=" << kv.second;
          if(count<(_config.size()-1))
        params << ":";
        }
      }

    av_opt_set(ctx.get()->priv_data, "x265-params", params.str().c_str(), 0);
      }

      try{
    ctx.open();
      }
      catch(std::exception& exc){
    std::cerr << __FILE__ << ":" << __LINE__ << "\t" << exc.what();
    return bytes_written;
      }

      int  ret,  got_output;
      sqeazy::av_frame_t frame(ctx.get()->width,ctx.get()->height, ctx.get()->pix_fmt);
      sqeazy::av_frame_t gray_frame(ctx.get()->width,ctx.get()->height,sqeazy::av_pixel_type<raw_type>::value);
      sqeazy::sws_context_t scale_ctx(gray_frame, frame);

      AVPacket pkt;


      const uint32_t frame_size = _shape[_shape.size()-1]*_shape[_shape.size()-2];
      const uint32_t num_frames = _shape[0];
      uint32_t idx = 0;

      uint32_t frames_encoded = 0;
      _buffer.clear();
      for (uint32_t z = 0; z < num_frames; z++) {
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    // fflush(stdout);
    idx = z*frame_size;

    // avpicture_fill((AVPicture *)gray_frame.get(), (const uint8_t*)&_volume[idx],
    //           av_pixel_type<raw_type>::value,
    //           gray_frame.get()->width,
    //           gray_frame.get()->height);

    for (uint32_t y = 0; y < _shape[1]; y++) {
      auto src_begin = &_volume[idx] + y*gray_frame.get()->width;
      auto src_end = src_begin + gray_frame.get()->width;
      auto dst_begin = &gray_frame.get()->data[0][y*gray_frame.get()->linesize[0]];
      std::copy(src_begin,src_end,dst_begin);
    }


    sws_scale(scale_ctx.get(),
          gray_frame.get()->data,
          gray_frame.get()->linesize,
          0,
          frame.get()->height,
          frame.get()->data,
          frame.get()->linesize);

    frame.get()->pts = z;

    /* encode the image */
    ret = avcodec_encode_video2(ctx.get(),
                    &pkt,
                    frame.get(),
                    &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding frame\n";

      return bytes_written;
    }

    if (got_output) {
      frames_encoded++;
      std::copy(pkt.data,pkt.data+pkt.size,std::back_inserter(_buffer));
      sqeazy::av_free_packet(&pkt);
    }

      }

      /* get the delayed frames */
      got_output = 1;
      for (uint32_t i = num_frames; got_output; i++) {
    //fflush(stdout);

    ret = avcodec_encode_video2(ctx.get(), &pkt, NULL, &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding [delayed] frame\n";
      return bytes_written;
    }

    if (got_output) {
      frames_encoded++;
      std::copy(pkt.data,pkt.data+pkt.size,std::back_inserter(_buffer));
      sqeazy::av_free_packet(&pkt);
    }
      }

      bytes_written = _buffer.size();

      //for debugging
      if(!_debug_filename.empty())
    write_encoded(_debug_filename,_buffer);

      sqeazy::av_free_packet(&pkt);

      return bytes_written;
    }


    /**
       \brief function that uses

       \param[in] _buffer stack that contains the encoded stack
       \param[out] _volume stack that is to be decoded
       \param[out] _shape decoded stack shape as in [x,y,z] dimensions


       \return
       \retval number of bytes that were written to _volume

    */
    template <typename raw_type, typename in_type = char>
    static uint32_t hevc_decode_stack(const in_type* _buffer,
                      const uint32_t& _buffer_len,
                      raw_type* _volume,
                      const uint32_t& _volume_len// ,
                      // std::vector<uint32_t>& _shape
                      ){


      uint32_t rcode = 0;


      sqeazy::avio_buffer_data read_this;
      read_this.ptr = _buffer;
      read_this.size = _buffer_len;

      sqeazy::avio_context_t avio_ctx(read_this);

      if (!avio_ctx.get()) {

    std::cerr << "failed to allocated avio_ctx_buffer\n";
    //handle deallocation
    return rcode;

      }

      sqeazy::av_format_context_t formatContext;

      if (formatContext.open_input(avio_ctx.get()) != 0)
    {

      std::cerr << "failed to open input\n";
      return rcode;
    }

      if (formatContext.find_stream_info() < 0)
    {
      std::cerr << "failed to find stream\n";
      return rcode;
    }

      if (formatContext.get()->nb_streams < 1 ||
      formatContext.get()->streams[0]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
    {
      std::cerr << "found stream is not a video stream\n";
      return rcode;
    }

      AVStream* stream = formatContext.get()->streams[0];
      sqeazy::av_codec_t decoder(stream->codec->codec_id, false);
      sqeazy::av_codec_context_t codec_ctx(decoder);
      if(codec_ctx.open()!=0){
    std::cerr << "decoder codec cannot be opened\n";
    return rcode;
      }


      AVPacket packet;
      av_init_packet(&packet);

      // struct SwsContext * sws_ctx = 0;
      std::shared_ptr<sqeazy::sws_context_t> sws_ctx(nullptr);

      std::vector<raw_type> temp;
      std::vector<uint32_t> shape(3,0);

      sqeazy::av_frame_t frame;
      sqeazy::av_frame_t gray_frame(0,0,av_pixel_type<raw_type>::value);

      while (av_read_frame(formatContext.get(), &packet) == 0)
    {
      if (packet.stream_index == stream->index)
        {
          int frameFinished = 0;
          avcodec_decode_video2(codec_ctx.get(), frame.get(), &frameFinished, &packet);

          if (frameFinished)
        {

          if(!(gray_frame.get()->width && gray_frame.get()->height)){

            gray_frame.get()->width = frame.get()->width;
            gray_frame.get()->height = frame.get()->height;

            int ret = av_image_alloc(gray_frame.get()->data,
                         gray_frame.get()->linesize,
                         gray_frame.get()->width,
                         gray_frame.get()->height,
                         av_pixel_type<raw_type>::value,
                         32 //align flag
                         );
            if (ret < 0) {
              std::cerr << "Could not allocate gray picture buffer\n";

              // avcodec_close(codec_ctx);
              throw std::runtime_error("unable to create gray_frame");
            }
          }

          if(!sws_ctx.get()){
            sws_ctx = std::make_shared<sqeazy::sws_context_t>(frame,gray_frame);
            if (!sws_ctx->get()) {
              fprintf(stderr,
                  "Impossible to create scale context for the conversion "
                  "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                  av_get_pix_fmt_name(av_pixel_type<raw_type>::value), gray_frame.get()->width, gray_frame.get()->height,
                  av_get_pix_fmt_name((AVPixelFormat)frame.get()->format), frame.get()->width, frame.get()->height);

              throw std::runtime_error("unable to create swscale context");
            }
          }


          shape[2] = shape[2] != (uint32_t)frame.get()->width ? frame.get()->width : shape[2];
          shape[1] = shape[1] != (uint32_t)frame.get()->height ? frame.get()->height : shape[1];
          shape[0]++;

          sws_scale((*sws_ctx).get(),
                frame.get()->data,//cast to sws_scale type
                frame.get()->linesize,
                0,
                frame.get()->height,
                gray_frame.get()->data,
                gray_frame.get()->linesize);

          for(uint32_t y=0;y<shape[1];++y){
            auto begin = gray_frame.get()->data[0] + y*gray_frame.get()->linesize[0];
            auto end = begin + gray_frame.get()->width;
            std::copy(begin, end,std::back_inserter(temp));
          }

        }
        }
    }


      int frameFinished = 1;

      while(frameFinished){

    packet.data = NULL;
    packet.size = 0;
    int err = avcodec_decode_video2(codec_ctx.get(), frame.get(), &frameFinished, &packet);
    if(err < 0 ){
      std::cerr << "[delayed]\tdecide error detected\n";
      break;
    }

    if (frameFinished)
      {

        shape[2] = shape[2] != (uint32_t)frame.get()->width ? frame.get()->width : shape[2];
        shape[1] = shape[1] != (uint32_t)frame.get()->height ? frame.get()->height : shape[1];
        shape[0]++;

        sws_scale((*sws_ctx).get(),
              frame.get()->data,
              frame.get()->linesize,
              0,
              frame.get()->height,
              gray_frame.get()->data,
              gray_frame.get()->linesize);

        for(uint32_t y=0;y<shape[1];++y){
          auto begin = gray_frame.get()->data[0] + y*frame.get()->linesize[0];
          auto end = begin + frame.get()->width;
          std::copy(begin, end,std::back_inserter(temp));
        }
      }
      }


      if(temp.size() <= _volume_len)
    std::copy(temp.begin(), temp.end(),_volume);

      sqeazy::av_free_packet(&packet);

      return temp.size()*sizeof(raw_type);

    }


  }; //detail namespace

}; //sqeazy namespace
#endif /* _HEVC_SCHEME_UTILS_H_ */
