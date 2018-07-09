#ifndef SQEAZY_H_   /* Include guard */
#define SQEAZY_H_

#include "sqeazy_definitions.h"

/*
*	Sqeazy - Fast and flexible volume compression library
*
* 	Header file
*
*	Note: endianess is little by default.
*/


/*
	SQY_Header_Size - Detect length of header stored in src.

	Search for the sqy header in src and save its length in Byte in lenght

	src 					: LZ4 compressed buffer (externally allocated & filled)
	length					: length in bytes of header in compressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Header_Size(const char* src, long* length);


/*
  SQY_Decompressed_NDims - Detect the number of dimensions contained in blob

  Search for the sqy header in src and save its length in Byte in lenght

  src 					: LZ4 compressed buffer (externally allocated & filled)
  num					: (in) size of src in bytes
                          (out) scalar that tells the number of dimensions of the data described by src

  Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Decompressed_NDims(const char* src, long* num);

/*
  SQY_Decompressed_Shape - Detect length of header stored in src.

  Search for the sqy header in src and save its length in Byte in lenght

  src 					: LZ4 compressed buffer (externally allocated & filled)
  shape					: (in) array of longs where first item describes the size of src
                               (array must be allocated to number of dimensions prior to this call)
                          (out) array holding the shape of the decoded volume in units of pixel/voxel

  Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Decompressed_Shape(const char* src, long* shape);

/*
  SQY_Decompressed_Sizeof - Pixel size in bytes

  Search for the sqy header in src and save its length in Byte in lenght

  src 					: LZ4 compressed buffer (externally allocated & filled)
  Sizeof				: (in) pointer to long which describes the length of src
                          (out) pointer to long which returns the number of bytes per pixel

  Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Decompressed_Sizeof(const char* src, long* Sizeof);


/*
	SQY_Version_Triple - store version of sqeazy into 3-element in array.

	version					: 3 element int array that holds the sqeazy version

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Version_Triple(int* version);

///////////////////////////////////////////////////////////////////////////////////
// SQY pipelines

/*
	SQY_PipelineEncode - Compress using Pipeline.

	Compresses the source buffer into the destination buffer using Pipeline.
	The destination buffer should be a bit larger than the source buffer
	in case the compressed buffer requires more space.
	ATTENTION: The output buffer contains a sqy only header!

	pipeline				: pipeline name
	src 					: contiguous array of voxels (externally allocated)
	shape     				: shape of the nD construct given as src (in units of unsigned int 8-bit)
	shape_size     				: number of items in shape
	dst 					: Pipeline compressed buffer (already allocated)
	dstlength 				: length in bytes of externally allocated destination buffer (needs to give the length of dst in Bytes),
							  modified by function call to reflect the effective
                              compressed buffer length after the call.
    nthreads                : set the number of threads allowed for the entire pipeline.

	Returns 0 if success, another code if there was an error (error codes provided below)

			error 1 -  destination buffer is not large enough

*/
SQY_FUNCTION_PREFIX int SQY_PipelineEncode_UI8(const char* pipeline,
                                                const char* src,
                                                long* shape,
                                                unsigned shape_size ,
                                                char* dst,
                                                long* dstlength,
                                                int nthreads);

/*
	SQY_PipelineEncode - Compress using Pipeline.

	Compresses the source buffer into the destination buffer using Pipeline.
	The destination buffer should be a bit larger than the source buffer
	in case the compressed buffer requires more space. 
	ATTENTION: The output buffer contains a sqy only header!

	pipeline				: pipeline name
	src 					: contiguous array of voxels (externally allocated)
	shape     				: shape of the nD construct given as src (in units of unsigned int 16-bit)
	shape_size     				: number of items in shape
	dst 					: Pipeline compressed buffer (already allocated)
	dstlength 				: length in bytes of externally allocated destination buffer (needs to give the length of dst in Bytes),
							  modified by function call to reflect the effective 
                              compressed buffer length after the call.
    nthreads                : set the number of threads allowed for the entire pipeline.

	Returns 0 if success, another code if there was an error (error codes provided below)

			error 1 -  destination buffer is not large enough

*/
SQY_FUNCTION_PREFIX int SQY_PipelineEncode_UI16(const char* pipeline,
                                                const char* src,
                                                long* shape,
                                                unsigned shape_size ,
                                                char* dst,
                                                long* dstlength,
                                                int nthreads);


