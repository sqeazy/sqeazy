#ifndef _HEVC_SCHEME_UTILS_H_
#define _HEVC_SCHEME_UTILS_H_

#include <memory>

#include "boost/align/aligned_allocator.hpp"

extern "C" {
  
  #include <libavcodec/avcodec.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
  #include <libavformat/avformat.h>
  
}

namespace sqeazy {

  struct av_codec_t {

    typedef AVCodec contained_type;
    
    contained_type* ptr_;

    av_codec_t(const AVCodecID& _id, bool is_encoder = true):
      ptr_(nullptr){

      if(is_encoder)
	ptr_ = avcodec_find_encoder(_id);
      else
	ptr_ = avcodec_find_decoder(_id);
      
      if (!ptr_) {
	std::cerr << "[av_codec_t] Could not allocate video codec context\n";
      }
      
    }

    av_codec_t(AVCodecID* _id, bool is_encoder = true):
      ptr_(nullptr){

      if(is_encoder)
	ptr_ = avcodec_find_encoder(*_id);
      else
	ptr_ = avcodec_find_decoder(*_id);
      
      if (!ptr_) {
	std::cerr << "[av_codec_t] Could not allocate video codec context\n";
      }
    }

    //valgrind: no additional free required
    // ~av_codec_type(){
    //   if(ptr_)
    // 	delete ptr_;
    // }

    contained_type* get() {
      return ptr_;
    }

    contained_type const * get() const {
      return ptr_;
    }

    
  };
  
  struct av_codec_context_t {

    typedef AVCodecContext contained_type;
    
    contained_type * ptr_;
    av_codec_t codec_;

    
    av_codec_context_t(AVCodecID *_codec):
      ptr_(nullptr),
      codec_(_codec)
    {

      ptr_ = avcodec_alloc_context3(codec_.get());
      if (!ptr_) {
	std::cerr << "[av_codec_context_t] Could not allocate video codec context\n";
      }
    }

    av_codec_context_t(const AVCodecID& _codec):
      ptr_(nullptr),
      codec_(_codec){

      ptr_ = avcodec_alloc_context3(codec_.get());
      if (!ptr_) {
	std::cerr << "[av_codec_context_t] Could not allocate video codec context\n";
      }
      
    }

    av_codec_context_t(const av_codec_t& _codec):
      ptr_(nullptr),
      codec_(_codec){

      ptr_ = avcodec_alloc_context3(codec_.get());
      if (!ptr_) {
	std::cerr << "[av_codec_context_t] Could not allocate video codec context\n";
      }
      
    }


    
    int open(AVCodec* _aux_codec = 0){
      /* open it */
      int rcode = 0;
      if ((rcode = avcodec_open2(ptr_, (_aux_codec) ? _aux_codec : codec_.get(), NULL)) < 0) {
	std::stringstream msg;
	if(!_aux_codec)
	  msg << "[av_codec_context_t] Could not open codec\n";
	else
	  msg << "[av_codec_context_t] Could not open codec from external AVCodecContext\n";
	throw std::runtime_error(msg.str());
      }
      return rcode;
    }
    
    contained_type* get() {
      return ptr_;
    }

    contained_type const * get() const {
      return ptr_;
    }


    void clear(){
      if(ptr_){
	avcodec_close(ptr_);
	avcodec_free_context(&ptr_);
	ptr_ = 0;
      }
    }
    
    ~av_codec_context_t(){
      clear();
    }

  };

  template <int align_by = 32>
  struct av_frame_t_any {

    //TODO: some bytes seem to be still reachable by valgrind with this approach!
    
    std::shared_ptr<AVFrame> ptr_;
    
    static void how_to_delete_me(AVFrame* _frame){
      if(_frame){
	if(&_frame->data[0])
	  av_freep(&_frame->data[0]);
      	av_frame_free(&_frame);
      }

    };
    
    av_frame_t_any(    uint32_t _width = 0, 
		 uint32_t _height= 0,
		 int32_t  _format = -1
		 ):
      ptr_(av_frame_alloc(), how_to_delete_me){

      ptr_->format = _format;
      ptr_->width  = _width;
      ptr_->height = _height;

      if(_width && _height)
	av_image_alloc(ptr_->data,
		       ptr_->linesize,
		       ptr_->width,
		       ptr_->height,
		       (AVPixelFormat)ptr_->format,
		       align_by//arch dependent
		       );
    }


    AVFrame* get(){
      return ptr_.get();
    }

    
    AVFrame const* get() const {
      return ptr_.get();
    }
    
  };

  //TODO: we could detect the current architecture
  typedef av_frame_t_any<32> av_frame_t;
  

