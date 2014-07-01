#ifndef SQEASY_H_   /* Include guard */
#define SQEASY_H_

/*
*	Sqeasy - Fast and flexible volume compression library
*
* 	Header file
*
*	Note: endianess is little by default.
/**/


/*
*	Example pipeline:
*
*	Encoding:
*	SQY_RasterDiffEncode_3D_UI16 -> SQY_BitSwap4Encode_UI16 -> SQY_RLEEncode_UI8 -> SQY_LZ4Encode
*
*
*	Decoding:
*	SQY_LZ4Decode -> SQY_RLEDecode_UI8 -> SQY_BitSwap4Decode_UI16 -> SQY_RasterDiffDecode_3D_UI16
*
/**/



//#############################################################################
// Differential coding schemes:

/*
	SQY_RasterDiffEncode_3D_UI16 - Raster differencing scheme on 3D unsigned 16 bit int data.

	Encoding: Each following voxel is visited in row major order and the difference between the current voxel
	and the average of already visited neighboring voxels is computed and stored at the
	corresponding location in the destination buffer. Boundary conditions: if a neighbor of a 
	voxel is outside of the volume, its value is assumed to be 0. For the first voxel, all neighbors
	are outside of the volume and the average is thus 0.

	width, height, depth 	: volume dimensions in voxels
	src 					: row major 3D contiguous array of voxels (externally allocated)
	dst 					: row major 3D contiguous array of voxels (externally allocated)

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_RasterDiffEncode_3D_UI16(int width, int height, int depth, const char* src, char* dst);

/*
	SQY_RasterDiffDecode_3D_UI16 - Raster differencing scheme on 3D unsigned 16 bit int data.

	Decoding: Each voxel is visited in row major order. The voxel in the destination buffer at the
	current location is computed as the sum of the average of the already decoded neighboring voxels
	and the current difference value stored in the source buffer at the current location.
	Boundary conditions: if a neighbor of a voxel is outside of the volume, its value is assumed to be 0.
	For the first voxel, all neighbors are outside of the volume and the average is thus 0.

	width, height, depth 	: volume dimensions in voxels
	src 					: row major 3D contiguous array of voxels (externally allocated)
	dst 					: row major 3D contiguous array of voxels (externally allocated)

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_RasterDiffDecode_3D_UI16(int width, int height, int depth, const char* src, char* dst);


//#############################################################################
// Bit-swapping coding schemes:


/*
	SQY_BitSwap4Encode_UI16 - 4 Bit swapping scheme on unsigned 16 bit int data.

	Encoding: Each 16 bit integer is split in four 4 bit parts: Ai, Bi, Ci, Di.
	The destination buffer is filled with first all Ai, then all Bi, then all Ci, 
	then all Di. Thus the destination buffer has the same length as the source buffer.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_BitSwap4Encode_UI16(const char* src, char* dst, long length);

/*
	SQY_BitSwap4Decode_UI16 - 4 Bit swapping scheme on unsigned 16 bit int data.

	Encoding: The permutation described above is inverted.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_BitSwap4Decode_UI16(const char* src, char* dst, long length);

/*
	SQY_BitSwap8Encode_UI16 - 8 Bit swapping scheme on unsigned 16 bit int data.

	Encoding: Each 16 bit integer is split in two 8 bit parts: Ai, Bi.
	The destination buffer is filled with first all Ai, then all Bi, 
	Thus the destination buffer has the same length as the source buffer.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_BitSwap8Encode_UI16(const char* src, char* dst, long length);

/*
	SQY_BitSwap8Decode_UI16 - 8 Bit swapping scheme on unsigned 16 bit int data.

	Encoding: The permutation described above is inverted.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_BitSwap8Decode_UI16(const char* src, char* dst, long length);


//#############################################################################
// Run-length encoding schemes (Optional)

/*
	SQY_RLEEncode_UI8 - 8 Bit Run length encoding scheme.

	Encoding: The first 8 bytes represented an unsigned 
	a contiguous stretch of repeated bytes: i.e. XXXXXXX...X of length n
	with n >= minrunlength is outputed as the byte sequence: 

				1nX 	: where 1 indicates that integer n is encoded with 1 byte (unsigned).
						  this is in the case that n<256
				2nnX	: where	2 indicates that the integer n is encoded with 2 bytes (unsigned).
						  this is the case that  256<= n < 256*256	
				4nnnnX	: where	4 indicates that the integer n is encoded with 4 bytes (unsigned).
						  this is the case that  256*256<= n 		

	Regions of length l (i.e.: 'XYZ...XYZ' )that are not repeated or with repetitions of length less than 
	minrunlength are simply copied to the destination buffer with a the following similar block structure:

				OlXYZ...XYZ 	: where O (O=One=1) indicates that integer l is encoded with 1 byte (unsigned).
						  		 this is in the case that l<256
				TllXYZ...XYZ 	: where	T (T=Two=2) indicates that the integer l is encoded with 2 bytes (unsigned).
						  		  this is the case that  256<= l < 256*256	
				FllllXYZ...XYZ 	: where	F (F=Four=4) indicates that the integer l is encoded with 4 bytes (unsigned).
						  		  this is the case that  256*256<= l

	The block headers 1n, 2nn, 4nnnn, Ol, Tll, Fllll  ensure that the output stream is made of blocks 
	of clearly defined lengths.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers
	minrunlength			: minimal run length to encode

	Returns 0 if success, another code if there was an error (error codes provided below)

	extensions 1: if src==dst the function is capable of doing in-place processing.	
*/
int SQY_RLEEncode_UI8(const char* src, char* dst, long length, long minrunlength);


