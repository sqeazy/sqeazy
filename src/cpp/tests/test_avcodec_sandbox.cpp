#define BOOST_TEST_MODULE TEST_AVCODEC_SANDBOX
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <stdlib.h>
#include <stdio.h>
//#include <string.h>

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

#include <cmath>

#define DEBUG_HEVC

#include "volume_fixtures.hpp"
#include "tiff_utils.hpp"

static uint32_t write_encoded(const std::string& _filename, std::vector<uint8_t>& _video ){
  
  std::ofstream ofile(_filename, std::ios::binary | std::ios::out );
  
  for ( const uint8_t& c : _video )
    ofile << c;

  ofile.close();

  return 0;
}

template <typename T> struct av_pixel_type {};
template <> struct av_pixel_type<uint8_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY8;};
template <> struct av_pixel_type<uint16_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY16;};


/**
   \brief function that uses 
   
   \param[in] _volume stack that is to be encoded
   \param[out] _buffer stack that will contain the encoded stack
   
   \return 
   \retval number of bytes the encoded _buffer contains
   
*/
template <typename stack_type, AVCodecID codec_id = AV_CODEC_ID_HEVC >
static uint32_t encode_stack(const stack_type& _volume,
			     std::vector<uint8_t>& _buffer ,
			     const std::string& _debug_filename = ""){


  typedef typename stack_type::element raw_type;
  
  AVCodec *codec;
  AVCodecContext *c= NULL;
  int  ret,  got_output;
  AVFrame *frame;
  AVFrame *gray_frame;
  AVPacket pkt;
    

  uint32_t rcode = 0;
    
  /* find the mpeg1 video encoder */
  codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    std::cerr << "Codec not found\n";
    return rcode;
  }

  c = avcodec_alloc_context3(codec);
  if (!c) {
    std::cerr << "Could not allocate video codec context\n";
    return rcode;      
  }

  /* put sample parameters */
  // c->bit_rate = 400000;
    
  /* resolution must be a multiple of two due to YUV420p format*/
  c->width = _volume.shape()[2];
  c->height = _volume.shape()[1];
  /* frames per second */
  AVRational temp = { 1,25 };
	c->time_base = temp;
  /* emit one intra frame every ten frames
   * check frame pict_type before passing frame
   * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
   * then gop_size is ignored and the output of encoder
   * will always be I frame irrespective to gop_size
   */
  c->gop_size = 10;
  c->max_b_frames = 1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;

    
    
  if (codec_id == AV_CODEC_ID_HEVC){
    av_opt_set(c->priv_data, "preset", "ultrafast", 0);
    av_opt_set(c->priv_data, "profile", "main", 0);
    //http://x265.readthedocs.org/en/default/lossless.html#lossless-encoding
    std::stringstream params("");
	
    params << "lossless=1";
		
#ifdef DEBUG_HEVC
    params << ":log-level=info";
#else
    params << ":log-level=warning";
#endif
    av_opt_set(c->priv_data, "x265-params", params.str().c_str(), 0);
  }

  if (codec_id == AV_CODEC_ID_H264){
    av_opt_set(c->priv_data, "preset", "medium", 0);
    //https://trac.ffmpeg.org/wiki/Encode/H.264#LosslessH.264
    av_opt_set_int(c->priv_data, "qp", 0,0);
  }

  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0) {
    std::cerr << "Could not open codec\n";
    return rcode;
  }

  frame = av_frame_alloc();
  if (!frame) {
    std::cerr << "Could not allocate video frame\n";
    return rcode;
  }
    
  frame->format = c->pix_fmt;
  frame->width  = c->width;
  frame->height = c->height;

  gray_frame = av_frame_alloc();
  if (!gray_frame) {
    std::cerr << "Could not allocate video gray_frame\n";
    return rcode;
  }
    
  gray_frame->format = av_pixel_type<raw_type>::value;
  gray_frame->width  = c->width;
  gray_frame->height = c->height;
    
  /* create scaling context */
  struct SwsContext * sws_ctx = sws_getContext(gray_frame->width,gray_frame->height,av_pixel_type<raw_type>::value,
					       frame->width,frame->height,c->pix_fmt,
					       SWS_BICUBIC, NULL, NULL, NULL);
  if (!sws_ctx) {
    fprintf(stderr,
	    "Impossible to create scale context for the conversion "
	    "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
	    av_get_pix_fmt_name(av_pixel_type<raw_type>::value), c->width, c->height,
	    av_get_pix_fmt_name(AV_PIX_FMT_YUV420P), c->width, c->height);
    return rcode;
    
  }
    
  /* the image can be allocated by any means and av_image_alloc() is
   * just the most convenient way if av_malloc() is to be used */
    
    
  ret = av_image_alloc(frame->data,
		       frame->linesize,
		       c->width,
		       c->height,
		       c->pix_fmt, 32);
  if (ret < 0) {
    std::cerr << "Could not allocate raw YUV picture buffer\n";
    return rcode;
  }

  ret = av_image_alloc(gray_frame->data,
		       gray_frame->linesize,
		       gray_frame->width,
		       gray_frame->height,
		       av_pixel_type<raw_type>::value,
		       32 //align flag
		       );
  if (ret < 0) {
    std::cerr << "Could not allocate raw YUV picture buffer\n";
    return rcode;
  }    

  const uint32_t frame_size = _volume.shape()[2]*_volume.shape()[1];
  uint32_t idx = 0;

  uint32_t frames_encoded = 0;
  _buffer.clear();
  for (uint32_t z = 0; z < _volume.shape()[0]; z++) {
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    // fflush(stdout);
    idx = z*frame_size;

    avpicture_fill((AVPicture *)gray_frame, (const uint8_t*)&_volume.data()[idx],
    		   av_pixel_type<raw_type>::value,
    		   gray_frame->width,
    		   gray_frame->height);
   
	
    sws_scale(sws_ctx,
	      (const uint8_t * const*)gray_frame->data, gray_frame->linesize, 0, frame->height,
	      frame->data, frame->linesize);

    /* prepare a dummy image */
    // for (uint32_t y = 0; y < (uint32_t)c->height; y++) {
    //   for (uint32_t x = 0; x < (uint32_t)c->width; x++) {

    //     idx = z*frame_size;
    //     idx += y*_volume.shape()[0];
    //     idx += x;
	    
    //     //http://stackoverflow.com/questions/8349352/how-to-encode-grayscale-video-streams-with-ffmpeg	      
    //     frame->data[0][y * frame->linesize[0] + x] = 0.895 * _volume.data()[idx] + 16. ;
	    
    //   }
    // }

    // /* Cb and Cr */
    // for (uint32_t y = 0; y < c->height/2u; y++) {
    //     for (uint32_t x = 0; x < c->width/2u; x++) {
    //         frame->data[1][y * frame->linesize[1] + x] = 128;
    //         frame->data[2][y * frame->linesize[2] + x] = 128;
    //     }
    // }

    frame->pts = z;

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding frame\n";
	  
      return rcode;
    }

    if (got_output) {
      frames_encoded++;
      std::copy(pkt.data,pkt.data+pkt.size,std::back_inserter(_buffer));
      av_free_packet(&pkt);
    }

  }
    
  /* get the delayed frames */
  got_output = 1;
  for (uint32_t i = _volume.shape()[0]; got_output; i++) {
    //fflush(stdout);

    ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding [delayed] frame\n";
      return rcode;
    }

    if (got_output) {
      frames_encoded++;
      std::copy(pkt.data,pkt.data+pkt.size,std::back_inserter(_buffer));
      av_free_packet(&pkt);
    }
  }

  rcode = _buffer.size();

  //for debugging
  if(!_debug_filename.empty())
    write_encoded(_debug_filename,_buffer);

  //clean-up
  avcodec_close(c);
  av_free(c);

  av_freep(&frame->data[0]);
  av_frame_free(&frame);

  av_frame_free(&gray_frame);

  sws_freeContext(sws_ctx);
    
  return rcode;
}

