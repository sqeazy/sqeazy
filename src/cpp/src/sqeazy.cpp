#define SQEAZY_CPP_

#include "boost/filesystem.hpp"

#include "sqeazy.h"

#include "sqeazy_header.hpp"
#include "sqeazy_pipelines.hpp"

#include "sqeazy_hdf5_impl.hpp"
#include "hdf5_utils.hpp"

namespace sqy = sqeazy;
namespace bfs = boost::filesystem;

int SQY_Header_Size(const char* src, long *srclength){

  sqy::header hdr(src, src + *srclength);
  *srclength = hdr.size();

  return 0;
}

int SQY_Decompressed_NDims(const char* data,
                           long *num){

  int value =0;

  sqy::header hdr(data,data+(*num));

  *num = hdr.shape()->size();
  return value;
}

int SQY_Decompressed_Shape(const char* data,
                           long *shape){

  int value =0;

  sqy::header hdr(data,data+shape[0]);

  for(std::size_t i = 0;i<hdr.shape()->size();++i)
    shape[i] = hdr.shape()->at(i);

  return value;
}

int SQY_Decompressed_Sizeof(const char* data,
                           long *Sizeof){

  int value =0;

  sqy::header hdr(data,data+(*Sizeof));

  *Sizeof = hdr.sizeof_header_type();

  return value;
}


int SQY_Version_Triple(int* version){

  version[0] = sqeazy_global_version_major;
  version[1] = sqeazy_global_version_minor;
  version[2] = sqeazy_global_version_patch;

  return 0;
}

int SQY_PipelineEncode_UI16(const char* pipeline,
                            const char* src,
                            long* shape,
                            unsigned shape_size ,
                            char* dst,
                            long* dstlength,
                            int nthreads)
{

  int value =1;
  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline))
    return value;


  std::vector<std::size_t> shape_(shape, shape+shape_size);
  auto pipe = sqy::dypeline<std::uint16_t>::from_string(pipeline);
  if(pipe.empty()){
    std::cerr << "[sqeazy]\t received " << pipe.name() << "pipeline of size 0, cannot encode buffer\n";
    return value;
  }

  pipe.set_n_threads(nthreads);

  char* encoded_end = pipe.encode(reinterpret_cast<const std::uint16_t*>(src),
                  dst,
                  shape_);

  if(!encoded_end)
    return value;
  else
    value = 0;

  *dstlength = encoded_end - dst;
  return value;
}

//NOT IMPLEMENTED YET
int SQY_Pipeline_Max_Compressed_Length_UI16(const char* pipeline,long* length){

  int value =1;

  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline))
    return value;

  auto received_pipeline = sqy::dypeline<std::uint16_t>::from_string(pipeline);

  if(!received_pipeline.size()){
    std::cerr << "[sqeazy]\t received " << received_pipeline.name() << "pipeline of size 0, cannot compite Max_Compressed_Length\n";
    return value;
  }

  *length = received_pipeline.max_encoded_size(*length);
  return 0;

}

int SQY_Pipeline_Max_Compressed_Length_3D_UI16(const char* pipeline,
                           long* shape,
                           unsigned shape_size,
                           long* length){

  int value = 1;
  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline))
    return value;

  std::uintmax_t size_in_byte = sizeof(std::uint16_t)*std::accumulate(shape,shape+shape_size,1,std::multiplies<long>());

  auto received_pipeline = sqy::dypeline<std::uint16_t>::from_string(pipeline);

  if(!received_pipeline.size()){
    std::cerr << "[sqeazy]\t received " << received_pipeline.name() << "pipeline of size 0, cannot compite Max_Compressed_Length\n";
    return value;}

  *length = received_pipeline.max_encoded_size(size_in_byte);

  return 0;

}

int SQY_Decompressed_Length(const char* data,
                            long *length){

  int value =0;

  sqy::header hdr(data,data+(*length));

  *length = hdr.raw_size_byte();
  return value;
}

int SQY_PipelineDecode_UI16(const char* src, long srclength, char* dst, int nthreads){
  int value =1;

  sqy::header hdr(src,src+(srclength));

  if(!sqy::dypeline<std::uint16_t>::can_be_built_from(hdr.pipeline())){
    std::cerr << "[sqeazy]\t" << hdr.pipeline() << " cannot be build with this version of sqeazy\n";
    return value;
  }

  auto pipe = sqy::dypeline<std::uint16_t>::from_string(hdr.pipeline());
  pipe.set_n_threads(nthreads);

  if(!pipe.size()){
    std::cerr << "[sqeazy]\t received " << pipe.name() << "pipeline of size 0, no decoding possible\n";
    return value;}

  std::vector<std::size_t> inshape_  = {std::size_t(srclength)};
  std::vector<std::size_t> outshape_(hdr.shape()->begin(),hdr.shape()->end());

  value = pipe.decode(src,
              reinterpret_cast<std::uint16_t*>(dst),
              inshape_,
              outshape_);

  return value;
}

bool SQY_Pipeline_Possible(const char* pipeline_string){

  bool value = false;

  value = sqy::dypeline<std::uint16_t>::can_be_built_from(pipeline_string);

  return value;

}

int SQY_h5_query_sizeof(const char* fname,
            const char* dname,
            unsigned* _sizeof){

  int rvalue = 1;

#ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif

  bfs::path lpath = fname;
  sqy::h5_file loaded(lpath);
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

#ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif
  bfs::path lpath = fname;
  sqy::h5_file loaded(lpath);
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
  #ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif
  bfs::path lpath = fname;
  sqy::h5_file loaded(lpath);
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
  #ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif
  bfs::path lpath = fname;
  sqy::h5_file loaded(lpath);
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
  #ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif

  bfs::path src_p = fname;

  sqy::h5_file loaded(src_p, bfs::exists(src_p) ? H5F_ACC_RDWR : H5F_ACC_TRUNC);

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
      rvalue = loaded.write_nd_dataset(dname,
                       in_filter.c_str(),
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
  #ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif

  bfs::path src_p = fname;
  sqy::h5_file loaded(src_p, bfs::exists(src_p) ? H5F_ACC_RDWR : H5F_ACC_TRUNC);

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
#ifndef _SQY_DEBUG_
  H5::Exception::dontPrint();
#endif
  bfs::path lpath = fname;
  sqy::h5_file loaded(lpath);

  if(!loaded.ready())
    return rvalue;
  if(!loaded.has_h5_item(dname))
    return rvalue;
  else{
    std::vector<int> shape;
    rvalue = loaded.read_nd_dataset(dname,
                    data,
                    shape);
  }

  return rvalue;
}

int SQY_h5_link(const char* pSrcFileName,
        const char* pSrcLinkPath,
        const char* pSrcLinkName,
        const char* pTargetFile,
        const char* pTargetDatasetPath,
        const char* pTargetDatasetName	){
  int rvalue = 1;
  //  H5::Exception::dontPrint();

  bfs::path src_p = pSrcFileName;
  sqy::h5_file src(src_p, bfs::exists(src_p) ? H5F_ACC_RDWR : H5F_ACC_TRUNC);
  std::stringstream src_path("");
  src_path << pSrcLinkPath << "/" << pSrcLinkName;

  bfs::path dst_p = pTargetFile;
  sqy::h5_file dest(dst_p);
  std::stringstream dest_path("");
  dest_path << pTargetDatasetPath << "/" << pTargetDatasetName;

  if(dest.has_h5_item(dest_path.str()))
    rvalue = src.setup_link(src_path.str(),dest,dest_path.str());
  else
    rvalue = 1;

  return rvalue;
}
