#define SQEAZY_CPP_
#include "sqeazy.h"
#include "sqeazy_impl.hpp"
#include "sqeazy_header.hpp"
#include "pipeline_select.hpp"
#include "sqeazy_hdf5_impl.hpp"
#include "hdf5_utils.hpp"
#include "pipeline.hpp"

/*
*	Sqeazy - Fast and flexible volume compression library
*
* 	Header file
*
*	Note: endianess is little by default.
*/

int SQY_RasterDiffEncode_3D_UI16(int width, int height, int depth, const char* src, char* dst){
  
  typedef unsigned short raw_type;
  typedef short compressed_type;

  return sqeazy::diff_scheme<raw_type>::encode(width, height, depth, 
						 reinterpret_cast<const raw_type*>(src),
						 reinterpret_cast<compressed_type*>(dst)
						 );
  
}


int SQY_RasterDiffDecode_3D_UI16(int width, int height, int depth, const char* src, char* dst){
  
  typedef unsigned short raw_type;
  typedef short compressed_type;

  return sqeazy::diff_scheme<raw_type>::decode(width, height, depth, 
					       reinterpret_cast<const compressed_type*>(src),
					       reinterpret_cast<raw_type*>(dst)
					       );

  
}


int SQY_BitSwap4Encode_UI16(const char* src, char* dst, long length){ 

  typedef unsigned short raw_type;

  return sqeazy::bitswap_scheme<raw_type,4>::encode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length/sizeof(raw_type)
						  );

}

int SQY_BitSwap4Encode_I16(const char* src, char* dst, long length){ 

  typedef short raw_type;
  
  return sqeazy::bitswap_scheme<raw_type,4>::encode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length/sizeof(raw_type)
						  );

}


int SQY_BitSwap4Decode_UI16(const char* src, char* dst, long length){
  
  typedef unsigned short raw_type;
  return sqeazy::bitswap_scheme<raw_type,4>::decode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length/sizeof(raw_type)
						  );
}

int SQY_BitSwap4Decode_I16(const char* src, char* dst, long length){
  
  typedef short raw_type;
  return sqeazy::bitswap_scheme<raw_type,4>::decode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length/sizeof(raw_type)
						  );
}



