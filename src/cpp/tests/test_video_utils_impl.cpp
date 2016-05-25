#define BOOST_TEST_MODULE TEST_VIDEO_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include "encoders/video_utils.hpp"

#define DEBUG_H264


namespace sqy = sqeazy;

struct frame_fixture {

  std::vector<std::uint8_t>	byte_buffer;
  std::size_t 			byte_width;
  std::size_t 			byte_height;
  
  frame_fixture():
    byte_buffer(),
    byte_width(64),
    byte_height(32){
    
    byte_buffer.resize(byte_width*byte_height);
    std::size_t count = 0;
    for(std::uint8_t& el : byte_buffer)
      el = count++;

  }
};

struct av_frame_fixture{


  std::size_t 		fix_width;
  std::size_t 		fix_height;
  std::size_t 		fix_size;
  sqeazy::av_frame_t	gray8;
  sqeazy::av_frame_t	yuv420;
  sqeazy::av_frame_t	yuv420_zeros;
  std::vector<std::uint8_t> ref;
  
  av_frame_fixture():
    fix_width(64),
    fix_height(32),
    fix_size(fix_width*fix_height),
    gray8(fix_width,fix_height,sqeazy::av_pixel_type<std::uint8_t>::value),
    yuv420(fix_width,fix_height,sqeazy::av_pixel_type<sqy::yuv420p>::value),
    yuv420_zeros(fix_width,fix_height,sqeazy::av_pixel_type<sqy::yuv420p>::value),
    ref(fix_size,0)
  {
    std::size_t count = 0;

    for(std::size_t y = 0;y<fix_height;++y){
      for(std::size_t x = 0;x<fix_width;++x){

	std::uint8_t val = count % 255;
	
	ref[count] = val;
	
	gray8.get()->data[0][y*(gray8.get()->linesize[0])+x] = val;
	yuv420.get()->data[0][y*(yuv420.get()->linesize[0])+x] = val ;
	yuv420_zeros.get()->data[0][y*(yuv420_zeros.get()->linesize[0])+x] = val;

	count++;
	
      }
    }

    for(std::size_t y = 0;y<fix_height/2;++y){
      for(std::size_t x = 0;x<fix_width/2;++x){
	yuv420.get()->data[1][y*(yuv420.get()->linesize[1])+x] = 128;
	yuv420.get()->data[2][y*(yuv420.get()->linesize[2])+x] = 64;

	yuv420_zeros.get()->data[1][y*(yuv420_zeros.get()->linesize[1])+x] = 0;
	yuv420_zeros.get()->data[2][y*(yuv420_zeros.get()->linesize[2])+x] = 0;
      }
    }
  }
  
};


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


BOOST_FIXTURE_TEST_SUITE( AVFrame_related, frame_fixture )

BOOST_AUTO_TEST_CASE( create_frame ){

  
  sqeazy::av_frame_t local(64,32,sqeazy::av_pixel_type<std::uint8_t>::value);

  BOOST_CHECK_EQUAL(local.get()->width,64);
  BOOST_CHECK_EQUAL(local.get()->height,32);
  
}

BOOST_AUTO_TEST_CASE( fill_frame ){

  
  sqeazy::av_frame_t gray_frame(64,32,sqeazy::av_pixel_type<std::uint8_t>::value);

  BOOST_CHECK_EQUAL(gray_frame.get()->width,64);
  BOOST_CHECK_EQUAL(gray_frame.get()->height,32);

  std::size_t bytes_copied = 0;

  const std::size_t nb_lines = gray_frame.get()->height;
  for(std::size_t y=0; y < nb_lines;++y){
    auto gray_frame_begin = gray_frame.get()->data[0] + y*gray_frame.get()->linesize[0];
    auto byte_buffer_begin = byte_buffer.begin() + y*gray_frame.get()->width;
    
    std::copy(byte_buffer_begin,
	      byte_buffer_begin + gray_frame.get()->width,
	      gray_frame_begin);
    bytes_copied += gray_frame.get()->width*sizeof(std::uint8_t);
  }

  BOOST_CHECK_EQUAL(bytes_copied,byte_buffer.size());
  size_t byte_offset  = 0;
  size_t frame_offset = 0;
  size_t line_offset = 0;

  for(int i = 0;i<3;++i){
    for(std::size_t x=0; x < byte_width;++x){
      line_offset  = (i/3.*byte_height);
      byte_offset  = byte_width*line_offset;
      frame_offset = gray_frame.get()->linesize[0]*line_offset;
      BOOST_CHECK_EQUAL(byte_buffer[x+byte_offset],gray_frame.get()->data[0][x+frame_offset]);
    }
  }
  
}