/*
	SQY_RLEDecode_UI8 - 8 Bit Run length encoding scheme.

	Decoding: The block stream described above is parsed and a decompressed stream of bytes
	is appended to the output buffer.

	src 					: contiguous array of voxels (externally allocated)
	dst 					: contiguous array of voxels (externally allocated)
	length 					: length of both buffers
	minrunlength			: minimal run length to encode

	Returns 0 if success, another code if there was an error (error codes provided below)	
*/
int SQY_RLEDecode_UI8(const char* src, char* dst, long length);


//#############################################################################
// Entropy encoding coding schemes:

/*
	SQY_LZ4Encode - Compress using LZ4.

	Compresses the source buffer into the destination buffer using LZ4.
	The destination buffer should be a bit larger than the source buffer
	in case the compressed buffer requires more space. 
	The first 8 bytes of the destination buffer encode a 64 bit long that
	represents the length of the source buffer.


	src 					: contiguous array of voxels (externally allocated)
	srclength 				: length in bytes of source buffer
	dst 					: LZ4 compressed buffer (already allocated)
	dstlength 				: length in bytes of externally allocated destination buffer,
							  modified by function call to reflect the effective 
							  compressed buffer length after the call.

	Returns 0 if success, another code if there was an error (error codes provided below)

			error 1 -  destination buffer is not large enough

*/
int SQY_LZ4Encode(const char* src, long srclength, char* dst, long* dstlength);



/*
	SQY_LZ4Length - Returns the decompressed length of a LZ4 compressed buffer.

	The first 8 bytes of the destination buffer encode a 64 bit long that
	represents the length of the source buffer.

	buffer 					: LZ4 compressed buffer (externally allocated)
	length 					: length in bytes of decompressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
int SQY_LZ4Length(const char* buffer, long* length);

/*
	SQY_LZ4Decode - Decompress using LZ4.

	Decompresses the source buffer into the destination buffer using LZ4.
	The destination buffer should be allocated externally. The first 8 bytes of 
	the source buffer are not part of the LZ4 compressed buffer but indicate the
	length of the original uncompressed buffer.
	The necessary buffer length can be obtained by calling SQY_LZ4Length. 

	src 					: LZ4 compressed buffer (externally allocated)
	srclength 				: length in bytes of compressed buffer
	dst 					: contiguous array of voxels 
							  (externally allocated, length from SQY_LZ4Length)

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
int SQY_LZ4Decode(const char* src, long srclength, char* dst);


#endif