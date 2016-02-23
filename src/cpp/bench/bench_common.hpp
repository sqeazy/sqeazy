#ifndef _BENCH_COMMON_H_
#define _BENCH_COMMON_H_


#include "pipeline.hpp"
#include "encoders/external_encoders.hpp"
#include "encoders/sqeazy_impl.hpp"

////////////////////////////////////////////////////////////////////////////////
// 16-bit

typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::diff_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_diff_bswap1_lz4;
typedef sqeazy::pipeline<rmbkg_diff_bswap1_lz4> rmbkg_diff_bswap1_lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::huffman_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_bswap1_huff_lz4;
typedef sqeazy::pipeline<rmbkg_bswap1_huff_lz4> rmbkg_bswap1_huff_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::huffman_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_huff_bswap1_lz4;
typedef sqeazy::pipeline<rmbkg_huff_bswap1_lz4> rmbkg_huff_bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_lz4;
typedef sqeazy::pipeline<rmbkg_lz4> rmbkg_lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_bswap1_lz4;
typedef sqeazy::pipeline<rmbkg_bswap1_lz4> rmbkg_bswap1_lz4_pipe;



typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > diff_bswap1_lz4;
typedef sqeazy::pipeline<diff_bswap1_lz4> diff_bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short, sqeazy::last_pixels_on_line_neighborhood<2> >,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > diffonrow_bswap1_lz4;
typedef sqeazy::pipeline<diffonrow_bswap1_lz4> diffonrow_bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::huffman_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > huff_bswap1_lz4;
typedef sqeazy::pipeline<huff_bswap1_lz4> huff_bswap1_lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > diff_lz4;
typedef sqeazy::pipeline<diff_lz4> diff_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short, sqeazy::last_pixels_on_line_neighborhood<2> >,
        sqeazy::lz4_scheme<unsigned short> > diffonrow_lz4;
typedef sqeazy::pipeline<diffonrow_lz4> diffonrow_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > bswap1_lz4;
typedef sqeazy::pipeline<bswap1_lz4> bswap1_lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::lz4_scheme<unsigned short> > lz4_;
typedef sqeazy::pipeline<lz4_> lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::huffman_scheme<unsigned short>, sqeazy::lz4_scheme<unsigned short> > huffman_lz4;
typedef sqeazy::pipeline<huffman_lz4> huff_lz4_pipe;


////////////////////////////////////////////////////////////////////////////////
// 8-bit

typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned char>,
        sqeazy::bitswap_scheme<unsigned char>,
        sqeazy::lz4_scheme<unsigned char> > char_rmbkg_bswap1_lz4;
typedef sqeazy::pipeline<char_rmbkg_bswap1_lz4> char_rmbkg_bswap1_lz4_pipe;


typedef sqeazy::bmpl::vector< sqeazy::bitswap_scheme<unsigned char>,
        sqeazy::lz4_scheme<unsigned char> > char_bswap1_lz4;
typedef sqeazy::pipeline<char_bswap1_lz4> char_bswap1_lz4_pipe;



#endif /* _BENCH_COMMON_H_ */