  struct sws_context_t {

    typedef SwsContext contained_type;
    
    std::shared_ptr<contained_type> ptr_;
    static void how_to_delete_me(contained_type* _ctx){
      if(_ctx)
	sws_freeContext(_ctx);
    };

    template <typename frame_type>
    sws_context_t( const frame_type& _from, const frame_type& _to, int _method = SWS_BICUBIC):
      ptr_(nullptr){
      
      contained_type * temp = sws_getContext(_from.get()->width,_from.get()->height,(AVPixelFormat)_from.get()->format,
						_to.get()->width,_to.get()->height,(AVPixelFormat)_to.get()->format,
						_method, NULL, NULL, NULL);
      if (!temp) {
	fprintf(stderr,
		"Impossible to create scale context for the conversion "
		"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
		av_get_pix_fmt_name((AVPixelFormat)_from.get()->format), _from.get()->width, _from.get()->height,
		av_get_pix_fmt_name((AVPixelFormat)_to.get()->format), _to.get()->width, _to.get()->height
		);
      } else {

	ptr_ = std::shared_ptr<contained_type>(temp,how_to_delete_me);
      }
    }

    contained_type* get(){
      return ptr_.get();
    }

    contained_type const* get() const {
      return ptr_.get();
    }
    
  };

  struct av_format_context_t {

    typedef AVFormatContext contained_type;
    
    std::shared_ptr<contained_type> ptr_;
    
    static void how_to_delete_me(contained_type* _ctx){
      if(_ctx){
	if(_ctx->iformat)
	  avformat_close_input(&_ctx);
	avformat_free_context(_ctx);
      }
    };

    av_format_context_t():
      ptr_(nullptr){
      
      ptr_ = std::shared_ptr<contained_type>(avformat_alloc_context(),how_to_delete_me);

    }

    contained_type* get(){
      return ptr_.get();
    }

    contained_type const* get() const {
      return ptr_.get();
    }

    int open_input(AVIOContext * _avio = nullptr){

      int rcode = -1;
      if(_avio){
	ptr_.get()->pb = _avio;
	contained_type* temp = ptr_.get();
	rcode = avformat_open_input(&temp, "", NULL, NULL);
      }

      return rcode;
    }

    int open_input(const std::string& _filename){

      int rcode = -1;
      if(!_filename.empty()){
	contained_type* temp = ptr_.get();
	rcode = avformat_open_input(&temp, _filename.c_str(), NULL, NULL);
      }

      return rcode;
    }
    
    int find_stream_info(){

      int rcode = 0;
      contained_type* temp = ptr_.get();      
      rcode = avformat_find_stream_info(temp, NULL);
      return rcode;
      
    }
    
  };

    /* required for reading encoded video from buffer */
//https://ffmpeg.org/doxygen/trunk/avio_reading_8c-example.html#a18
struct avio_buffer_data {
    const uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
  avio_buffer_data *bd = (avio_buffer_data *)opaque;
  buf_size = std::min((decltype(bd->size))buf_size, bd->size);
  
  /* copy internal buffer data to buf */
  std::copy(bd->ptr, bd->ptr + buf_size,buf);
  // memcpy(buf, bd->ptr, buf_size);
  bd->ptr  += buf_size;
  bd->size -= buf_size;
  return buf_size;
}

  //taken from https://ffmpeg.org/doxygen/trunk/avio_reading_8c-example.html#a18
  struct avio_context_t {

    typedef AVIOContext contained_type;
    
    std::shared_ptr<contained_type> ptr_;
    
    // std::vector<uint8_t,boost::alignment::aligned_allocator<char, 32> > buffer_;
    uint8_t* buffer_;
    static const uint32_t buffer_size_ = 4096;//given by ffmpeg examples

    avio_buffer_data data_2_read_;
    
    static void how_to_delete_me(contained_type* _ctx){
      if(_ctx){
	//av_freep(&avio_ctx->buffer);
	av_freep(&_ctx);
      }
    };

    avio_context_t(const avio_buffer_data& _read_data):
      ptr_(nullptr),
      buffer_(new uint8_t[buffer_size_]),
      data_2_read_(_read_data)
    {

      
      ptr_ = std::shared_ptr<contained_type>(avio_alloc_context(&buffer_[0], buffer_size_,
								0, &data_2_read_, &read_packet, NULL, NULL),how_to_delete_me);

    }

    contained_type* get(){
      return ptr_.get();
    }

    contained_type const* get() const {
      return ptr_.get();
    }
    
  };
    
};
#endif /* _HEVC_SCHEME_UTILS_H_ */
