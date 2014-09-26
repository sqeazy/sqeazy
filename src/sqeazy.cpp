#define SQEAZY_CPP_
#include "sqeazy.h"
#include "sqeazy_impl.hpp"

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

int SQY_LZ4Encode(const char* src, long srclength, char* dst, long* dstlength){

  int retvalue = sqeazy::lz4_scheme<char,long>::encode(src,dst,srclength,*dstlength);
  
  return retvalue;
}


int SQY_LZ4_Max_Compressed_Length(long* length){
  
  long value = sqeazy::lz4_scheme<char>::max_encoded_size(*length);//compression size + one long to encode size of
  *length = value;
  return 0;

}

int SQY_LZ4_Decompressed_Length(const char* data, long *length){
  
  
  *length = sqeazy::lz4_scheme<char>::decoded_size_byte(data,*length);
  return 0;

}



int SQY_LZ4Decode(const char* src, long srclength, char* dst){

  

  int retcode = sqeazy::lz4_scheme<char>::decode(src,dst,srclength);
  
  return retcode;
}

