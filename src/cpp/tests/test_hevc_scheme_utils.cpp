#define BOOST_TEST_MODULE TEST_HEVC_SCHEME_UTILS
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>
#include <stdexcept>

#include "encoders/video_utils.hpp"
#include "volume_fixtures.hpp"

extern "C" {

#include <libavutil/opt.h>
#include <libavformat/avformat.h>

}

static void warn_on_enabled_codecs(){

  av_register_all();
  sqeazy::av_codec_t l264(AV_CODEC_ID_H264);
  bool h264_works = l264.get()!=nullptr; 

  if(!h264_works)
    std::cout << "failed to instantiate h264, check if you ffmpeg installation supports the h264 ENCODER AND DECODER\n";

  sqeazy::av_codec_t l265(AV_CODEC_ID_HEVC);
  bool h265_works = l265.get()!=nullptr; 

  if(!h265_works)
    std::cout << "failed to instantiate h265, check if you ffmpeg installation supports the h265 ENCODER AND DECODER\n";
}




struct utils_fixture {

  AVCodecID hevc_id_;
  AVCodecID h264_id_;

  utils_fixture():
    hevc_id_(AV_CODEC_ID_HEVC),
    h264_id_(AV_CODEC_ID_H264)
  {
    av_register_all();
    warn_on_enabled_codecs();
  }

  

};


BOOST_FIXTURE_TEST_SUITE( av_types, utils_fixture )

BOOST_AUTO_TEST_CASE( av_codec_constructs ){

  sqeazy::av_codec_t l264(h264_id_);
  BOOST_REQUIRE_NO_THROW(l264.get());
  BOOST_REQUIRE(l264.get());
  BOOST_CHECK_EQUAL(l264.get()->name,"libx264");
  BOOST_CHECK_EQUAL(l264.get()->type,AVMEDIA_TYPE_VIDEO);

  sqeazy::av_codec_t local(hevc_id_);
  BOOST_REQUIRE_NO_THROW(local.get());
  BOOST_REQUIRE(local.get());
  BOOST_CHECK_EQUAL(local.get()->name,"libx265");
  BOOST_CHECK_EQUAL(local.get()->type,AVMEDIA_TYPE_VIDEO);


  
}

BOOST_AUTO_TEST_CASE( av_codec_constructs_ptr ){

  sqeazy::av_codec_t local(&hevc_id_);
  BOOST_REQUIRE(local.get());
  BOOST_CHECK_EQUAL(local.get()->name,"libx265");
  BOOST_CHECK_EQUAL(local.get()->type,AVMEDIA_TYPE_VIDEO);
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_constructs ){


  sqeazy::av_codec_context_t ctx(hevc_id_);
  
  BOOST_CHECK(ctx.get());
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_open_empty ){

  sqeazy::av_codec_t local(&hevc_id_);
  sqeazy::av_codec_context_t ctx(local);

  BOOST_CHECK(ctx.get());
  BOOST_CHECK_THROW(ctx.open(), std::runtime_error);
  //ctx.get()->codec_descriptor is not cleaned-up
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_open ){

  sqeazy::av_codec_t local(&hevc_id_);
  sqeazy::av_codec_context_t ctx(local);

  BOOST_REQUIRE_NO_THROW(ctx.get());
  BOOST_REQUIRE(ctx.get());
  ctx.get()->width = 124;
  ctx.get()->height = 456;
  AVRational temp = { 1,25 };
  ctx.get()->time_base = temp;
  ctx.get()->gop_size = 10;
  ctx.get()->max_b_frames = 1;
  ctx.get()->pix_fmt = AV_PIX_FMT_YUV420P;

  av_opt_set(ctx.get()->priv_data, "preset", "ultrafast", 0);
  av_opt_set(ctx.get()->priv_data, "profile", "main", 0);
  av_opt_set(ctx.get()->priv_data, "x265-params", "lossless=1:log-level=warning", 0);

  BOOST_REQUIRE_NO_THROW(ctx.get());
  BOOST_REQUIRE(ctx.get());
  BOOST_REQUIRE_NO_THROW(ctx.open());
}

BOOST_AUTO_TEST_CASE( av_frame_constructs ){

  sqeazy::av_frame_t local;
  
  BOOST_REQUIRE(local.get());

}

BOOST_AUTO_TEST_CASE( av_frame_constructs_with_values ){

  sqeazy::av_frame_t local(123,456,AV_PIX_FMT_YUV420P);
  
  BOOST_REQUIRE(local.get());
  
  BOOST_CHECK(local.get()->width == 123);
}

BOOST_AUTO_TEST_CASE( sws_context_constructs ){

  sqeazy::av_frame_t from(123,456,AV_PIX_FMT_GRAY8);
  sqeazy::av_frame_t to(123,456,AV_PIX_FMT_YUV420P);
  
  sqeazy::sws_context_t local(from, to);

  BOOST_REQUIRE(local.get());
  
  
}

BOOST_AUTO_TEST_CASE( avformat_context_constructs ){

  sqeazy::av_format_context_t local;
  BOOST_REQUIRE(local.get());
  BOOST_CHECK(local.open_input()<0);
}

BOOST_AUTO_TEST_CASE( avio_context_constructs ){

  std::vector<uint8_t> dummy(100,42);
  sqeazy::avio_buffer_data buffer;
  buffer.ptr =  reinterpret_cast<decltype(buffer.ptr)>(dummy.data());
  buffer.size = dummy.size();
  
  sqeazy::avio_context_t local(buffer);
  BOOST_REQUIRE(local.get());

}


BOOST_AUTO_TEST_SUITE_END()
