#ifndef _YUV_UTILS_HPP_
#define _YUV_UTILS_HPP_
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

#include "regex_helpers.hpp"
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

namespace sqeazy {


  /**
   \brief write stack to plain yuv, we assert that c_storage_order is setup within _stack and the stack shape is set accordingly
   for c_storage_order that would imply shape = {z-dim,y-dim,x-dim} for a discrete euclidean 3D stack

   \param[in] _stack payload
   \param[in] _dest_stem file name to write to

   \return 
   \retval given file name as string
   
  */  
  template <typename stack_t>
  std::string write_stack_as_yuv(const stack_t& _stack,
			  const std::string& _dest_stem,
			  bool _verbose = false){

    typedef typename stack_t::element value_type;

    static_assert(sizeof(value_type)<2,"[sqeazy::write_stack_as_yuv] non-compliant pixel type received");


    const uint32_t frame_size = _stack.shape()[1]*_stack.shape()[2];
    std::vector<char> all_chroma_values(frame_size/2,0);

    std::string dest;
    if(_dest_stem.empty())
      dest = "sqeazy_stack";

    size_t pos = _dest_stem.rfind(".yuv");
    if(pos!=std::string::npos){
      dest.resize(pos);
      std::copy(_dest_stem.begin(), _dest_stem.begin() + pos,dest.begin());
    } else {
      dest.resize(_dest_stem.size());
      std::copy(_dest_stem.begin(), _dest_stem.end(),dest.begin());
    }
    
    std::stringstream dest_file;
    dest_file << dest
	      << "_w" << _stack.shape()[2]
	      << "_h" << _stack.shape()[1]
	      << "_f" << _stack.shape()[0]
	      << "_yuv420p.yuv";
      
    std::ofstream ofile(dest_file.str(),std::ios::out | std::ios::binary | std::ios::trunc);
    float mbytes_written = 0;
    
    for(std::uint32_t z = 0;z<_stack.shape()[0];++z){
      const char* fptr = reinterpret_cast<const char*>(&_stack.data()[z*frame_size]);
      //luminance
      ofile.write(fptr,frame_size);
      //both chroma
      ofile.write(&all_chroma_values[0],all_chroma_values.size());
      mbytes_written += frame_size + all_chroma_values.size();
      
    }
      
    ofile.close();
    
    const float mbytes_written_exp = _stack.num_elements()*1.5f/(1 << 20);
    mbytes_written /= (1 << 20);
    
    if(_verbose)
      std::cout << "[sqeazy::write_stack_as_yuv] wrote " << dest_file.str() << " ("<< mbytes_written << "/" << mbytes_written_exp << " MB)\n";

    return dest_file.str();
  }


  struct yuv420formatter{
    
    static std::string y4m_code() {

      return "C420";
    }

    template <typename stack_t, typename stream_t>
    static void write_stack(const stack_t& _stack, stream_t& _stream) {

      typedef typename stack_t::element value_type;
      static_assert(sizeof(value_type)<2,"[sqeazy::yuv420formatter] unable to write non-8-bit data");

      const std::string frame_title("FRAME\n");
      const uint32_t frame_size = _stack.shape()[1]*_stack.shape()[2];
      const std::vector<char> all_chroma_values(frame_size/2,0);
      
      for(std::uint32_t z = 0;z<_stack.shape()[0];++z){

	//header
	_stream.write(frame_title.c_str(),frame_title.size());

	const char* fptr = reinterpret_cast<const char*>(&_stack.data()[z*frame_size]);
	//luminance
	_stream.write(fptr,frame_size);
	//both chroma
	_stream.write(&all_chroma_values[0],all_chroma_values.size());
      }

	  
      return ;
    }

