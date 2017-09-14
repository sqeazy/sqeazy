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
	SQY_Version_Triple - store version.

	Just store the version of sqeazy used. (NB: gives dummy values right now)

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
	length 					: (in) length in bytes of decompressed buffer
						  (out) maximum length in bytes of compressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_UI16(const char* pipeline,long* length);

/*
	SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression

	pipeline				: pipeline name
	shape					: (in) shape of the incoming nD dataset
	shape_size				: (in) number of items in shape
	length 					: (out) maximum length in bytes of compressed buffer

	Returns 0 if success, another code if there was an error (error codes provided below)
*/
SQY_FUNCTION_PREFIX int SQY_Pipeline_Max_Compressed_Length_3D_UI16(const char* pipeline,long* shape, unsigned shape_size, long* length);

/*
	SQY_Pipeline_Decompressed_Length - Extracts the size of the decompressed buffer from the first 8 Byte of data

	data					: buffer that contains the compressed data as output by SQY_PipelineEncode
	length					: (in) length in bytes of incoming data buffer
						  (out) extracted length in bytes of decompressed buffer to be output by SQY_PipelineDecode called on data

	Returns 0 if success, another code if there was an error (error codes provided below)
*/

SQY_FUNCTION_PREFIX int SQY_Pipeline_Decompressed_Length(const char* data, long *length);

/*
	SQY_PipelineDecode_UI16 - Decompress using Pipeline.

	Decompresses the source buffer into the destination buffer using Pipeline.
	The destination buffer should be allocated externally. The first 8 bytes of 
	the source buffer are not part of the Pipeline compressed buffer but indicate the
	length of the original uncompressed buffer.
	The necessary buffer length can be obtained by calling SQY_PipelineLength. 

	pipeline				: pipeline name
	src 					: Pipeline compressed buffer (externally allocated)
	srclength 				: length in bytes of compressed buffer
	dst 					: contiguous array of voxels 
							  (externally allocated, length from SQY_Pipeline_Decompressed_Length)
    nthreads                : set the number of threads allowed for the entire pipeline.

	Returns 0 if success, another code if there was an error (error codes provided below)

*/
SQY_FUNCTION_PREFIX int SQY_PipelineDecode_UI16(const char* src,
                                                long srclength,
                                                char* dst,
                                                int nthreads);

/*
	SQY_Pipeline_Possible - check if pipeline string can be used to build pipeline from

	By default 16-bit input data is assumed.

	pipeline_string				: string that describes the pipeline ('->' delimited)

	Returns true if success, false if not!

*/
SQY_FUNCTION_PREFIX bool SQY_Pipeline_Possible(const char* pipeline_string);

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
