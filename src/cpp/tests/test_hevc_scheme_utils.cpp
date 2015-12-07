#define BOOST_TEST_MODULE TEST_HEVC_SCHEME_UTILS
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>
#include <stdexcept>

#include "hevc_scheme_utils.hpp"
#include "volume_fixtures.hpp"

extern "C" {

#include <libavutil/opt.h>
#include <libavformat/avformat.h>

}

struct utils_fixture {

  AVCodecID id_;

  utils_fixture():
    id_(AV_CODEC_ID_HEVC)
  {
    av_register_all();
  }

  

};

BOOST_FIXTURE_TEST_SUITE( av_types, utils_fixture )

BOOST_AUTO_TEST_CASE( av_codec_constructs ){

  sqeazy::av_codec_t local(id_);
  BOOST_CHECK(local.get());
  BOOST_CHECK_EQUAL(local.get()->name,"libx265");
  BOOST_CHECK_EQUAL(local.get()->type,AVMEDIA_TYPE_VIDEO);
  
}

BOOST_AUTO_TEST_CASE( av_codec_constructs_ptr ){

  sqeazy::av_codec_t local(&id_);
  BOOST_CHECK(local.get());
  BOOST_CHECK_EQUAL(local.get()->name,"libx265");
  BOOST_CHECK_EQUAL(local.get()->type,AVMEDIA_TYPE_VIDEO);
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_constructs ){


  sqeazy::av_codec_context_t ctx(id_);
  
  BOOST_CHECK(ctx.get());
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_open_empty ){

  sqeazy::av_codec_t local(&id_);
  sqeazy::av_codec_context_t ctx(local);

  BOOST_CHECK(ctx.get());
  BOOST_CHECK_THROW(ctx.open(), std::runtime_error);
  //ctx.get()->codec_descriptor is not cleaned-up
  
}

BOOST_AUTO_TEST_CASE( av_codec_context_open ){

  sqeazy::av_codec_t local(&id_);
  sqeazy::av_codec_context_t ctx(local);

  ctx.get()->width = 124;
  ctx.get()->height = 456;
  ctx.get()->time_base = (AVRational){1,25};
  ctx.get()->gop_size = 10;
  ctx.get()->max_b_frames = 1;
  ctx.get()->pix_fmt = AV_PIX_FMT_YUV420P;

  av_opt_set(ctx.get()->priv_data, "preset", "ultrafast", 0);
  av_opt_set(ctx.get()->priv_data, "profile", "main", 0);
  av_opt_set(ctx.get()->priv_data, "x265-params", "lossless=1:log-level=warning", 0);
  
  BOOST_CHECK(ctx.get());
  ctx.open();
}

BOOST_AUTO_TEST_CASE( av_frame_constructs ){

  sqeazy::av_frame_t local;
  
  BOOST_CHECK(local.get());

}

BOOST_AUTO_TEST_CASE( av_frame_constructs_with_values ){

  sqeazy::av_frame_t local(123,456,AV_PIX_FMT_YUV420P);
  
  BOOST_CHECK(local.get());
  
  BOOST_CHECK(local.get()->width == 123);
}

BOOST_AUTO_TEST_CASE( sws_context_constructs ){

  sqeazy::av_frame_t from(123,456,AV_PIX_FMT_GRAY8);
  sqeazy::av_frame_t to(123,456,AV_PIX_FMT_YUV420P);
  
  sqeazy::sws_context_t local(from, to);

  BOOST_CHECK(local.get());
  
  
}

BOOST_AUTO_TEST_CASE( avformat_context_constructs ){

  sqeazy::av_format_context_t local;
  BOOST_CHECK(local.get());
  BOOST_CHECK(local.open_input()<0);
}

BOOST_AUTO_TEST_CASE( avio_context_constructs ){

  std::vector<uint8_t> dummy(100,42);
  sqeazy::avio_buffer_data buffer;
  buffer.ptr = &dummy[0];
  buffer.size = dummy.size();
  
  sqeazy::avio_context_t local(buffer);
  BOOST_CHECK(local.get());

}


BOOST_AUTO_TEST_SUITE_END()