    template <typename stl_vec_t, typename stream_t, typename size_t>
    static void read_stack(stl_vec_t& _flat_stack, stream_t& _stream, std::vector<size_t>& _shape) {

      typedef typename stl_vec_t::value_type value_t;
      if(sizeof(value_t)!=1){
	std::cerr << "[sqeazy::yuv420formatter] unable to write non-8-bit data\n";
	return;
      }
      
      if(_shape.size()<2 || _shape.size()>3){
	std::cerr << "[sqeazy::yuv420formatter] unable to read " << _shape.size() << "-dim data (2D or 3D allowed)\n";
	return;
      }

      _stream.seekg(0,_stream.beg);
      size_t frame_size_bytes = _shape[_shape.size()-1]*_shape[_shape.size()-2]*sizeof(value_t);
      size_t frames_consumed = 0;
      //resizing upon assumption
      _flat_stack.clear();
      _flat_stack.resize(frame_size_bytes*(_shape[_shape.size()-1]+_shape[_shape.size()-2]));
    
      for (std::string line; std::getline(_stream, line); ) {
      
	if(line.find("FRAME")!=std::string::npos){
	  //seek ptr should be after 'FRAME\n' now
	  if(_flat_stack.size()<((frames_consumed+1)*frame_size_bytes))
	    _flat_stack.resize(_flat_stack.size()+frame_size_bytes);
	  _stream.read((char*)&_flat_stack[(frames_consumed++)*frame_size_bytes],frame_size_bytes);
	
	}
      
      }

      _flat_stack.resize(frames_consumed*frame_size_bytes);

      std::vector<size_t> final_shape(3);
      
      if(_shape.size()==2){
	std::copy(_shape.begin(),_shape.end(),final_shape.begin()+1);
	_shape.clear();
	_shape.resize(3);
      }
      else
	std::copy(_shape.begin(),_shape.end(),final_shape.begin());

      final_shape[0] = frames_consumed ;
      _shape = final_shape;
      return ;
    }
    
  };

  struct yuv444formatter{
    
    static std::string y4m_code() {

      return "C444";
    }

    
    template <typename stack_t, typename stream_t>
    static void write_stack_8(const stack_t& _stack, stream_t& _stream) {

      const std::string frame_title("FRAME\n");
      const uint32_t frame_size = _stack.shape()[1]*_stack.shape()[2];
      const std::vector<char> all_chroma_values(frame_size,0);
      
      for(std::uint32_t z = 0;z<_stack.shape()[0];++z){

	//header
	_stream.write(frame_title.c_str(),frame_title.size());

	const char* fptr = reinterpret_cast<const char*>(&_stack.data()[z*frame_size]);
	//luminance
	_stream.write(fptr,frame_size);

	//both chroma
	_stream.write(&all_chroma_values[0],all_chroma_values.size());
	_stream.write(&all_chroma_values[0],all_chroma_values.size());
	
      }

	  
      return ;

      
    }

    template <typename stack_t, typename stream_t>
    static void write_stack_16(const stack_t& _stack, stream_t& _stream) {

      typedef typename stack_t::element value_type;
      
      const std::string frame_title("FRAME\n");
      const uint32_t frame_size = _stack.shape()[1]*_stack.shape()[2];

      std::vector<char> chroma_values(frame_size,0);
      const std::vector<char> all_zeros(frame_size,0);
      std::vector<char> lumi_values(frame_size,0);
      const value_type lumi_mask = 0xff;
      const value_type cb_mask = (0xff) << 8;
            
      uint32_t frame_offset = 0;
      for(std::uint32_t z = 0;z<_stack.shape()[0];++z){

	frame_offset = z*frame_size;
	//header
	_stream.write(frame_title.c_str(),frame_title.size());

	//luminance
	for(uint32_t i = 0;i<frame_size;++i){
	  lumi_values[i] = _stack.data()[frame_offset+i] & lumi_mask;
	  chroma_values[i] = (_stack.data()[frame_offset+i] & cb_mask) >> 8;
	}
	_stream.write((char*)&lumi_values[0],lumi_values.size());
	_stream.write((char*)&chroma_values[0],chroma_values.size());
	_stream.write((char*)&all_zeros[0],all_zeros.size());
      }

	  
      return ;

      
    }

    
    template <typename stack_t, typename stream_t>
    static void write_stack(const stack_t& _stack, stream_t& _stream) {

      typedef typename stack_t::element value_t;
      static_assert(sizeof(value_t)<3,"[sqeazy::yuv444formatter] unable to write anything else as 8 or 16 bit data");

      if(sizeof(value_t)==1)
	return write_stack_8(_stack, _stream);
      else
	return write_stack_16(_stack, _stream);
      
    }