int SQY_BitSwap8Encode_UI16(const char* src, char* dst, long length){
    typedef unsigned short raw_type;
    return sqeazy::bitswap_scheme<raw_type,8>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap8Decode_UI16(const char* src, char* dst, long length){
  
  typedef unsigned short raw_type;
  return sqeazy::bitswap_scheme<raw_type,8>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}

int SQY_BitSwap8Encode_I16(const char* src, char* dst, long length){
    typedef short raw_type;
    return sqeazy::bitswap_scheme<raw_type,8>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap8Decode_I16(const char* src, char* dst, long length){
  
  typedef  short raw_type;
  return sqeazy::bitswap_scheme<raw_type,8>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}

int SQY_BitSwap2Encode_UI16(const char* src, char* dst, long length){
    typedef unsigned short raw_type;
    return sqeazy::bitswap_scheme<raw_type,2>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap2Decode_UI16(const char* src, char* dst, long length){
  
  typedef unsigned short raw_type;
  return sqeazy::bitswap_scheme<raw_type,2>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}

int SQY_BitSwap2Encode_I16(const char* src, char* dst, long length){
    typedef short raw_type;
    return sqeazy::bitswap_scheme<raw_type,2>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap2Decode_I16(const char* src, char* dst, long length){
  
  typedef  short raw_type;
  return sqeazy::bitswap_scheme<raw_type,2>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}


int SQY_BitSwap1Encode_UI16(const char* src, char* dst, long length){
    typedef unsigned short raw_type;
    return sqeazy::bitswap_scheme<raw_type,1>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap1Decode_UI16(const char* src, char* dst, long length){
  
  typedef unsigned short raw_type;
  return sqeazy::bitswap_scheme<raw_type,1>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}

int SQY_BitSwap1Encode_I16(const char* src, char* dst, long length){
    typedef short raw_type;
    return sqeazy::bitswap_scheme<raw_type,1>::encode(reinterpret_cast<const raw_type*>(src),
						      reinterpret_cast<raw_type*>(dst),
						      length/sizeof(raw_type)
						      );
}


int SQY_BitSwap1Decode_I16(const char* src, char* dst, long length){
  
  typedef  short raw_type;
  return sqeazy::bitswap_scheme<raw_type,1>::decode(reinterpret_cast<const raw_type*>(src),
						    reinterpret_cast<raw_type*>(dst),
						    length/sizeof(raw_type)
						    );
}

int SQY_RLEEncode_UI8(const char* src, char* dst, long length, long minrunlength){return 42;}


int SQY_RLEDecode_UI8(const char* src, char* dst, long length){return 42;}


int SQY_RmBackground_AtMode_UI16(char* src, char* dst, long length, unsigned short epsilon){

  typedef unsigned short raw_type;
  return sqeazy::remove_background<raw_type>::encode(reinterpret_cast<raw_type*>(src),
							reinterpret_cast<raw_type*>(dst),
							length/sizeof(raw_type),
							epsilon
							);

}

int SQY_RmBackground_Estimated_UI16(int width, int height, int depth, char* src, char* dst){  
  typedef unsigned short raw_type;
  std::vector<int> dims(3);
  dims[0] = width;
  dims[1] = height;
  dims[2] = depth;
    return sqeazy::remove_estimated_background<raw_type>::encode(reinterpret_cast<raw_type*>(src),
            reinterpret_cast<raw_type*>(dst),
            dims
								    );
}

#ifndef LZ4_VERSION_MAJOR
#include "external_encoders.hpp"
#endif

typedef sqeazy::bmpl::vector< sqeazy::lz4_scheme<char> > lz4_;
typedef sqeazy::pipeline<lz4_> lz4_pipe;


int SQY_LZ4Encode(const char* src, long srclength, char* dst, long* dstlength){

  int retvalue = lz4_pipe::compress(src,dst,srclength,*dstlength);
  
  return retvalue;
}


int SQY_LZ4_Max_Compressed_Length(long* length){
  
  std::vector<unsigned> shape(1);
  shape[0] = *length;
  sqeazy::image_header hdr(char(),shape, lz4_pipe::name());
  
  long value = lz4_pipe::max_bytes_encoded(*length, hdr.size());//compression size + one long to encode size of
  *length = value;
  return 0;

}

int SQY_LZ4_Decompressed_Length(const char* data, long *length){
  
  sqeazy::image_header hdr(data, data + *length);
  if(!hdr.empty())
    *length = hdr.raw_size_byte();
  else
    *length = 0;
  return 0;

}



int SQY_LZ4Decode(const char* src, long srclength, char* dst){

  int retcode = lz4_pipe::decompress(src,dst,srclength);
  
  return retcode;
}


int SQY_Header_Size(const char* src, long *srclength){

  sqeazy::image_header hdr(src, src + *srclength);
  *srclength = hdr.size();
  
  return 0;
}

int SQY_Version_Triple(int* version){
  version[0] = 2015;//placeholder
  version[1] = 4;//placeholder
  version[2] = 28;//placeholder

  return 0;
}

template <typename T>
inline static std::string prepend_type_id(const std::string& _in){

  static std::string type_name = sqeazy::type_to_name_match<unsigned short>::id();
  std::string pipeline = _in;
  std::stringstream filter_name_;
  if(pipeline.find(type_name)==std::string::npos)
    filter_name_ << type_name << "_";
  filter_name_ << pipeline;

  return filter_name_.str();
}

template <typename T>
inline static std::string prepend_type_id(const char* _in){

  std::string pipeline = _in;
  return prepend_type_id<T>(pipeline);
}

int SQY_PipelineEncode_UI16(const char* pipeline,
			    const char* src,
			    long* shape,
			    unsigned shape_size ,
			    char* dst,
			    long* dstlength){

  int value =1;
  std::vector<unsigned> shape_(shape, shape+shape_size);
  
  std::string filter_name = prepend_type_id<uint16_t>(pipeline);

  
  sqeazy::image_header hdr(uint16_t(),
			   shape_,
			   filter_name);

  sqeazy::pipeline_select<> selected(16,filter_name);
  if(selected.empty())
    return value;

  unsigned long num_bytes_written = 0;
  value = selected.compress(src,dst,shape_,num_bytes_written);
  *dstlength = num_bytes_written;
  return value;
}

//NOT IMPLEMENTED YET
int SQY_Pipeline_Max_Compressed_Length_UI16(const char* pipeline,long* length){
  int value =1;
  std::string filter_name = prepend_type_id<uint16_t>(pipeline);
  sqeazy::pipeline_select<> selected(16,filter_name);
  if(selected.empty())
    return value;

  std::vector<unsigned long> assumed_shape(1,*length);
  sqeazy::image_header hdr(uint16_t(),
			   assumed_shape,
			   filter_name);
  
  *length = selected.max_compressed_size(*length,hdr.size());
  return 0;

}

int SQY_Pipeline_Max_Compressed_Length_3D_UI16(const char* pipeline,long* shape, unsigned shape_size, long* length){

  int value =1;
  std::string filter_name = prepend_type_id<uint16_t>(pipeline);
  sqeazy::pipeline_select<> selected(16,filter_name);
  if(selected.empty())
    return value;

  std::vector<unsigned long> lshape(shape,shape+shape_size);
  sqeazy::image_header hdr(uint16_t(),
			   lshape,
			   filter_name);
  
  *length = selected.max_compressed_size(*length,hdr.size());
  return 0;


}

int SQY_Pipeline_Decompressed_Length(const char* data, long *length){
  
  int value =1;

  sqeazy::image_header hdr(data,data+(*length));
  sqeazy::pipeline_select<> selected(hdr);
  if(selected.empty())
    return value;

  *length = selected.decoded_size_byte(data,*length);
  return 0;
}

int SQY_PipelineDecode_UI16(const char* src, long srclength, char* dst){
  int value =1;
 
  sqeazy::image_header hdr(src,src+srclength);
  if(hdr.empty())
    return value;
  
  sqeazy::pipeline_select<> selected(hdr);
  if(selected.empty())
    return value;

  unsigned long long size = srclength;
  value = selected.decompress(src,dst,size);

  return value;
}


int SQY_h5_query_sizeof(const char* fname,
			const char* dname,
			unsigned* _sizeof){

  int rvalue = 1;
  
  H5::Exception::dontPrint();  
  sqeazy::h5_file loaded(fname);
  if(!loaded.ready())
    return rvalue;
  else{
    std::string dn = dname;
    *_sizeof = loaded.type_size_in_byte(dn);
    rvalue = (*_sizeof) > 0 ? 0 : 1;
  }
  
  return rvalue;
}

int SQY_h5_query_dtype(const char* fname,
			const char* dname,
			unsigned* dtype){

  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname);
  if(!loaded.ready())
    return 1;
  else{
    std::string dn = dname;
    *dtype = 0;
    if(loaded.is_integer(dn)){
      *dtype += 1;
    if(loaded.is_signed(dn))
      *dtype += 1;
    }
  }

  return 0;
}

int SQY_h5_query_ndims(const char* fname,
		      const char* dname,
		      unsigned* ndims){

  int rvalue = 1;
  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname);
  if(!loaded.ready())
    return rvalue;
  else{
    std::string dn = dname;
    *ndims = 0;
    std::vector<int> shape;
    loaded.shape(shape, dn);
    rvalue = shape.empty();
    *ndims = shape.size();
  }

  return rvalue;

}

int SQY_h5_query_shape(const char* fname,
		       const char* dname,
		       unsigned* shape){

  
  int rvalue = 1;
  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname);
  if(!loaded.ready())
    return rvalue;
  else{
    std::string dn = dname;
    *shape = 0;
    std::vector<int> local;
    loaded.shape(local, dn);
    rvalue = local.empty();
    std::copy(local.begin(), local.end(), shape);
  }

  return rvalue;
}

int SQY_h5_write_UI16(const char* fname,
		      const char* dname,
		      const unsigned short* data,
		      unsigned shape_size,
		      const unsigned* shape,
		      const char* filter){

  int rvalue = 1;
  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname, H5F_ACC_TRUNC);
  if(!loaded.ready())
    return rvalue;
  else{

    std::string in_filter = filter;
    
    if(in_filter.empty())
      rvalue = loaded.write_nd_dataset(dname,
				       data,
				       shape,
				       shape_size);
    else{
      std::stringstream filter_name_;
      filter_name_ << sqeazy::type_to_name_match<unsigned short>::id() << "_" << in_filter;
      std::string filter_name = filter_name_.str();
      rvalue = loaded.write_nd_dataset(dname,
				       filter_name.c_str(),
    				       data,
    				       shape,
    				       shape_size
    				       );
    }
    
  }

  return rvalue;
}

int SQY_h5_write(const char* fname,
		 const char* dname,
		 const char* data,
		 unsigned long data_size){

  int rvalue = 1;
  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname, H5F_ACC_TRUNC);
  if(!loaded.ready())
    return rvalue;
  else{

    rvalue = loaded.write_compressed_buffer(dname,
					    data,
					    data_size);
    
  }

  return rvalue;
}

int SQY_h5_read_UI16(const char* fname,
		      const char* dname,
		      unsigned short* data){
  int rvalue = 1;
  H5::Exception::dontPrint();
  sqeazy::h5_file loaded(fname);
  if(!loaded.ready())
    return rvalue;
  if(!loaded.has_dataset(dname))
    return rvalue;
  else{
    std::vector<int> shape;
    rvalue = loaded.read_nd_dataset(dname,
				    data,
				    shape);
  }

  return rvalue;
}