/*
	SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression

	pipeline				: pipeline name
    pipeline_length         : number of bytes in <pipeline>
	length 					: (in) length of data buffer in bytes
                              (out) maximum length of compressed buffer in bytes

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_UI8(const char* pipeline,
                                                               long pipeline_length,
                                                               long* length);

/*
	SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression

	pipeline				: pipeline name
	shape					: (in) shape of the incoming nD dataset
	shape_size				: (in) number of items in shape, i.e. the number of dimensions which shape describes
    length 					: pointer to long
                              (in) number of bytes in <pipeline>
                              (out) maximum length in bytes of compressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_3D_UI8(const char* pipeline,
                                                                   long* shape,
                                                                   unsigned shape_size,
                                                                   long* length);

/*
	SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression

	pipeline				: pipeline name
	pipeline_length         : number of bytes in <pipeline>
	length 					: (in) length of pipeline buffer in bytes
                              (out) maximum length of compressed buffer in bytes

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_UI16(const char* pipeline,long pipeline_length,long* length);

/*
	SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression

	pipeline				: pipeline name
	shape					: (in) shape of the incoming nD dataset
	shape_size				: (in) number of items in shape
	length 					: pointer to long
                              (in) number of bytes in <pipeline>
                              (out) maximum length in bytes of compressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_3D_UI16(const char* pipeline,
                                                                   long* shape,
                                                                   unsigned shape_size,
                                                                   long* length);



/*
	SQY_Pipeline_Possible_UI16 - check if pipeline string can be used to build pipeline from, 16-bit input data is assumed.

	pipeline_string				: string that describes the pipeline ('->' delimited)

	Returns true if success, false if not!

*/
SQY_FUNCTION_PREFIX bool SQY_Pipeline_Possible_UI16(const char* pipeline_string);

/*
  SQY_Pipeline_Possible_UI16 - check if pipeline string can be used to build pipeline from, 8-bit input data is assumed.

  pipeline_string				: string that describes the pipeline ('->' delimited)

  Returns true if success, false if not!

*/
SQY_FUNCTION_PREFIX bool SQY_Pipeline_Possible_UI8(const char* pipeline_string);


/*
  SQY_Pipeline_Possible - check if pipeline string can be used to build pipeline from

  By default 16-bit input data is assumed.

  pipeline_string				: string that describes the pipeline ('->' delimited)
  sizeofpixel                 : sizeof pixel type, e.g. grayscale 16-bit = 2 bytes, grayscale 8-bit = 1 byte

  Returns true if success, false if not!

*/
SQY_FUNCTION_PREFIX bool SQY_Pipeline_Possible(const char* pipeline_string, int sizeofpixel);

/*
	SQY_Decompressed_Length - Extracts the size of the decompressed buffer from the contained pipeline description

	data					: buffer that contains the compressed data as output by SQY_PipelineEncode
	length					: (in)  length in bytes of incoming data buffer
                              (out) number of bytes of decompressed buffer to be output by SQY_Decode_UI16 called on data; any sizeof of the decompressed volume is already taken into account

	Returns 0 if success, another code if there was an error (error codes provided below)
*/

SQY_FUNCTION_PREFIX int SQY_Decompressed_Length(const char* data, long *length);