    template <typename stl_vec_t, typename stream_t, typename size_t>
    static void read_stack_8(stl_vec_t& _flat_stack, stream_t& _stream, std::vector<size_t>& _shape) {

      typedef typename stl_vec_t::value_type value_t;
      if(sizeof(value_t)!=1){
	std::cerr << "[sqeazy::yuv444formatter::read_stack_8] unable to write non-8-bit data\n";
	return;
      }
      
      if(_shape.size()<2 || _shape.size()>3){
	std::cerr << "[sqeazy::yuv444formatter::read_stack_8] unable to read " << _shape.size() << "-dim data (2D or 3D allowed)\n";
	return;
      }

      _stream.seekg(0,_stream.beg);
      size_t frame_size_bytes = _shape[_shape.size()-1]*_shape[_shape.size()-2]*sizeof(value_t);
      size_t frames_consumed = 0;
      //resizing upon assumption
      _flat_stack.clear();
      _flat_stack.resize(frame_size_bytes*(_shape[_shape.size()-1]+_shape[_shape.size()-2]));
    
      for (std::string line; std::getline(_stream, line); ) {
      
	if(line.find("FRAME")!=std::string::npos){
	  //seek ptr should be after 'FRAME\n' now
	  if(_flat_stack.size()<((frames_consumed+1)*frame_size_bytes))
	    _flat_stack.resize(_flat_stack.size()+frame_size_bytes);
	  _stream.read((char*)&_flat_stack[(frames_consumed++)*frame_size_bytes],frame_size_bytes);
	
	}
      
      }

      _flat_stack.resize(frames_consumed*frame_size_bytes);

      std::vector<size_t> final_shape(3);
      
      if(_shape.size()==2){
	std::copy(_shape.begin(),_shape.end(),final_shape.begin()+1);
	_shape.clear();
	_shape.resize(3);
      }
      else
	std::copy(_shape.begin(),_shape.end(),final_shape.begin());

      final_shape[0] = frames_consumed ;
      _shape = final_shape;
      return ;
      
    }

    
    template <typename stl_vec_t, typename stream_t, typename size_t>
    static void read_stack_16(stl_vec_t& _flat_stack, stream_t& _stream, std::vector<size_t>& _shape) {

      typedef typename stl_vec_t::value_type value_t;
      if(sizeof(value_t)!=2){
	std::cerr << "[sqeazy::yuv444formatter::read_stack_16] unable to write non-16-bit data\n";
	return;
      }

      if(_shape.size()<2 || _shape.size()>3){
	std::cerr << "[sqeazy::yuv444formatter::read_stack_8] unable to read " << _shape.size() << "-dim data (2D or 3D allowed)\n";
	return;
      }

      _stream.seekg(0,_stream.beg);
      const size_t frame_size = _shape[_shape.size()-1]*_shape[_shape.size()-2];
      const size_t frame_size_bytes = frame_size*sizeof(value_t);

      std::vector<uint16_t> raw_frame(frame_size,0);
      std::vector<char> chroma_values(frame_size,0);
      std::vector<char> lumi_values(frame_size,0);
      const value_t lumi_mask = 0xff;
      const value_t cb_mask = (0xf) << 8;
      const value_t cr_mask = (0xf) << 12;
      
      size_t frames_consumed = 0;
      
      //resizing upon assumption
      _flat_stack.clear();
      _flat_stack.resize(frame_size*(_shape[_shape.size()-1]+_shape[_shape.size()-2]));
    
      for (std::string line; std::getline(_stream, line); ) {
      
	if(line.find("FRAME")!=std::string::npos){
	  _stream.read((char*)&lumi_values[0],lumi_values.size());
	  _stream.read((char*)&chroma_values[0],chroma_values.size());

	  //lumi
	  for(uint32_t i = 0;i<frame_size;++i){
	    raw_frame[i] = lumi_values[i];
	    raw_frame[i] &= 0xff;
	    value_t temp = (value_t)chroma_values[i];
	    temp <<= 8;
	    temp &= 0xff00;
	    raw_frame[i] |= temp;
	  }
	  
	  //seek ptr should be after 'FRAME\n' now
	  //hope that preserves the data
	  if(_flat_stack.size()<((frames_consumed+1)*frame_size))
	    _flat_stack.resize(_flat_stack.size()+frame_size);
	  
	  std::copy(raw_frame.begin(), raw_frame.end(),&_flat_stack[frames_consumed*frame_size]);
	  frames_consumed++;
	}
      
      }

      _flat_stack.resize(frames_consumed*frame_size);

      std::vector<size_t> final_shape(3);
      
      if(_shape.size()==2){
	std::copy(_shape.begin(),_shape.end(),final_shape.begin()+1);
	_shape.clear();
	_shape.resize(3);
      }
      else
	std::copy(_shape.begin(),_shape.end(),final_shape.begin());

      final_shape[0] = frames_consumed ;
      _shape = final_shape;
      return ;
      
    }
    
