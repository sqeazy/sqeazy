#ifndef _TIFF_UTILS_H_
#define _TIFF_UTILS_H_
#include <iostream>
#include <climits>
#include <set>
#include <algorithm>
#include "tiffio.h"
#include "boost/filesystem.hpp"


namespace sqeazy_bench {

  unsigned get_num_tiff_dirs(TIFF* _tiff_handle) {

    unsigned dircount = 0;
    if (_tiff_handle) {
      dircount = 1;

      while (TIFFReadDirectory(_tiff_handle)) {
	dircount++;
      }
      TIFFSetDirectory(_tiff_handle,tdir_t(0));
    }

    return dircount;
  }

  int get_tiff_bits_per_sample(TIFF* _tiff_handle){
    int value = 0;
    
    if(_tiff_handle)
      TIFFGetField(_tiff_handle, TIFFTAG_BITSPERSAMPLE, &value);

    return value;
  }

  int get_tiff_width(TIFF* _tiff_handle){
    int value = 0;
    
    if(_tiff_handle)
      TIFFGetField(_tiff_handle, TIFFTAG_IMAGEWIDTH, &value);

    return value;
  }


  int get_tiff_height(TIFF* _tiff_handle){
    int value = 0;
    
    if(_tiff_handle)
      TIFFGetField(_tiff_handle, TIFFTAG_IMAGELENGTH, &value);

    return value;
  }

  void get_tiff_dirs(TIFF* _tiff_handle, std::vector< tdir_t >& _value) {

    //rewind incase the incoming handle_ is not at the beginning of the file
    if(TIFFCurrentDirectory(_tiff_handle)!=0)
      TIFFSetDirectory(_tiff_handle,tdir_t(0));

    if (_tiff_handle) {
      _value.reserve(512);
      _value.push_back(TIFFCurrentDirectory(_tiff_handle));


      while (TIFFReadDirectory(_tiff_handle)) {
	_value.push_back(TIFFCurrentDirectory(_tiff_handle));


      }

    }

  }

  template <typename ExtentT>
  std::vector<ExtentT> extract_max_extents(TIFF* _tiff_handle, const std::vector< tdir_t >& _tiff_dirs ) {

    std::vector<ExtentT> value(3);
    std::set<unsigned> widths;
    std::set<unsigned> heights;
    unsigned w,h;
    unsigned size_z = _tiff_dirs.size();
    for(unsigned i = 0; i<size_z; ++i)
      {
        w = h = 0;
        TIFFSetDirectory(_tiff_handle,_tiff_dirs[i]);
        TIFFGetField(_tiff_handle, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(_tiff_handle, TIFFTAG_IMAGELENGTH, &h);
        widths.insert(w);
        heights.insert(h);
      }

    value[0] = *(std::max_element(widths.begin(), widths.end()));
    value[1] = *(std::max_element(heights.begin(), heights.end()));
    value[2] = size_z;

    return value;
  }

  template <typename ValueT>
  std::vector<unsigned> extract_tiff_to_vector(TIFF* _tiff_handle, const std::vector<tdir_t>& _tiff_dirs ,std::vector<ValueT>& _container) {

    int bits_per_sample = 0;
    TIFFGetField(_tiff_handle, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if(bits_per_sample!=(sizeof(ValueT)*CHAR_BIT)){
      std::string fname(TIFFFileName(_tiff_handle));
      std::ostringstream msg; 
      msg << "[SQY] ERROR: expected " << sizeof(ValueT)*CHAR_BIT << ", but received " << bits_per_sample << " from " << fname << "\n";
      throw std::runtime_error(msg.str());
    }

    std::vector<unsigned> extents = extract_max_extents<unsigned>(_tiff_handle, _tiff_dirs );

    unsigned w,h;
    unsigned long frame_offset = extents[0]*extents[1];
    unsigned long total = frame_offset*extents[2];
    _container.clear();
    _container.resize(total);
    unsigned long index = 0;

    for(unsigned frame = 0; frame<extents[2]; ++frame)
      {
        TIFFSetDirectory(_tiff_handle,_tiff_dirs[frame]);
        TIFFGetField(_tiff_handle, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(_tiff_handle, TIFFTAG_IMAGELENGTH, &h);
        for (unsigned y=0; y<h; ++y) {
	  index = frame*frame_offset+y*w;
	  TIFFReadScanline(_tiff_handle,&_container[index], y);
        }
      }

    return extents;
  }


  template <typename T>
  struct tiff_type_equivalent {
    enum { value = 0 };
  };

  template <> struct tiff_type_equivalent<int> {
    enum { value = SAMPLEFORMAT_INT } ;
  };
  template <> struct tiff_type_equivalent<unsigned> {
    enum { value = SAMPLEFORMAT_UINT };
  };
  template <> struct tiff_type_equivalent<float> {
    enum { value = SAMPLEFORMAT_IEEEFP };
  };
  template <> struct tiff_type_equivalent<short> {
    enum { value = SAMPLEFORMAT_INT };
  };
  template <> struct tiff_type_equivalent<unsigned short> {
    enum { value = SAMPLEFORMAT_UINT };
  };
  template <> struct tiff_type_equivalent<char> {
    enum { value = SAMPLEFORMAT_INT };
  };
  template <> struct tiff_type_equivalent<unsigned char> {
    enum { value = SAMPLEFORMAT_UINT };
  };
  template <> struct tiff_type_equivalent<long long> {
    enum { value = SAMPLEFORMAT_INT };
  };
  template <> struct tiff_type_equivalent<unsigned long long> {
    enum { value = SAMPLEFORMAT_UINT };
  };
  template <> struct tiff_type_equivalent<double> {
    enum { value = SAMPLEFORMAT_IEEEFP };
  };



  template <typename im_vec_type, typename ext_type>
  void write_tiff_from_vector(const im_vec_type& _stack,
			      const ext_type& _dims,
			      const std::string& _dest) {

    typedef typename im_vec_type::value_type value_type;
    //typedef image_stack::const_array_view<1>::type		stack_line;

    TIFF *output_image = TIFFOpen(_dest.c_str(), "w");


    if(!output_image) {
      std::cerr << "Unable to open "<< _dest<<"\n";
      return;
    }
    else
      std::cout << "Writing to "<< _dest<<"\n";

    unsigned w = _dims[0];
    unsigned h = _dims[1];
    unsigned z = _dims[2];
    const unsigned long long frame_size = w*h;
    unsigned long long index = 0;
    for(unsigned frame = 0; frame<z; ++frame) {
      TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH,		w);
      TIFFSetField(output_image, TIFFTAG_IMAGELENGTH,		h);
      TIFFSetField(output_image, TIFFTAG_PAGENUMBER,		frame, z);
      TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE,		sizeof(value_type)*CHAR_BIT);
      TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL,	1);
      TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, 		PLANARCONFIG_CONTIG);
      TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC,  		PHOTOMETRIC_MINISBLACK);
      TIFFSetField(output_image, TIFFTAG_COMPRESSION,  		COMPRESSION_NONE);
      TIFFSetField(output_image, TIFFTAG_SAMPLEFORMAT, 		tiff_type_equivalent<value_type>::value);
      TIFFSetField(output_image, TIFFTAG_SUBFILETYPE,		FILETYPE_PAGE);
      TIFFSetField(output_image, TIFFTAG_ROWSPERSTRIP, 		TIFFDefaultStripSize(output_image, 0));
	
        
      for (unsigned y=0; y<h; ++y) {
	index = frame*frame_size + y*w;
	TIFFWriteScanline(output_image,(void*)&_stack[index],y,0);

      }

