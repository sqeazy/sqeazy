#ifndef _SQEAZY_PREDEF_PIPELINES_H_
#define _SQEAZY_PREDEF_PIPELINES_H_

#include "sqeazy_impl.hpp"
#include "pipeline.hpp"
#include "external_encoders.hpp"


namespace sqeazy {
  ////////////////////////////////////////////////////////////////////////////////
  // 32-bit
  //CONVENIENCE
  typedef sqeazy::bmpl::vector< sqeazy::pass_through<int> > int32_passthrough;
  typedef sqeazy::pipeline<int32_passthrough> int32_passthrough_pipe;

  typedef sqeazy::bmpl::vector< sqeazy::pass_through<unsigned int> > uint32_passthrough;
  typedef sqeazy::pipeline<uint32_passthrough> uint32_passthrough_pipe;

  
  ////////////////////////////////////////////////////////////////////////////////
  // 16-bit
  
  
  //LOSSY
  typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
				sqeazy::bitswap_scheme<unsigned short>,
				sqeazy::lz4_scheme<unsigned short> > rmbkg_bswap1_lz4;
  typedef sqeazy::pipeline<rmbkg_bswap1_lz4> rmbkg_bswap1_lz4_pipe;

  //LOSSLESS
  typedef sqeazy::bmpl::vector< sqeazy::bitswap_scheme<unsigned short>,
				sqeazy::lz4_scheme<unsigned short> > bswap1_lz4;
  typedef sqeazy::pipeline<bswap1_lz4> bswap1_lz4_pipe;

  typedef sqeazy::bmpl::vector< sqeazy::lz4_scheme<unsigned short> > lz4_;
  typedef sqeazy::pipeline<lz4_> lz4_pipe;

  //CONVENIENCE
  typedef sqeazy::bmpl::vector< sqeazy::pass_through<unsigned short> > uin16_passthrough;
  typedef sqeazy::pipeline<uin16_passthrough> uint16_passthrough_pipe;

  ////////////////////////////////////////////////////////////////////////////////
  // 8-bit

  //LOSSY
  typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned char>,
				sqeazy::bitswap_scheme<unsigned char>,
				sqeazy::lz4_scheme<unsigned char> > char_rmbkg_bswap1_lz4;
  typedef sqeazy::pipeline<char_rmbkg_bswap1_lz4> char_rmbkg_bswap1_lz4_pipe;

  // LOSSLESS
  typedef sqeazy::bmpl::vector< sqeazy::bitswap_scheme<unsigned char>,
				sqeazy::lz4_scheme<unsigned char> > char_bswap1_lz4;
  typedef sqeazy::pipeline<char_bswap1_lz4> char_bswap1_lz4_pipe;

  typedef sqeazy::bmpl::vector< sqeazy::lz4_scheme<unsigned char> > char_lz4_;
  typedef sqeazy::pipeline<lz4_> char_lz4_pipe;

  //CONVENIENCE
  typedef sqeazy::bmpl::vector< sqeazy::pass_through<unsigned char> > uint8_passthrough;
  typedef sqeazy::pipeline<uint8_passthrough> uint8_passthrough_pipe;


};

#endif /* _BENCH_COMMON_H_ */