/*
	SQY_Decode_UI16 - Decompress using Pipeline, assuming output buffer is a uint16 typed memory location

	Decompresses the source buffer into the destination buffer using a contained Pipeline.
	The destination buffer should be allocated externally. The necessary buffer length can be
    obtained by calling SQY_PipelineLength.

	pipeline				: pipeline name
	src 					: Pipeline compressed buffer (externally allocated)
	srclength 				: length in bytes of compressed buffer
	dst 					: contiguous array of voxels, assumed to be 16-bit data behind a char* pointer
							  (externally allocated, length from SQY_Pipeline_Decompressed_Length)
    nthreads                : set the number of threads allowed for the entire pipeline.

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Decode_UI16(const char* src,
                                        long srclength,
                                        char* dst,
                                        int nthreads);

/*
	SQY_Decode_UI8 - Decompress using Pipeline, assuming output buffer is a uint8 typed memory location

	Decompresses the source buffer into the destination buffer using a contained Pipeline.
	The destination buffer should be allocated externally. The necessary buffer length can be
    obtained by calling SQY_PipelineLength.

	pipeline				: pipeline name
	src 					: Pipeline compressed buffer (externally allocated)
	srclength 				: length in bytes of compressed buffer
	dst 					: contiguous array of voxels, assumed to be 8-bit data behind a char* pointer
							  (externally allocated, length from SQY_Pipeline_Decompressed_Length)
    nthreads                : set the number of threads allowed for the entire pipeline.

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_Decode_UI8(const char* src,
                                        long srclength,
                                        char* dst,
                                        int nthreads);

///////////////////////////////////////////////////////////////////////////////////
// HDF5 filter definition
//
// references:
// http://www.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf
// http://www.hdfgroup.org/HDF5/doc/H5.user/Filters.html
// http://www.hdfgroup.org/HDF5/faq/compression.html

//
// TODO: check if these need to be here, hdf5 example does store all of this directly into library
//


///////////////////////////////////////////////////////////////////////////////////
// SQY HDF5 I/O functions 


/*
	SQY_h5_query_sizeof - query the size of the datatype stored in an hdf5 file (in byte)

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	_sizeof					: number of bytes the stored data type is wide 
						  (filled with 0 if dataset is not found)

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_query_sizeof(const char* fname,
					    const char* dname,
					    unsigned* _sizeof);

/*
	SQY_h5_query_dtype - query the type of the data stored in an hdf5 file (floating point, signed integer or unsigned integer)

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	dtype					: constant that notes if datatype found is floating point, signed integer or unsigned integer
		dtype = 0			: floating point
		dtype = 1			: signed integer
		dtype = 2			: unsigned integer

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_query_dtype(const char* fname,
					   const char* dname,
					   unsigned* dtype);

/*
	SQY_h5_query_ndims - query the rank of the data stored in an hdf5 file (1D, 2D, 3D, ...)

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	dtype					: rank of the stored data

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_query_ndims(const char* fname,
					  const char* dname,
					  unsigned* rank);

/*
	SQY_h5_query_shape - query the shape of the data stored in an hdf5 file

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	shape					: shape of the stored data (in row-wise ordering a la C), externally allocated

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_query_shape(const char* fname,
					   const char* dname,
					   unsigned* shape);

/*
	SQY_h5_read_UI16 - load contents of hdf5 file into data (if the dataset on disk is found to be compressed by sqeazy, it is uncompressed)

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	data					: data buffer (externally allocated)
    TODO: add multi-threading support for the pipeline only

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_read_UI16(const char* fname,
                                         const char* dname,
                                         unsigned short* data);
/*
	SQY_h5_write_UI16 - store unsigned 16-bit int buffer in hdf5 file (no compression is applied).

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	data					: unsigned 16-bit data to compress and store
	shape_size				: number of dimensions in data
	shape					: dimension of data
	
	filter					: filter to use
    TODO: add multi-threading support for the pipeline only

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_write_UI16(const char* fname,
					  const char* dname,
					  const unsigned short* data,
					  unsigned shape_size,
					  const unsigned* shape,
					  const char* filter);

/*
	SQY_h5_write - store compressed data into file.

	fname 					: hdf5 file to store data in
	dname 					: dataset name inside hdf5 file 
	data					: compressed data
    data_size				: size of data in byte

    TODO: add multi-threading support for the pipeline only

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_write(const char* fname,
				     const char* dname,
				     const char* data,
				     unsigned long data_size);

/*
	SQY_h5_link - establish hdf5 link between src.h5:/path/to/object/by_name and dest.h5:/path/to/object/by_name
	
	pSrcFileName				: path to file the link is stored in
	pSrcLinkPath				: path/group inside pSrcFileName 
	pSrcLinkName				: name of link inside pSrcFileName 

	pDestFileName				: path to file the dataset the link points to
	pDestDatasetPath			: path/group inside pDestFileName where dataset is located 
	pDestDatasetName			: name of dataset inside pDestFileName

	Returns 0 if success, another code if there was an error

*/
SQY_FUNCTION_PREFIX int SQY_h5_link(	const char* pSrcFileName,
					const char* pSrcLinkPath,
					const char* pSrcLinkName,
					const char* pTargetFile,
					const char* pTargetDatasetPath,
					const char* pTargetDatasetName	
					);

#endif