      TIFFWriteDirectory(output_image);
    }
    TIFFClose(output_image);
  }


  struct tiff_facet {
    
    boost::filesystem::path location_;
    TIFF* handle_;
    unsigned long pixels_in_byte_;
    std::vector<unsigned> dims_;
    
    tiff_facet(const std::string& _path = ""):
      location_(_path),
      handle_(0),
      pixels_in_byte_(0),
      dims_()
    {
      if(boost::filesystem::exists(location_) && boost::filesystem::is_regular_file(location_)){
	handle_ = TIFFOpen(_path.c_str(), "r");
	dimensions(dims_);
	rewind();
	pixels_in_byte_ = std::accumulate(dims_.begin(), dims_.end(), 1, std::multiplies<unsigned long>());
	
      } else {
	if(_path.size())
	  std::cerr << "[sqeazy_bench::tiff_facet] \t unable to open " << _path << " does not exist or is not a file\n";
      }
    }

    bool empty() const {
      return !handle_;
    }

    ~tiff_facet(){
      
      if(handle_)
	TIFFClose(handle_);
      
    }

    void rewind(){
      TIFFSetDirectory(handle_,tdir_t(0));
    }

    int bits_per_sample() const {
      
      return empty() ? 0 : get_tiff_bits_per_sample(handle_);
      
    }
    
    unsigned long size_in_byte() const {
      return this->pixels_in_byte_;
    }

    template <typename T>
    void dimensions(std::vector<T>& _dims) const {
      _dims.clear();

      if(!empty()){
	_dims.reserve(3);
	T n_frames = get_num_tiff_dirs(handle_);
	T width = get_tiff_width(handle_);
	T height = get_tiff_height(handle_);
	if(width){
	  _dims.push_back(width);
	} else {
	  return;
	}

	if(height)
	  _dims.push_back(height);
	else
	  return;
	
	if(n_frames)
	  _dims.push_back(n_frames);
	
      }
    }

    void reload(const std::string& _path = ""){

      if(!empty()){
	TIFFClose(handle_);
	handle_ = 0;
	dims_.clear();
	pixels_in_byte_ = 0;
      }

      location_ = _path;

      if(boost::filesystem::exists(location_) && boost::filesystem::is_regular_file(location_)){
	handle_ = TIFFOpen(_path.c_str(), "r");
	dimensions(dims_);
	rewind();
	pixels_in_byte_ = std::accumulate(dims_.begin(), dims_.end(), 1, std::multiplies<unsigned long>());
      } else {
	if(!_path.empty())
	  std::cerr << "[sqeazy_bench::tiff_facet] \t unable to open " << _path << " does not exist or is not a file\n";
      }
    }


    TIFF* tiff_handle(){
      return handle_;
    }

  };

}
#endif /* _TIFF_UTILS_H_ */