    template <typename stl_vec_t, typename stream_t, typename size_t>
    static void read_stack(stl_vec_t& _flat_stack, stream_t& _stream, std::vector<size_t>& _shape) {
      
      typedef typename stl_vec_t::value_type value_t;
      static_assert(sizeof(value_t)<3,"[sqeazy::yuv444formatter] unable to write anything else as 8 or 16 bit data");
      if(_shape.size()<2 || _shape.size()>3){
	std::cerr << "[sqeazy::yuv444formatter] unable to read " << _shape.size() << "-dim data (2D or 3D allowed)\n";
	return;
      }

      if(sizeof(value_t)==1)
	return read_stack_8(_flat_stack, _stream,_shape);
      else
	return read_stack_16(_flat_stack, _stream,_shape);
      
    }
    
  };
  
    /**
   \brief write stack to plain y4m (according to http://wiki.multimedia.cx/index.php?title=YUV4MPEG2), 
   we assert that c_storage_order is setup within _stack and the stack shape is set accordingly
   for c_storage_order that would imply shape = {z-dim,y-dim,x-dim} for a discrete euclidean 3D stack

   \param[in] _stack payload
   \param[in] _dest_stem file name to write to

   \return 
   \retval 
   
  */  
  template <typename stack_t,typename y4m_formatter = sqeazy::yuv420formatter>
  void write_stack_as_y4m(const stack_t& _stack,
			  const std::string& _dest_stem,
			  bool _verbose = false,
			  y4m_formatter _formatter = yuv420formatter()){

    typedef typename stack_t::element value_type;

    static_assert(sizeof(value_type)<3,"[sqeazy::write_stack_as_y4m] non-compliant pixel type received");

    // const uint32_t frame_size = _stack.shape()[1]*_stack.shape()[2];
    //std::vector<char> all_chroma_values(frame_size/2,0);


    std::stringstream dest_file;
    if(_dest_stem.empty())
      dest_file << "sqeazy_stack";
    else
      dest_file << _dest_stem;
    
    static const std::string suffix = ".y4m";
    if(_dest_stem.find(suffix)==std::string::npos)
      dest_file << ".y4m";
      
    std::ofstream ofile(dest_file.str(),std::ios::out |std::ios::binary | std::ios::trunc);

    std::stringstream y4m_header;
    y4m_header << "YUV4MPEG2"
	       << " W" << _stack.shape()[2]
	       << " H" << _stack.shape()[1]
	       << " F25:1"
      	       << " Ip"
      	       << " " << y4m_formatter::y4m_code()
	       << " XSQYCONVERTED=1"
	       <<"\n"
      ;

    ofile.write(y4m_header.str().c_str(),y4m_header.str().size());
    
    const std::string frame_title("FRAME\n");
    
    y4m_formatter::write_stack(_stack, ofile);
    
    ofile.close();
    
    float mbytes_written = _stack.num_elements()*1.5f;
    mbytes_written += y4m_header.str().size();
    mbytes_written += frame_title.size()*_stack.shape()[0];

    mbytes_written /= float(1<<20);
    
    if(_verbose)
      std::cout << "[sqeazy::write_stack_as_y4m] wrote " << dest_file.str() << " ("<< mbytes_written << " MB)\n";
  }

