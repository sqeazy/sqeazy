#ifndef _TIFF_UTILS_H_
#define _TIFF_UTILS_H_
#include <iostream>
#include <climits>
#include <set>
#include <algorithm>
#include <numeric>


#include "tiffio.h"
//#include "tiff.h"


#include "boost/filesystem.hpp"
#include "image_stack.hpp"

//TODO: setup namespace trick for identifying dimensions

namespace sqeazy {

  unsigned get_num_tiff_dirs(TIFF* _tiff_handle) {

    unsigned dircount = 0;
    if (_tiff_handle) {
      TIFFSetDirectory(_tiff_handle,tdir_t(0));
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

  /**
     \brief extract dimensions of image stack from tiff_handle
     
     \param[in] _tiff_handle TIFF pointer to open Tiff file (not error checking imposed)
     \param[in] _tiff_dirs opened tiff directories

     \return shape vector complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
     \retval 
     
  */
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

    value[2] = *(std::max_element(widths.begin(), widths.end()));
    value[1] = *(std::max_element(heights.begin(), heights.end()));
    value[0] = size_z;

    return value;
  }

  /**
     \brief extract pixels from tiff handle
     
     \param[in] _tiff_handle TIFF pointer to open Tiff file (not error checking imposed, needed to check encoding)
     \param[in] _tiff_dirs TIFF directories in open file
     \param[in] _container containter to put the pixels in
     
     \return shape vector complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
     \retval 
     
  */
  template <typename ValueT>
  std::vector<unsigned> extract_tiff_to_vector(TIFF* _tiff_handle,
					       const std::vector<tdir_t>& _tiff_dirs ,
					       std::vector<ValueT>& _container) {

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
    unsigned long frame_offset = extents[2]*extents[1];
    unsigned long total = std::accumulate(extents.begin(), extents.end(),1,std::multiplies<size_t>());
    _container.clear();
    _container.resize(total);
    unsigned long index = 0;

    for(unsigned frame = 0; frame<extents[0]; ++frame)
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

  /**
     \brief extract pixels from tiff handle
     
     \param[in] _tiff_handle TIFF pointer to open Tiff file (not error checking imposed, needed to check encoding)
     \param[in] _tiff_dirs TIFF directories in open file
     \param[in] _container stack complying to boost::multi_array concept
          
     \return 
     \retval 
     
  */
  template <typename stack_type>
  void extract_tiff_to_image_stack(TIFF* _tiff_handle,
				   const std::vector<tdir_t>& _tiff_dirs ,
				   stack_type& _container){
    
    std::vector<unsigned> extents = extract_max_extents<unsigned>(_tiff_handle, _tiff_dirs );

    
    unsigned w,h;
    unsigned frame_offset = extents[2]*extents[1];
    unsigned total = frame_offset*extents[0];
    std::vector<float> local_pixels;
    local_pixels.clear();
    local_pixels.resize(total);

    for(unsigned frame = 0;frame<extents[0];++frame)
      {
  	TIFFSetDirectory(_tiff_handle,_tiff_dirs[frame]);
  	TIFFGetField(_tiff_handle, TIFFTAG_IMAGEWIDTH, &w);
  	TIFFGetField(_tiff_handle, TIFFTAG_IMAGELENGTH, &h);
  	for (unsigned y=0;y<h;++y) {
  	  TIFFReadScanline(_tiff_handle,&local_pixels[frame*frame_offset+y*w], y);
  	}
      }

    _container.resize(extents);

    //c_storage_order is the default for boost multi_array
    //so we need to convert x,y,z (tiff through extract_max_extents) to z,y,z (stack_type)
    int tiff_order[3] = {0,1,2};
    bool ascending[3] = {true, true, true};
    // sqeazy::storage tiff_storage(tiff_order,ascending);
    typedef typename stack_type::storage_order_type storage_order_t;
    storage_order_t tiff_storage(tiff_order,ascending);
    stack_type  local_stack    (&local_pixels[0],    extents,    tiff_storage);

    //uses boost::multi_array conversion to do map all loaded pixels to the storage order in _container
    _container = local_stack;
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


  /**
     \brief write image stack (assumed to be 3D)
     
     \param[in] _stack in c_storage_order
     \param[in] _dims complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
     \param[in] _dest filename to write to

     \return 
     \retval 
     
  */
  template <typename im_vec_type, typename ext_type>
  void write_tiff_from_array(const im_vec_type* _stack,
			      const ext_type& _dims,
			      const std::string& _dest) {


    TIFF *output_image = TIFFOpen(_dest.c_str(), "w");


    if(!output_image) {
      std::cerr << "Unable to open "<< _dest<<"\n";
      return;
    }
    else
      std::cout << "Writing to "<< _dest<<"\n";

    unsigned w = _dims[_dims.size()-1];
    unsigned h = _dims[_dims.size()-2];
    unsigned z = _dims.size() > 2 ? _dims[0] : 1;
    
    const unsigned long long frame_size = w*h;
    unsigned long long index = 0;
    for(unsigned frame = 0; frame<z; ++frame) {
      TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH,		w);
      TIFFSetField(output_image, TIFFTAG_IMAGELENGTH,		h);
      TIFFSetField(output_image, TIFFTAG_PAGENUMBER,		frame, z);
      TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE,		sizeof(im_vec_type)*CHAR_BIT);
      TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL,	1);
      TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, 		PLANARCONFIG_CONTIG);
      TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC,  		PHOTOMETRIC_MINISBLACK);
      TIFFSetField(output_image, TIFFTAG_COMPRESSION,  		COMPRESSION_NONE);
      TIFFSetField(output_image, TIFFTAG_SAMPLEFORMAT, 		tiff_type_equivalent<im_vec_type>::value);
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


