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

  return sqeazy::bitswap_scheme<raw_type>::encode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length
						  );

}

int SQY_BitSwap4Decode_UI16(const char* src, char* dst, long length){
  
  typedef unsigned short raw_type;
  return sqeazy::bitswap_scheme<raw_type>::decode(reinterpret_cast<const raw_type*>(src),
						  reinterpret_cast<raw_type*>(dst),
						  length
						  );
}


int SQY_BitSwap8Encode_UI16(const char* src, char* dst, long length){return 42;}


int SQY_BitSwap8Decode_UI16(const char* src, char* dst, long length){return 42;}


int SQY_RLEEncode_UI8(const char* src, char* dst, long length, long minrunlength){return 42;}


int SQY_RLEDecode_UI8(const char* src, char* dst, long length){return 42;}


#include "lz4.h"

int SQY_LZ4Encode(const char* src, long srclength, char* dst, long* dstlength){

  int retcode = LZ4_compress(src,dst,srclength);
  
  if(retcode){
    *dstlength = retcode;
    return 0;
  }
  else{
    *dstlength = 0;
    return -1;
  }
}


int SQY_LZ4Length(const char* buffer, long* length){
  
  return LZ4_compressBound(*length);

}


int SQY_LZ4Decode(const char* src, long srclength, char* dst){

  int maxOutputSize = LZ4_compressBound(srclength);
  int retcode = LZ4_decompress_safe(src,dst,srclength, maxOutputSize);
  
  return retcode;
}