  template <typename string_it>
  static bool is_y4m_buffer(string_it _begin, string_it _end){

    // static const std::regex tag_present("YUV4MPEG2.*",std::regex::egrep);
    // static const std::regex width_present(".*[:space:]W[0-9]+[:space:].*",std::regex::egrep);
    // static const std::regex height_present(".*[:space:]H[0-9]+[:space:].*",std::regex::egrep);
    // static const std::regex fps_present(".*[:space:]F[0-9]+:.*",std::regex::egrep);
    
    bool value =	sqeazy::matches(_begin, _end,"YUV4MPEG2.*");
    bool width_found =  sqeazy::matches(_begin, _end,".*W[0-9]+.*");
    bool height_found = sqeazy::matches(_begin, _end,".*H[0-9]+.*");
    bool fps_found =  	sqeazy::matches(_begin, _end,".*F[0-9]+:.*");

    return value && width_found && height_found && fps_found;
  }

  template <typename string_t>
  static string_t y4m_header(const string_t& _path){
    std::ifstream y4m_file(_path,std::ios::in|std::ios::binary);
    string_t header;
    std::getline(y4m_file,header);
    return header;
  }
    
  /**
     \brief checks if file at _path exists, opens it and searches for y4m header
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename string_t>
  static bool is_y4m_file(const string_t& _path){

    bfs::path loc = _path;
    bool value = bfs::exists(loc);

    if(!value)
      return value;
    
    std::ifstream file_candidate(loc.string(),std::ios::in | std::ios::binary);
    std::string header;
    std::getline(file_candidate, header);


    value = is_y4m_buffer(header.begin(), header.end());
    return value;
  }

  /**
     \brief checks if file at _path exists and if the file size has modulus 4 size
     
     \param[in] 
     
     \return 
     \retval 
     
  */

  template <typename string_t>
  static bool is_yuv_file(const string_t& _path){

    bfs::path loc = _path;
    
    bool value = bfs::exists(loc);

    if(!value)
      return value;

    value = value && bfs::file_size(loc) % 4 == 0;
    return value;
  }