    template <typename im_vec_type, typename ext_type>
  void write_tiff_from_vector(const im_vec_type& _stack,
			      const ext_type& _dims,
			      const std::string& _dest) {
      return write_tiff_from_array(&_stack[0], _dims, _dest);
    }


  /**
   \brief write stack to tiff, we assert that c_storage_order is setup within _stack and the stack shape is set accordingly
   for c_storage_order that would imply shape = {z-dim,y-dim,x-dim}

   \param[in] 
   
   \return 
   \retval 
   
  */  
  template <typename stack_type>
  void write_image_stack(const stack_type& _stack, const std::string& _dest){

    typedef typename stack_type::element value_type;
    typedef typename stack_type::template const_array_view<1>::type		stack_line_t;
    typedef typename stack_type::index_range		range_t;

    TIFF *output_image = TIFFOpen(_dest.c_str(), "w");
    if(!output_image){
      std::cerr << "Unable to open "<< _dest<<"\n";
      return;
    }
    else
      std::cout << "Writing "<< _dest<<"\n";

    if(!(_stack.storage_order() == boost::c_storage_order())){
      std::cerr << "inconsistent ordering found! Nothing saved.\n";
      return;
    }
    
    unsigned w = _stack.shape()[2];
    unsigned h = _stack.shape()[1];
    unsigned z = _stack.shape()[0];
    
    for(unsigned frame = 0;frame<z;++frame){
      TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH,		w);
      TIFFSetField(output_image, TIFFTAG_IMAGELENGTH,		h);
      TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE,		sizeof(value_type)*CHAR_BIT);
      TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL,	1);
      TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, 		PLANARCONFIG_CONTIG);
      TIFFSetField(output_image, TIFFTAG_ROWSPERSTRIP, 		TIFFDefaultStripSize(output_image, 0));
      TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC,  		PHOTOMETRIC_MINISBLACK);
      TIFFSetField(output_image, TIFFTAG_COMPRESSION,  		COMPRESSION_NONE);
      TIFFSetField(output_image, TIFFTAG_SUBFILETYPE,		FILETYPE_PAGE);
      TIFFSetField(output_image, TIFFTAG_SAMPLEFORMAT, 		tiff_type_equivalent<value_type>::value);
      TIFFSetField(output_image, TIFFTAG_PAGENUMBER,		frame, z); 
      TIFFSetField(output_image, TIFFTAG_ORIENTATION,		(int)ORIENTATION_TOPLEFT);
      
      std::vector<value_type> temp_row(w);
      
      for (unsigned y=0;y<h;++y) {

	stack_line_t temp = _stack[ boost::indices[frame][y][range_t(0,w)] ];
	std::copy(temp.begin(), temp.end(), temp_row.begin());
	TIFFWriteScanline(output_image,&temp_row[0],y,0);

      }

      TIFFWriteDirectory(output_image);
    }
    TIFFClose(output_image);
  }













  
  struct tiff_facet {
    
    boost::filesystem::path location_;
    TIFF* handle_;
    unsigned long n_pixels_;
    std::vector<unsigned> shape_;
    std::vector<char> buffer_;
    
    tiff_facet(const std::string& _path = "", bool _all_errors = false):
      location_(_path),
      handle_(0),
      n_pixels_(0),
      shape_(),
      buffer_()
    {
      
      
      if(!_path.empty())
	load(_path, _all_errors);
    }
    
    
    

    template <typename T, typename U>
    tiff_facet(T* _begin, T* _end, const std::vector<U>& _shape):
      location_(""),
      handle_(0),
      n_pixels_(_end - _begin),
      shape_(_shape),
      buffer_()
    {
      
      unsigned long long bytes = n_pixels_*sizeof(T);
      buffer_.resize(bytes);
      T* dest_begin = reinterpret_cast<T*>(&buffer_[0]);
      std::copy(_begin, _end, dest_begin);
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
      return (this->n_pixels_)*bits_per_sample()/CHAR_BIT;
    }

    unsigned long size() const {
      return (this->n_pixels_);
    }

    /**
       \brief insert width, height, n_tiff_dirs into _dims vector
       
       \param[out] _dims 
       
       \return 
       \retval 
       
    */
    template <typename T>
    void dimensions(std::vector<T>& _shape) const {
      _shape.clear();

      if(!empty()){

	

	T n_frames = get_num_tiff_dirs(handle_);
	T width = get_tiff_width(handle_);
	T height = get_tiff_height(handle_);

	if(n_frames){
	  _shape.resize(3);
	  _shape[0] = n_frames;
	} else {
	  _shape.resize(2);
	}
	
	if(width){
	  _shape[_shape.size()-1] = width;
	} else {
	  return;
	}

	if(height)
	  _shape[_shape.size()-2] = height;
	else
	  return;
	
	
      }
    }

    

    void load(const std::string& _path = "", bool _all_errors = false){

      if(!_all_errors)
	TIFFSetWarningHandler(NULL);
      
      if(!empty()){
	TIFFClose(handle_);
	handle_ = 0;
	shape_.clear();
	n_pixels_ = 0;
	buffer_.clear();
      }

      location_ = _path;

      if(boost::filesystem::exists(location_) && boost::filesystem::is_regular_file(location_)){
	handle_ = TIFFOpen(_path.c_str(), "r");
	dimensions(shape_);
	rewind();
	n_pixels_ = std::accumulate(shape_.begin(), shape_.end(), 1, std::multiplies<unsigned long>());
	load_to_buffer();
      } else {
	if(!_path.empty())
	  std::cerr << "[sqeazy_bench::tiff_facet] \t unable to open " << _path << " does not exist or is not a file\n";
      }
    }

    void write(const std::string& _path = "unknown", unsigned _bits_per_sample=8){
      
      if(_path.empty()){
	std::cerr << "[sqeazy_bench::tiff_facet::write] \t unable to open " << _path << " does not exist\n";
	return;
      }
      
      switch(_bits_per_sample){
      case 8:
	write_tiff_from_array(reinterpret_cast<const unsigned char*>(&buffer_[0]), 
			      shape_,
			      _path);
	break;
      case 16:
	write_tiff_from_array(reinterpret_cast<const unsigned short*>(&buffer_[0]), 
			      shape_,
			      _path);
	break;
      default:
	std::cerr << "[sqeazy_bench::tiff_facet::write] \t unknown number bits per sample " << _bits_per_sample << ", nothing to write to " <<  _path << "\n";
	
      }

      return;
    }

    TIFF* tiff_handle(){
      return handle_;
    }

    
    void load_to_buffer(){
      
      buffer_.resize(size_in_byte());

      unsigned w,h;
      unsigned long frame_offset = shape_[2]*shape_[1];

      unsigned long index = 0;

      for(unsigned frame = 0; frame<shape_[0]; ++frame)
	{

	  TIFFGetField(handle_, TIFFTAG_IMAGEWIDTH, &w);
	  TIFFGetField(handle_, TIFFTAG_IMAGELENGTH, &h);
	  for (unsigned y=0; y<h; ++y) {
	    index = frame*frame_offset+y*w;
	    TIFFReadScanline(handle_,&buffer_[index*bits_per_sample()/CHAR_BIT], y);
	  }
	  TIFFReadDirectory(handle_);
	}
    }

    char const * data() const {
      return &buffer_[0];
    }

  };

}
#endif /* _TIFF_UTILS_H_ */