BOOST_AUTO_TEST_CASE( roundtrip_frame ){
  
  sqeazy::av_frame_t gray_frame(64,32,sqeazy::av_pixel_type<std::uint8_t>::value);
  
  
  const std::size_t nb_lines = gray_frame.get()->height;
  for(std::size_t y=0; y < nb_lines;++y){
    auto gray_frame_begin = gray_frame.get()->data[0] + y*gray_frame.get()->linesize[0];
    auto byte_buffer_begin = byte_buffer.begin() + y*gray_frame.get()->width;
    
    std::copy(byte_buffer_begin,
	      byte_buffer_begin + gray_frame.get()->width,
	      gray_frame_begin);

  }

  auto roundtrip = byte_buffer;
  std::fill(roundtrip.begin(), roundtrip.end(),0);

  
  std::size_t bytes_copied = 0;

  int rval = av_image_copy_to_buffer(roundtrip.data(),
				     roundtrip.size(),
				     (const std::uint8_t* const*)gray_frame.get()->data,
				     (gray_frame.get()->linesize),
				     gray_frame.pixel_format(),
				     gray_frame.w(),
				     gray_frame.h(),
				     byte_width
				     );

  if(rval >= 0)
    bytes_copied += rval;

  BOOST_CHECK_EQUAL(bytes_copied,byte_buffer.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(byte_buffer.begin(), byte_buffer.end(),
				roundtrip.begin(), roundtrip.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( av_frame_functions, av_frame_fixture )

BOOST_AUTO_TEST_CASE( gray_frame_av_copy ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = av_image_copy_to_buffer(output.data(),
					      output.size(),
					      (const std::uint8_t* const*)gray8.get()->data,
					      (gray8.get()->linesize),
					      gray8.pixel_format(),
					      gray8.w(),
					      gray8.h(),
					      fix_width
					      );
  BOOST_CHECK_GT(bytes_copied,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(output.begin(), output.begin()+10,
				  ref.begin(), ref.begin()+10);
  BOOST_CHECK_EQUAL_COLLECTIONS(output.begin(), output.end(),
				ref.begin(), ref.end());
  
}

BOOST_AUTO_TEST_CASE( yuv420_frame_av_copy ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = av_image_copy_to_buffer(output.data(),
					      output.size(),
					      (const std::uint8_t* const*)yuv420.get()->data,
					      (yuv420.get()->linesize),
					      yuv420.pixel_format(),
					      yuv420.w(),
					      yuv420.h(),
					      fix_width
					      );
  BOOST_CHECK_LT(bytes_copied,0);//fmpeg 3.0 forbids this
  
}

BOOST_AUTO_TEST_CASE( yuv420_zeros_frame_av_copy ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = av_image_copy_to_buffer(output.data(),
					      output.size(),
					      (const std::uint8_t* const*)yuv420_zeros.get()->data,
					      (yuv420_zeros.get()->linesize),
					      yuv420_zeros.pixel_format(),
					      yuv420_zeros.w(),
					      yuv420_zeros.h(),
					      fix_width
					      );
  BOOST_CHECK_LT(bytes_copied,0);//fmpeg 3.0 forbids this
  
}


BOOST_AUTO_TEST_CASE( gray_frame_to_vector ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = sqy::y_to_vector(gray8, output);
  BOOST_REQUIRE_GT(bytes_copied,0);
  for(int i = 0;i<10;++i)
    BOOST_REQUIRE_MESSAGE(output[i] == ref[i], "frame_to_vector produces wrong result, ref["<< i <<"] =  "<< (int)ref[i] <<" became " << (int)output[i]);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(output.begin(), output.end(),
				ref.begin(), ref.end());
  
}

BOOST_AUTO_TEST_CASE( yuv420_frame_to_vector ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = sqy::y_to_vector(yuv420, output);
  BOOST_REQUIRE_GT(bytes_copied,0);
  for(int i = 0;i<10;++i)
    BOOST_REQUIRE_MESSAGE(output[i] == ref[i], "frame_to_vector produces wrong result, ref["<< i <<"] =  "<< (int)ref[i] <<" became " << (int)output[i]);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(output.begin(), output.end(),
				ref.begin(), ref.end());
  
}

BOOST_AUTO_TEST_CASE( yuv420_zeros_frame_to_vector ){

  std::vector<std::uint8_t> output(ref.size(),0);
  auto bytes_copied = sqy::y_to_vector(yuv420_zeros, output);
  BOOST_REQUIRE_GT(bytes_copied,0);
  for(int i = 0;i<10;++i)
    BOOST_REQUIRE_MESSAGE(output[i] == ref[i], "frame_to_vector produces wrong result, ref["<< i <<"] =  "<< (int)ref[i] <<" became " << (int)output[i]);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(output.begin(), output.end(),
				ref.begin(), ref.end());
  
}

BOOST_AUTO_TEST_SUITE_END()