  /**
     \brief open file at _path and read it as it was a YUV420P y4m formatted video 
     (only the luminance is extracted and written to _data). the file header is used to extract the frame size
     
     \param[in] 
     
     \return 
     \return shape vector complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape} that describes read in shape

     
  */
  template <typename stl_vec_t, typename string_t>
  static std::vector<uint32_t> read_y4m_to_gray(stl_vec_t& _data,const string_t& _path, bool _verbose = false){

    typedef typename stl_vec_t::value_type pixel_t;
    static_assert(sizeof(pixel_t)>0 && sizeof(pixel_t)<3, "read_y4m_to_gray8\treceived vector with non-char or non-short type");
    
    std::vector<uint32_t> shape(3,0);
    _data.clear();

    
    std::ifstream y4m_file(_path,std::ios::in|std::ios::binary);
    std::string header;
    std::getline(y4m_file,header);
    std::string chroma_sampling = sqeazy::string_extract(header,"(C[0-9]+)",0);

    int re_result = 0;
    if((re_result = sqeazy::int_extract(header,"W([0-9]+)",1))>=0)
      shape[shape.size()-1] = re_result;

    if((re_result = sqeazy::int_extract(header,"H([0-9]+)",1))>=0)
      shape[shape.size()-2] = re_result;

    if(!shape[1] || !shape[2]){
      std::cerr << "unable to extract width and height from header\n\t>> " << header << "<<\n";
      shape.clear();
      return shape;
    }
    
    if(_verbose)
      std::cout << _path << ": width = " << shape[2] << ", heigth = " << shape[1] << "\n";

    if(chroma_sampling==yuv420formatter::y4m_code())
      yuv420formatter::read_stack(_data,y4m_file,shape);
    
    if(chroma_sampling==yuv444formatter::y4m_code())
      yuv444formatter::read_stack(_data,y4m_file,shape);

    
    
    if(_verbose)
      std::cout << _path << ": contained "<< shape[0] <<" frames of " << shape[2] << "x" << shape[1] << " \n";
    
    return shape;
  }

  /**
     \brief read luminance file contents of yuv file at _path into buffer _data (YUV402P is assumed), the buffer dimensions are either guesses from the file name or from a string _shape_given that should comply to WxH
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename stl_vec_t, typename string_t>
  static std::vector<uint32_t> read_yuv_to_gray8(stl_vec_t& _data,
						 const string_t& _path,
						 const string_t& _shape_given = "",
						 bool _verbose = false){

    static_assert(sizeof(_data[0])==1, "read_yuv_to_gray8\treceived vector with non-char type");

    
    std::vector<uint32_t> shape(3,0);
    _data.clear();

    int re_result = 0;
    if((re_result = sqeazy::int_extract(_path,"[wW]([0-9]+)",1))>=0)
      shape[shape.size()-1] = re_result;

    if((re_result = sqeazy::int_extract(_path,"[hH]([0-9]+)",1))>=0)
      shape[shape.size()-2] = re_result;

        
    if(!shape[1] || !shape[2]){
      if((re_result = sqeazy::int_extract(_shape_given,"[hH]([0-9]+)x",1))>=0)
	shape[shape.size()-2] = re_result;

      if((re_result = sqeazy::int_extract(_shape_given,"x([0-9]+)",1))>=0)
	shape[shape.size()-1] = re_result;
    }
        
    if(!shape[1] || !shape[2]){
      std::cerr << "unable to extract width and height from header\n\t>> " << _path << "\tor\t" << _shape_given << "<<\n";
      shape.clear();
      return shape;
    }
    
    if(_verbose)
      std::cout << _path << ": width = " << shape[2] << ", heigth = " << shape[1] << "\n";
    
    std::ifstream yuv_file(_path,std::ios::in|std::ios::binary);
    
    size_t frame_size_bytes = shape[2]*shape[1];
    size_t yuv_frame_size_bytes = 1.5*frame_size_bytes;
    
    std::string yuv_frames;

    yuv_file.seekg(0, std::ios::end);
    yuv_frames.resize(yuv_file.tellg());
    yuv_file.seekg(0, std::ios::beg);
    yuv_file.read(&yuv_frames[0], yuv_frames.size());
    yuv_file.close();

    size_t frames_needed = yuv_frames.size()/yuv_frame_size_bytes;    
    _data.resize(frame_size_bytes*frames_needed);

    for(size_t f = 0;f<frames_needed;++f){
      std::copy(&yuv_frames[f*yuv_frame_size_bytes],
		&yuv_frames[f*yuv_frame_size_bytes]+frame_size_bytes,
		&_data[f*frame_size_bytes]
		);
    }
     
    shape[0] = frames_needed;
    
    if(_verbose)
      std::cout << _path << ": contained "<< shape[0] <<" frames of " << shape[2] << "x" << shape[1] << " \n";
    
    return shape;
  }
  
}
#endif /* _YUV_UTILS_H_ */
