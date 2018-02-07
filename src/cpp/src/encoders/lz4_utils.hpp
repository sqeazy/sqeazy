#ifndef LZ4_UTILS_H
#define LZ4_UTILS_H

#include "traits.hpp"

#ifndef LZ4_VERSION_MAJOR
#include "lz4_utils.hpp"
#include "lz4frame.h"
#endif

#include <vector>
#include <array>
#include <cstdint>
#include <algorithm>
#include <iostream>

namespace sqeazy {

    namespace lz4 {

        typedef typename sqeazy::twice_as_wide<std::size_t>::type local_size_type;


        struct closest_blocksize {

            typedef decltype(LZ4F_max64KB) lz4f_blocksize_t;

            static constexpr std::array<std::size_t,4> sizes = {64 , 256 , 1024 , 4096 };
            static constexpr std::array<lz4f_blocksize_t,4> vals = {LZ4F_max64KB , LZ4F_max256KB, LZ4F_max1MB, LZ4F_max4MB };

            static lz4f_blocksize_t of(std::size_t _blocksize_in_kb){

                auto itr = std::lower_bound(sizes.begin(),sizes.end(),_blocksize_in_kb);
                if(itr == sizes.end())
                    return vals.back();
                else{

                    if(itr == sizes.begin())
                        return vals.front();

                    std::size_t upper_idx = std::distance(sizes.begin(),itr);
                    std::size_t lower_idx = std::distance(sizes.begin(),itr)-1;

                    auto middle = sizes[lower_idx]+((sizes[upper_idx]-sizes[lower_idx])/2);

                    if(_blocksize_in_kb >= middle)
                        return vals[upper_idx];
                    else{
                        return vals[lower_idx];

                    }

                }
            }


        };





        template <typename compressed_type>
        compressed_type* encode_serial( const compressed_type* _in,
                                        const compressed_type* _in_end,
                                        compressed_type* _out,
                                        std::size_t _framestep_byte,
                                        LZ4F_preferences_t& _lz4prefs
            )  {



            std::size_t num_written_bytes = 0;
            const std::size_t len = std::distance(_in,_in_end);
            const std::size_t bytes = len*sizeof(compressed_type);
            const local_size_type out_bytes = max_encoded_size(bytes);
            const local_size_type max_framestep_in_byte = max_encoded_size(_framestep_byte );

            compressed_type* value = nullptr;

            LZ4F_compressionContext_t ctx;
            auto rcode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
            if (LZ4F_isError(rcode)) {
                std::cerr << "[sqy::lz4] Failed to create context: error " << rcode << "\n";
                return value;
            } else {
                rcode = 1;//don't ask me why, taken from https://github.com/lz4/lz4/blob/v1.8.0/examples/frameCompress.c#L34
            }

            rcode = num_written_bytes = LZ4F_compressBegin(ctx,
                                                           _out,
                                                           max_framestep_in_byte ,
                                                           &lz4_prefs);
            if (LZ4F_isError(rcode)) {
                std::cerr << "[sqy::lz4] Failed to start compression: error " << rcode << "\n";
                return value;
            }

            std::size_t n_steps = (bytes + _framestep_byte - 1) / _framestep_byte;

            auto dst = _out + num_written_bytes;
            auto src = _in;
            auto srcEnd = _in_end;

            for( std::size_t s = 0;s<n_steps;++s){

                auto n = LZ4F_compressUpdate(ctx,
                                             dst,
                                             out_bytes - std::distance(_out,dst),
                                             src,
                                             (local_size_type)std::distance(src,srcEnd) < _framestep_byte ? std::distance(src,srcEnd) : _framestep_byte, nullptr);

                src += _framestep_byte;
                dst += n;

            }

            rcode = LZ4F_compressEnd(ctx, dst, out_bytes - std::distance(_out,dst), NULL);
            if (LZ4F_isError(rcode)) {
                std::cerr << "[sqy::lz4] Failed to end compression: error " << rcode << "\n";
                return value;
            }

            dst += rcode;


            if (ctx)
                LZ4F_freeCompressionContext(ctx);

            value = dst;

            return value;
        }

        template <typename compressed_type>
        compressed_type* encode_parallel(const compressed_type* _in,
                                         const compressed_type* _in_end,
                                         compressed_type* _out,
                                         std::size_t _nbytes_inputchunk,
                                         const LZ4F_preferences_t& _lz4prefs,
                                         int nthreads)  {

            const std::size_t len = std::distance(_in,_in_end);
            const std::size_t bytes = len*sizeof(compressed_type);

            compressed_type* value = nullptr;
            lz4_prefs.frameInfo.contentSize = bytes;

            const std::size_t maxbytes_encoded_chunk = max_compressed_size(_nbytes_inputchunk);
            const std::size_t nchunks = (bytes + _nbytes_inputchunk - 1) / _nbytes_inputchunk;

            std::vector<std::size_t> nbytes_written(nchunks,0);
            if(nthreads > nchunks)
                nthreads = nchunks;

            auto nbytes_ptr = nbytes_written.data();
            auto pref_ptr = &_lz4prefs;

#pragma omp parallel num_threads(nthreads) firstprivate(nbytes_ptr,pref_ptr,_in,_in_end,nthreads)
            {
                int thread_id = omp_get_thread_num();
                LZ4F_preferences_t local_prefs = *pref_ptr;

                const compressed_type* t_in = nullptr;
                const compressed_type* t_in_end = nullptr;
                compressed_type* t_out = nullptr;

                for(std::size_t cnt = 0;t_in < _in_end;++cnt){

                    t_in = _in + (thread_id + (cnt*nthreads))*_nbytes_inputchunk;
                    t_in_end = t_in + _nbytes_inputchunk;
                    t_out = _out + (thread_id + (cnt*nthreads))*maxbytes_encoded_chunk;

                    auto val = encode_serial(t_in,t_in_end,
                                             t_out,
                                             _nbytes_inputchunk,
                                             local_prefs
                        );

                    nbytes_ptr[thread_id + (cnt*nthreads)] = std::distance(t_out,val);
                }
            }

            //shrink the output buffer to discard the bytes that have not been filled with encoded data
            compressed_type* dst_out = _out + nbytes_written.front();
            compressed_type* src_out = _out + maxbytes_encoded_chunk;

            for(std::size_t i = 1;i < nbytes_written.size();++i){

                dst_out = std::copy(src_out,src_out+nbytes_written[i],dst_out);
                src_out += maxbytes_encoded_chunk;

            }

            return dst_out;
        }


    }  // lz4
}


constexpr std::array<std::size_t,4> sqeazy::lz4::closest_blocksize::sizes;
constexpr std::array<sqeazy::lz4::closest_blocksize::lz4f_blocksize_t,4> sqeazy::lz4::closest_blocksize::vals;



#endif /* LZ4_UTILS_H */