/* required for reading encoded video from buffer */
//https://ffmpeg.org/doxygen/trunk/avio_reading_8c-example.html#a18
struct buffer_data {
    const uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	
    struct buffer_data *bd = (struct buffer_data *)opaque;
	using local_size_t = decltype(bd->size);

#ifdef _WIN32
	buf_size = min(static_cast<local_size_t>(buf_size), bd->size);
#else
    buf_size = std::min(static_cast<local_size_t>(buf_size), bd->size);
#endif

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
    return buf_size;
}

template <typename stack_type>
static uint32_t decode_stack(const std::vector<uint8_t>& _buffer,
			     stack_type& _volume){

  
  typedef typename stack_type::element pixel_type;
  
  uint32_t rcode = 0;
    
  AVFrame* frame = av_frame_alloc();
  if (!frame)
    {
      std::cerr << "[decode_stack] unable to allocate simple frame\n";
      return rcode;
    }

  AVFrame* gray_frame = av_frame_alloc();
  if (!gray_frame)
    {
      std::cerr << "[decode_stack] unable to allocate gray frame\n";
      return rcode;
    }
  
  gray_frame->format = av_pixel_type<pixel_type>::value;
  
  AVFormatContext* formatContext = NULL;
  formatContext = avformat_alloc_context();//TODO: handle errors!!

  //taken from https://ffmpeg.org/doxygen/trunk/avio_reading_8c-example.html#a18
  AVIOContext *avio_ctx = NULL;
  uint8_t *avio_ctx_buffer = NULL;
  size_t avio_ctx_buffer_size = 4096;

    
  buffer_data read_data = {0};
  read_data.ptr = &_buffer[0];
  read_data.size = _buffer.size();
      
  avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
  if (!avio_ctx_buffer) {
    std::cerr << "failed to allocated avio_ctx_buffer\n";
    //handle deallocation
    av_free(frame);
    av_free(gray_frame);
    avformat_close_input(&formatContext);
    av_freep(&avio_ctx_buffer);
    return rcode;
  }
    
  avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
				0, &read_data, &read_packet, NULL, NULL);
  if (!avio_ctx) {
      
    std::cerr << "failed to allocated avio_ctx_buffer\n";
    //handle deallocation
    av_free(frame);
    av_free(gray_frame);
    avformat_close_input(&formatContext);
    return rcode;
      
  }
  formatContext->pb = avio_ctx;
    
  if (avformat_open_input(&formatContext, "", NULL, NULL) != 0)
    {
      av_free(frame);
      av_free(gray_frame);
      avformat_close_input(&formatContext);
      if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
      }
      av_free(frame);
      std::cerr << "failed to open input\n";
      return rcode;
    }

  if (avformat_find_stream_info(formatContext, NULL) < 0)
    {
      av_free(frame);
      av_free(gray_frame);
      avformat_close_input(&formatContext);
      if (avio_ctx) {
	av_freep(&avio_ctx->buffer);
	av_freep(&avio_ctx);
      }
      std::cerr << "failed to find stream\n";
      return rcode;
    }

  if (formatContext->nb_streams < 1 ||
      formatContext->streams[0]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
    {
      av_free(frame);
      av_free(gray_frame);
      avformat_close_input(&formatContext);
      if (avio_ctx) {
	av_freep(&avio_ctx->buffer);
	av_freep(&avio_ctx);
      }
      std::cerr << "found stream is not a video stream\n";
      return rcode;
    }

  AVStream* stream = formatContext->streams[0];
  AVCodecContext* codecContext = stream->codec;

  codecContext->codec = avcodec_find_decoder(codecContext->codec_id);
  if (codecContext->codec == NULL)
    {
      av_free(frame);
      av_free(gray_frame);
      avcodec_close(codecContext);
      avformat_close_input(&formatContext);
      if (avio_ctx) {
	av_freep(&avio_ctx->buffer);
	av_freep(&avio_ctx);
      }
      std::cerr << "codec used in stream not found\n";
      return rcode;
    }
  else if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0)
    {
      av_free(frame);
      av_free(gray_frame);
      avcodec_close(codecContext);
      avformat_close_input(&formatContext);
      if (avio_ctx) {
	av_freep(&avio_ctx->buffer);
	av_freep(&avio_ctx);
      }

      std::cerr << "codec cannot be opened\n";
      
      return rcode;
    }

  
  AVPacket packet;
  av_init_packet(&packet);

  struct SwsContext * sws_ctx = 0;
  
  std::vector<pixel_type> temp;
  std::vector<uint32_t> shape(3,0);

  while (av_read_frame(formatContext, &packet) == 0)
    {
      if (packet.stream_index == stream->index)
        {
	  int frameFinished = 0;
	  avcodec_decode_video2(codecContext, frame, &frameFinished, &packet);

	  if (frameFinished)
            {

	      if(!(gray_frame->width && gray_frame->height)){

		gray_frame->width = frame->width;
		gray_frame->height = frame->height;

		int ret = av_image_alloc(gray_frame->data,
					 gray_frame->linesize,
					 gray_frame->width,
					 gray_frame->height,
					 av_pixel_type<pixel_type>::value,
					 32 //align flag
					 );
		if (ret < 0) {
		  std::cerr << "Could not allocate gray picture buffer\n";
		  av_free(frame);
		  av_free(gray_frame);
		  avcodec_close(codecContext);
		  avformat_close_input(&formatContext);
		  if (avio_ctx) {
		    av_freep(&avio_ctx->buffer);
		    av_freep(&avio_ctx);
		  }
		  sws_freeContext(sws_ctx);
		  throw std::runtime_error("unable to create gray_frame");
		}    
	      }
	      
	      if(!sws_ctx){
		sws_ctx = sws_getContext(frame->width,frame->height,codecContext->pix_fmt,
					 gray_frame->width,gray_frame->height,av_pixel_type<pixel_type>::value,
					 SWS_BICUBIC, NULL, NULL, NULL);
		if (!sws_ctx) {
		  fprintf(stderr,
			  "Impossible to create scale context for the conversion "
			  "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			  av_get_pix_fmt_name(av_pixel_type<pixel_type>::value), gray_frame->width, gray_frame->height,
			  av_get_pix_fmt_name(AV_PIX_FMT_YUV420P), frame->width, frame->height);
		  av_free(frame);
		  av_free(gray_frame);
		  avcodec_close(codecContext);
		  avformat_close_input(&formatContext);
		  if (avio_ctx) {
		    av_freep(&avio_ctx->buffer);
		    av_freep(&avio_ctx);
		  }
		  sws_freeContext(sws_ctx);
		  throw std::runtime_error("unable to create swscale context");
    		}
	      }
	      
	      
	      shape[2] = shape[2] != (uint32_t)frame->width ? frame->width : shape[2];
	      shape[1] = shape[1] != (uint32_t)frame->height ? frame->height : shape[1];
	      shape[0]++;

	      sws_scale(sws_ctx,
			(const uint8_t * const*)frame->data, frame->linesize, 0, frame->height,
			gray_frame->data, gray_frame->linesize);

	      for(uint32_t y=0;y<shape[1];++y){
		auto begin = gray_frame->data[0] + y*gray_frame->linesize[0];
		auto end = begin + gray_frame->width;
		std::copy(begin, end,std::back_inserter(temp));
	      } 

	    }
        }
    }


  int frameFinished = 1;

  while(frameFinished){

    packet.data = NULL;
    packet.size = 0;
    int err = avcodec_decode_video2(codecContext, frame, &frameFinished, &packet);
    if(err < 0 ){
      std::cerr << "[delayed]\tdecide error detected\n";
      break;
    }
      
    if (frameFinished)
      {
	   
	shape[2] = shape[2] != (uint32_t)frame->width ? frame->width : shape[2];
	shape[1] = shape[1] != (uint32_t)frame->height ? frame->height : shape[1];
	shape[0]++;

	sws_scale(sws_ctx,
		  (const uint8_t * const*)frame->data, frame->linesize, 0, frame->height,
		  gray_frame->data, gray_frame->linesize);

	for(uint32_t y=0;y<shape[1];++y){
	  auto begin = gray_frame->data[0] + y*frame->linesize[0];
	  auto end = begin + frame->width;
	  std::copy(begin, end,std::back_inserter(temp));
	}
      }
  }

  _volume.resize(shape);

  std::copy(temp.begin(), temp.end(),_volume.data());
  // std::transform(temp.begin(), temp.end(),_volume.data(),
  // 		 [](const uint8_t& _value){
  // 		   return (_value-16)/0.895;
  // 		 });
    
  av_free_packet(&packet);
  av_free(frame);
  av_free(gray_frame);
  avcodec_close(codecContext);
  avformat_close_input(&formatContext);
  if (avio_ctx) {
    av_freep(&avio_ctx->buffer);
    av_freep(&avio_ctx);
  }
  sws_freeContext(sws_ctx);
  
  return temp.size();

}


BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<uint8_t> )

BOOST_AUTO_TEST_CASE( encode_aligned ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);

  
  uint32_t bytes_written = encode_stack(embryo_,results)
    ;

  
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_EQUAL(bytes_written,results.size());


  /* check that results is not filled with 0s anymore */
  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);

  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());
  write_encoded("avcodec_sandbox_results_unqual_0_aligned.hevc",results);
  sqeazy::write_image_stack(embryo_,"aligned.tiff");
}

BOOST_AUTO_TEST_CASE( encode_unaligned ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);

  boost::multi_array<uint8_t, 3> unaligned = embryo_;

  uint32_t bytes_written = encode_stack(unaligned,results);

  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_EQUAL(bytes_written,results.size());

  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  sqeazy::write_image_stack(unaligned,"unaligned.tiff");
  write_encoded("avcodec_sandbox_results_unqual_0_unaligned.hevc",results);

}


BOOST_AUTO_TEST_CASE( roundtrip ){


  av_register_all();
  
  std::vector<uint8_t> encoded(embryo_.num_elements(),0);

  uint32_t bytes_written = encode_stack(embryo_,encoded);

  BOOST_CHECK_NE(bytes_written,0u);

  uint32_t bytes_produced = decode_stack(encoded,retrieved_);

  BOOST_CHECK_EQUAL(bytes_produced,embryo_.num_elements());

  for(uint32_t i = 0;i<decltype(embryo_)::dimensionality;++i)
    BOOST_CHECK_MESSAGE(embryo_.shape()[i] == retrieved_.shape()[i],
			      "dimension " << i << " doesn't match " << embryo_.shape()[i] << " != " << retrieved_.shape()[i]);

  std::vector<uint16_t> diff(retrieved_.num_elements());

  std::transform(retrieved_.data(),
		 retrieved_.data()+retrieved_.num_elements(),
		 embryo_.data(),
		 diff.begin(),
		 std::minus<uint16_t>());

    
  double l2norm = std::sqrt(std::inner_product(diff.begin(), diff.end(),diff.begin(),0.f));
  std::cout << "l2norm: " << l2norm << "\n";
    
  try{
    BOOST_REQUIRE_LT(l2norm,1);
    
    BOOST_REQUIRE_MESSAGE(std::equal(retrieved_.data(),
				     retrieved_.data()+retrieved_.num_elements(),
				     embryo_.data()),"raw volume and encoded/decoded volume do not match");
  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }
  
}

