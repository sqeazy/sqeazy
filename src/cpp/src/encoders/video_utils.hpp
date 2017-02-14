#ifndef _VIDEO_UTILS_H_
#define _VIDEO_UTILS_H_

#include <iostream>
#include <memory>
#include <cstdint>
#include <iterator>
#include <vector>

#include "boost/align/aligned_allocator.hpp"

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
  #include <libavformat/avformat.h>
}

namespace sqeazy {

  void init(AVPacket* _pkt){
    if(_pkt){
      av_init_packet(_pkt);
      _pkt->data = NULL;
      _pkt->size = 0;
    }
  }

  //see http://ffmpeg.org/doxygen/trunk/pixfmt_8h.html#a9a8e335cf3be472042bc9f0cf80cd4c5a107eadfcfe8c9fc751289eeb7744a6d3
  //for references on pixel format
  struct yuv420p;

  template <typename T> struct av_pixel_type {};
  template <> struct av_pixel_type<char> { static const AVPixelFormat value = AV_PIX_FMT_GRAY8;};
  template <> struct av_pixel_type<std::uint8_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY8;};
  template <> struct av_pixel_type<std::int8_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY8;};

  template <> struct av_pixel_type<std::uint16_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY16;};
  template <> struct av_pixel_type<std::int16_t> { static const AVPixelFormat value = AV_PIX_FMT_GRAY16;};

  template <> struct av_pixel_type<std::uint32_t> { static const AVPixelFormat value = AV_PIX_FMT_RGBA;};
  template <> struct av_pixel_type<std::int32_t> { static const AVPixelFormat value = AV_PIX_FMT_RGBA;};

  template <> struct av_pixel_type<yuv420p> { static const AVPixelFormat value = AV_PIX_FMT_YUV420P;};

  // template <> struct av_pixel_type<short> { static const AVPixelFormat value = AV_PIX_FMT_GRAY16;};
  // template <> struct av_pixel_type<unsigned shortte> { static const AVPixelFormat value = AV_PIX_FMT_GRAY16;};

  void av_free_packet(AVPacket* pkt){

    //FIXME: inserted due to deprecation error, needs to be replaced
    //       with something maintainable as this is hardcoded and might be changed upstream
    if (pkt) {
      // if (pkt->buf)
      //    av_buffer_unref(&pkt->buf);
      av_packet_unref(pkt);
      pkt->data            = NULL;
      pkt->size            = 0;

      // FIXME: I am unclear if this call is actually needed! (most examples live with just av_buffer_unref)
      // av_packet_free_side_data(pkt);
    }

  }

  struct av_codec_t {

    typedef AVCodec contained_type;

    contained_type* ptr_;

    av_codec_t(const AVCodecID& _id, bool is_encoder = true):
      ptr_(nullptr){

      if(_id){
        if(is_encoder)
          ptr_ = avcodec_find_encoder(_id);
        else
          ptr_ = avcodec_find_decoder(_id);
      }
      if (!ptr_) {
        std::cerr << "[av_codec_t] Could not allocate video codec context from AVCodecID& = "<< _id <<"\n";
      }

    }

    av_codec_t(AVCodecID* _id, bool is_encoder = true):
      ptr_(nullptr){

      if(_id){
        if(is_encoder)
          ptr_ = avcodec_find_encoder(*_id);
        else
          ptr_ = avcodec_find_decoder(*_id);
      }
      if (!ptr_) {
        std::cerr << "[av_codec_t] Could not allocate video codec context from AVCodecID*\n";
      }
    }

    //valgrind: no additional free required
    // ~av_codec_type(){
    //   if(ptr_)
    //  delete ptr_;
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
        av_free(ptr_);
        // avcodec_free_context(&ptr_);
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

      int allocation_status = 0;
      if(_width && _height)
        allocation_status = av_image_alloc(ptr_->data,
                                           ptr_->linesize,
                                           ptr_->width,
                                           ptr_->height,
                                           (AVPixelFormat)ptr_->format,
                                           align_by//arch dependent
          );

      if(allocation_status<0){
        std::cerr << "unable to create frame "
                  << "width: " << _width << ", "
                  << "height: " << _height << ", "
                  << "format: " << _format << "\n";

      }
    }


    AVFrame* get(){
      return ptr_.get();
    }

    enum AVPixelFormat pixel_format() const {
      enum AVPixelFormat value = (AVPixelFormat)(ptr_.get()->format);
      return value;
    }

    AVFrame const* get() const {
      return ptr_.get();
    }

    std::size_t width() const {
      return ptr_.get()->width;
    }

    std::size_t w() const {
      return ptr_.get()->width;
    }

    std::size_t height() const {
      return ptr_.get()->height;
    }

    std::size_t h() const {
      return ptr_.get()->height;
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
        // if(_ctx->pb)
        //   avio_closep(&_ctx->pb);
        //FIXME?
        if(_ctx->iformat)
          avformat_close_input(&_ctx);

        avformat_free_context(_ctx);
      }
    };

    av_format_context_t():
      ptr_(nullptr){

      ptr_ = std::shared_ptr<contained_type>(avformat_alloc_context(),
                                             how_to_delete_me);

    }

    // ~av_format_context_t(){
    //   if(ptr_ && ptr_.get())
    //  delete ptr_;
    // }


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
    const char *ptr;
    size_t size; ///< size left in the buffer
  };

  static int read_packet(void *opaque, std::uint8_t *buf, int buf_size)
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

    // std::vector<char,boost::alignment::aligned_allocator<char, 32> > buffer_;
    std::uint8_t* buffer_;
    static const uint32_t buffer_size_ = 4096;//given by ffmpeg example avio_reading.c

    avio_buffer_data data_2_read_;

    static void how_to_delete_me(contained_type* _ctx){
      if(_ctx){
        if(_ctx->buffer)
          av_freep(&_ctx->buffer);
        av_freep(&_ctx);
      }
    };

    avio_context_t(const avio_buffer_data& _read_data):
      ptr_(nullptr),
      buffer_((std::uint8_t*)av_malloc(buffer_size_)),
      data_2_read_(_read_data)
      {


        ptr_ = std::shared_ptr<contained_type>(avio_alloc_context(buffer_,
                                                                  buffer_size_,
                                                                  0,
                                                                  &data_2_read_,
                                                                  &read_packet,
                                                                  NULL,
                                                                  NULL),
                                               how_to_delete_me);

      }

    contained_type* get(){
      return ptr_.get();
    }

    contained_type const* get() const {
      return ptr_.get();
    }

  };




  template <typename itr_type>
  std::size_t y_to_range(const sqeazy::av_frame_t& _frame,
                         itr_type _begin, itr_type _end ){

    std::size_t frame_size = _frame.get()->width*_frame.get()->height;
    const std::size_t itr_size = _end - _begin;

    std::size_t bytes_copied = 0;

    //static_assert(sizeof(*_begin)==sizeof(_frame.get()->data[0][0]), "unable to copy y_to_range due to mismtaching type");

    if(itr_size != frame_size)
      return bytes_copied;


    const std::size_t height = _frame.get()->height;
    for(uint32_t y=0;y<height;++y){
      auto begin = _frame.get()->data[0] + (y*_frame.get()->linesize[0]);
      auto end = begin + _frame.get()->width;
      auto dst_begin = _begin+(y*_frame.get()->width);
      std::copy(begin, end,dst_begin);
      bytes_copied += (end-begin)*sizeof(*_begin);
    }

    return bytes_copied;
  }

  template <typename raw_type>
  std::size_t y_to_vector(const sqeazy::av_frame_t& _frame,
                          std::vector<raw_type>& _vector ){

    return y_to_range(_frame,_vector.begin(), _vector.end());
  }


  template <typename itr_type>
  std::size_t range_to_y(itr_type _begin, itr_type _end,
                         sqeazy::av_frame_t& _frame
    ){

    std::size_t value =0;
    std::size_t frame_size = _frame.get()->width*_frame.get()->height;
    std::size_t itr_size = _end - _begin;

    if(itr_size != frame_size)
      return value;

    std::size_t bytes_copied = 0;

    const std::size_t height = _frame.get()->height;

    //static_assert(sizeof(*_begin)==sizeof(_frame.get()->data[0][0]), "unable to copy range_to_y due to mismtaching type");

    for(uint32_t y=0;y<height;++y){
      auto dst_begin = _frame.get()->data[0] + (y*_frame.get()->linesize[0]);

      auto begin = _begin+(y*_frame.get()->width);
      auto end = begin + _frame.get()->width;

      std::copy(begin, end,dst_begin);
      bytes_copied += (end-begin)*sizeof(*_begin);
    }

    value = bytes_copied;
    return value;
  }



  template <typename raw_type>
  std::size_t vector_to_y(const std::vector<raw_type>& _vector,
                          sqeazy::av_frame_t& _frame
    ){

    return range_to_y(_vector.begin(),_vector.end(),_frame);

  }

  /**
   *  \brief encoding a 16-bit input buffer to yuv420 by extracting the minimum of 4 input values,
   *  store the minimum into 2 bytes of the first color channel and substract the minumum for the 4 values mentioned
   *
   *  example:
   *		input   = {4048, 4047, 4045, 4100}
   *		min     = 4045
   *		encoded[0]	= {3, 2, 0, 55}
   *		encoded[1]	= {15, 205} //4045 = 0xfcd = {0xf, 0xcd}
   *
   *  \param _frame read-only frame to extract the 16-bit data from
   *  \param _begin input iterator for 16-bit data pointing to the beginning of the sequence to fill
   *  \param _end input iterator for 16-bit data pointing to 1 plus the last element of the sequence to fill
   *  \return std::size_t number of bytes decoded
   */
  template <typename itr_type>
  std::size_t range_to_yuv420(itr_type _begin, itr_type _end,
                              sqeazy::av_frame_t& _frame
    ){

    typedef typename std::iterator_traits<itr_type>::value_type value_t;
    typedef typename std::remove_reference<decltype(_frame.get()->data[0][0])>::type frame_y_t;
    typedef typename std::remove_reference<decltype(_frame.get()->data[0][1])>::type frame_c_t;

    // static_assert(sizeof(value_t)==2,"range_to_yuv420 only works on 16bit data");
    if(sizeof(value_t)!=2){
      std::cerr << "range_to_yuv420 only works on 16bit data\n";
      return 0;
    }

    std::size_t frame_size = _frame.get()->width*_frame.get()->height;
    std::size_t itr_size = _end - _begin;
    std::size_t bytes_copied = 0;

    if(itr_size != frame_size)
      return bytes_copied;

    const std::size_t height = _frame.get()->height;
    const std::size_t width = _frame.get()->width;
    value_t min_el = 0;

    // TODO: what if height is odd?
    for(std::uint32_t y=0;y<height;y+=2){

      auto begin = _begin+(y*_frame.get()->width);

      value_t min_el_top = 0;
      value_t min_el_bot = 0;

      auto color_begin = &_frame.get()->data[1][(y/2)*_frame.get()->linesize[1]];

      for(std::uint32_t x=0;x<width;x+=4){

        const auto offset =((width - x > 3) ? 4 : width - x);
        min_el_top = *std::min_element(begin+x,
                                       begin+x+offset);

        if(y+1 < height)
          min_el_bot = *std::min_element(begin+x+width,
                                         begin+x+width+offset);
        else
          min_el_bot = min_el_top;

        min_el = std::min(min_el_top,min_el_bot);

        frame_c_t hihalf = (min_el >> 8) & 0xff;
        frame_c_t lohalf = (min_el) & 0xff;
        *(color_begin ) = hihalf;
        *(color_begin  + 1) = lohalf;
        color_begin+=2;
      }
    }

    std::vector<frame_y_t> single_line(width,0);

    for(std::uint32_t y=0;y<height;++y){

      auto begin = _begin+(y*_frame.get()->width);
      auto end = begin + _frame.get()->width;
      auto min_el_itr = &_frame.get()->data[1][(y/2)*_frame.get()->linesize[1]];

      for(std::uint32_t x=0;x<width;++x){

        min_el = value_t((*(min_el_itr)) << 8) | value_t(*(min_el_itr+1));
        single_line[x] = *(begin+x) - min_el;
        if(x % 4 == 3) min_el_itr += 2;
      }

      auto dst_begin = _frame.get()->data[0] + (y*_frame.get()->linesize[0]);
      std::copy(single_line.begin(), single_line.end(),
                dst_begin);


      bytes_copied += (end-begin)*sizeof(*_begin);
    }

    return bytes_copied;
  }


  template <typename value_type>
  std::size_t vector_to_yuv420(const std::vector<value_type>& _container,
                               sqeazy::av_frame_t& _frame
    ){
    return range_to_yuv420(_container.cbegin(), _container.cend(),_frame);
  }

  /**
   *  \brief decoding a 16-bit input buffer from yuv420 by extracting the minimum of 4 values from 2 bytes of the first color channel
   *
   *  \param _frame read-only frame to extract the 16-bit data from
   *  \param _begin input iterator for 16-bit data pointing to the beginning of the sequence to fill
   *  \param _end input iterator for 16-bit data pointing to 1 plus the last element of the sequence to fill
   *  \return std::size_t number of bytes decoded
   */
  template <typename itr_type>
  std::size_t yuv420_to_range(const sqeazy::av_frame_t& _frame,
                              itr_type _begin, itr_type _end
    ){

    typedef typename std::remove_reference<typename std::iterator_traits<itr_type>::value_type>::type value_t;

    if(sizeof(value_t)!=2){
      std::cerr << "yuv420_to_range only works on 16bit data\n";
      return 0;
    }

    std::size_t frame_size = _frame.get()->width*_frame.get()->height;
    const std::size_t itr_size = _end - _begin;

    std::size_t bytes_decoded = 0;

    if(itr_size != frame_size)
      return bytes_decoded;


    const std::size_t height = _frame.get()->height;
    const std::size_t width = _frame.get()->width;

    value_t to_add = 0;

    for(std::uint32_t y=0;y<height;++y){

      auto begin = _frame.get()->data[0] + (y*_frame.get()->linesize[0]);
      auto end = begin + _frame.get()->width;
      auto dst_begin = _begin+(y*_frame.get()->width);
      auto color_begin = &_frame.get()->data[1][(y/2)*_frame.get()->linesize[1]];

      for(std::uint32_t x=0;x<width;++x){
        to_add = value_t((*color_begin) << 8) | value_t(*(color_begin+1));
        *(dst_begin + x) = (*(begin + x) ) + to_add;

        if(x % 4 == 3) color_begin += 2;
      }

      bytes_decoded += (end-begin)*sizeof(*_begin);

    }

    return bytes_decoded;
  }


};

#endif /* _VIDEO_UTILS_H_ */
