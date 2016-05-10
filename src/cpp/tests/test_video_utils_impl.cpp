#define BOOST_TEST_MODULE TEST_VIDEO_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include "encoders/video_utils.hpp"

#define DEBUG_H264


namespace sqy = sqeazy;


BOOST_AUTO_TEST_SUITE( avcodec_and_friends )

BOOST_AUTO_TEST_CASE( create_h264 ){

  av_register_all();

  AVCodecID id = AV_CODEC_ID_H264;
  
  sqeazy::av_codec_t test(id);

  BOOST_CHECK(test.ptr_!=nullptr);

}

BOOST_AUTO_TEST_CASE( create_h264_direct ){

  sqeazy::av_codec_t test(AV_CODEC_ID_H264);

  BOOST_CHECK(test.ptr_!=nullptr);

}

BOOST_AUTO_TEST_CASE( create_hevc ){

  AVCodecID id = AV_CODEC_ID_HEVC;
  
  sqeazy::av_codec_t test(id);

  BOOST_CHECK(test.ptr_!=nullptr);

}


BOOST_AUTO_TEST_SUITE_END()