BOOST_AUTO_TEST_CASE( noisy_roundtrip ){

  av_register_all();
  
  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  uint32_t bytes_written = encode_stack(noisy_embryo_,encoded);

  BOOST_CHECK_NE(bytes_written,0u);

  uint32_t bytes_produced = decode_stack(encoded,retrieved_);

  BOOST_CHECK_EQUAL(bytes_produced,noisy_embryo_.num_elements());

  for(uint32_t i = 0;i<decltype(noisy_embryo_)::dimensionality;++i)
    BOOST_CHECK_MESSAGE(noisy_embryo_.shape()[i] == retrieved_.shape()[i],
			      "dimension " << i << " doesn't match " << noisy_embryo_.shape()[i] << " != " << retrieved_.shape()[i]);

  std::vector<uint16_t> diff(retrieved_.num_elements());

  std::transform(retrieved_.data(),
		 retrieved_.data()+retrieved_.num_elements(),
		 noisy_embryo_.data(),
		 diff.begin(),
		 std::minus<uint16_t>());

    
  double l2norm = std::sqrt(std::inner_product(diff.begin(), diff.end(),diff.begin(),0.f));
  std::cout << "l2norm: " << l2norm << "\n";
    
  try{
    BOOST_REQUIRE_LT(l2norm,1);
    
    BOOST_REQUIRE_MESSAGE(std::equal(retrieved_.data(),
				     retrieved_.data()+retrieved_.num_elements(),
				     noisy_embryo_.data()),"raw volume and encoded/decoded volume do not match");
  }
  catch(...){
    sqeazy::write_image_stack(noisy_embryo_,"noisy_embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"noisy_retrieved.tiff");
    throw;
  }
    
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( avcodec_16bit, sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( results_unqual_0 ){

  av_register_all();
  
  std::vector<uint8_t> results(embryo_.num_elements(),0);

  uint32_t bytes_written = encode_stack(embryo_,results);

  
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_EQUAL(bytes_written,results.size());


  /* check that results is not filled with 0s anymore */
  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());
  
}

// BOOST_AUTO_TEST_CASE( roundtrip ){


//   av_register_all();
  
//   std::vector<uint8_t> encoded(embryo_.num_elements(),0);

//   uint32_t bytes_written = encode_stack(embryo_,encoded,"encoded_16.hevc");

//   BOOST_CHECK_NE(bytes_written,0u);

//   uint32_t bytes_produced = decode_stack(encoded,retrieved_);

//   BOOST_CHECK_EQUAL(bytes_produced,embryo_.num_elements());

//   for(uint32_t i = 0;i<decltype(embryo_)::dimensionality;++i)
//     BOOST_CHECK_MESSAGE(embryo_.shape()[i] == retrieved_.shape()[i],
// 			      "dimension " << i << " doesn't match " << embryo_.shape()[i] << " != " << retrieved_.shape()[i]);

//   std::vector<uint16_t> diff(retrieved_.num_elements());

//   std::transform(retrieved_.data(),
// 		 retrieved_.data()+retrieved_.num_elements(),
// 		 embryo_.data(),
// 		 diff.begin(),
// 		 std::minus<uint16_t>());

    
//   double l2norm = std::sqrt(std::inner_product(diff.begin(), diff.end(),diff.begin(),0.f));
//   std::cout << "l2norm: " << l2norm << "\n";
    
//   try{
//     BOOST_REQUIRE_LT(l2norm,1);
    
//     BOOST_REQUIRE_MESSAGE(std::equal(retrieved_.data(),
// 				     retrieved_.data()+retrieved_.num_elements(),
// 				     embryo_.data()),"raw volume and encoded/decoded volume do not match");
//   }
//   catch(...){
//     sqeazy::write_image_stack(embryo_,"embryo_16.tiff");
//     sqeazy::write_image_stack(retrieved_,"retrieved_16.tiff");
//     throw;
//   }
    
// }
BOOST_AUTO_TEST_SUITE_END()
