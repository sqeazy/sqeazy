#ifndef LZ4_UTILS_H
#define LZ4_UTILS_H

#include "sqeazy_common.hpp"
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
                                        compressed_type* _out_end,
                                        std::size_t _framestep_byte,
                                        LZ4F_preferences_t& _lz4prefs
            )  {



            std::size_t num_written_bytes = 0;
            const std::size_t len = std::distance(_in,_in_end);
            const std::size_t bytes = len*sizeof(compressed_type);
            const local_size_type out_bytes = std::distance(_out, _out_end);

            compressed_type* dst = _out;
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
                                                           dst,
                                                           out_bytes ,
                                                           &_lz4prefs);
            if (LZ4F_isError(rcode)) {
                std::cerr << "[sqy::lz4] Failed to start compression: error " << rcode << "\n";
                return value;
            }

            std::size_t n_steps = (bytes + _framestep_byte - 1) / _framestep_byte;

            auto src = _in;
            auto srcEnd = _in_end;
            dst += num_written_bytes;

            for( std::size_t s = 0;s<n_steps;++s){

                auto src_size = (local_size_type)std::distance(src,srcEnd) < _framestep_byte ? std::distance(src,srcEnd) : _framestep_byte;
                auto n = LZ4F_compressUpdate(ctx,
                                             dst,
                                             out_bytes-num_written_bytes,
                                             src,
                                             src_size,
                                             nullptr);

                src += src_size;
                num_written_bytes += n;
                dst += n;

            }

            rcode = LZ4F_compressEnd(ctx, dst, out_bytes- num_written_bytes, NULL);
            if (LZ4F_isError(rcode)) {
                std::cerr << "[sqy::lz4] Failed to end compression: error " << rcode << "\n";
                return value;
            }

            num_written_bytes += rcode;
            value = _out + num_written_bytes;

            LZ4F_freeCompressionContext(ctx);

            return value;
        }

        template <typename T, typename size_type>
        T* remove_blanks(T* _payload, const std::vector<size_type> items_written, size_type chunk_len){

            auto value = _payload + items_written.front();
            T* src_out = _payload + chunk_len;

            for(std::size_t i = 1;i < items_written.size();++i){

                value = std::copy(src_out,src_out+items_written[i],value);
                src_out += chunk_len;

            }

            return value;

        }


        template <typename compressed_type>
        compressed_type* encode_parallel(const compressed_type* _in,
                                         const compressed_type* _in_end,
                                         compressed_type* _out,
                                         compressed_type* _out_end,
                                         std::size_t _nbytes_inputchunk,
                                         LZ4F_preferences_t& _lz4prefs,
                                         int nthreads)  {

            const std::size_t len = std::distance(_in,_in_end);
            const std::size_t bytes = len*sizeof(compressed_type);

            compressed_type* value = nullptr;
            //_lz4prefs.frameInfo.contentSize = 0;

            const std::size_t maxbytes_encoded_chunk = LZ4F_compressBound(_nbytes_inputchunk, &_lz4prefs);
            const std::size_t nchunks = (bytes + _nbytes_inputchunk - 1) / _nbytes_inputchunk;

            if(nchunks==1){

                return encode_serial(_in,_in_end,
                                     _out,_out_end,
                                     _nbytes_inputchunk,
                                     _lz4prefs
                    );

            }

            std::vector<std::size_t> nbytes_written(nchunks,0);
            if(std::uint32_t(nthreads) > nchunks)
                nthreads = nchunks;

            auto nbytes_ptr = nbytes_written.data();
            auto pref_ptr = &_lz4prefs;

#pragma omp parallel num_threads(nthreads) firstprivate(nbytes_ptr,pref_ptr,_in,_in_end,nthreads,maxbytes_encoded_chunk,_out,_out_end)
            {
                int thread_id = omp_get_thread_num();
                LZ4F_preferences_t local_prefs = *pref_ptr;
                const compressed_type* t_in = nullptr;
                const compressed_type* t_in_end = nullptr;
                compressed_type* t_out = nullptr;
                const std::size_t nrounds = (nchunks + nthreads - 1 )/nthreads;

                for(std::size_t cnt = 0;cnt < nrounds;++cnt){

                    t_in = _in + (thread_id + (cnt*nthreads))*_nbytes_inputchunk;
                    t_in_end = (t_in + _nbytes_inputchunk) > _in_end ? _in_end : t_in + _nbytes_inputchunk;

                    if(t_in>=_in_end)
                        break;

                    t_out = _out + (thread_id + (cnt*nthreads))*maxbytes_encoded_chunk;
                    auto t_out_end = (t_out + maxbytes_encoded_chunk) > _out_end ? _out_end : t_out + maxbytes_encoded_chunk;

                    auto val = encode_serial(t_in,
                                             t_in_end,
                                             t_out,
                                             t_out_end,
                                             _nbytes_inputchunk,
                                             local_prefs
                        );

                    nbytes_ptr[thread_id + (cnt*nthreads)] = val != nullptr ? std::distance(t_out,val) : 0;
                }
            }

            //shrink the output buffer to discard the bytes that have not been filled with encoded data

            std::size_t total_bytes_written = std::accumulate(nbytes_written.begin(), nbytes_written.end(), 0);
            value = remove_blanks(_out,nbytes_written,maxbytes_encoded_chunk);
            // value = _out + nbytes_written.front();
            // compressed_type* src_out = _out + maxbytes_encoded_chunk;

            // for(std::size_t i = 1;i < nbytes_written.size();++i){

            //     value = std::copy(src_out,src_out+nbytes_written[i],value);
            //     src_out += maxbytes_encoded_chunk;

            // }

            return (value == (_out+total_bytes_written)) ? value : nullptr;
        }


    }  // lz4
}


constexpr std::array<std::size_t,4> sqeazy::lz4::closest_blocksize::sizes;
constexpr std::array<sqeazy::lz4::closest_blocksize::lz4f_blocksize_t,4> sqeazy::lz4::closest_blocksize::vals;



#endif /* LZ4_UTILS_H */
