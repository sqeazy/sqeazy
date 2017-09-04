#ifndef BASE64_H
#define BASE64_H

#include <cmath>
#include <string>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace sqeazy {

    namespace base64 {

        static const char codes[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" ;

        /**
         *  \brief returns the expected size of the data when encoded by base64
         *
         *  \param _nbytes size of the input in bytes
         *  \return std::size_t
         *  \retval number of bytes which the encoded data will consume
         */
        static std::size_t encoded_bytes(std::size_t _nbytes){
            return 4*std::ceil(_nbytes/3.);
        }

        /**
         *  \brief returns the expected size of the data when decoded by base64
         *
         *  \param _nbytes size of the encoded input in bytes
         *  \return std::size_t
         *  \retval number of bytes which the decoded data will consume
         */
        static std::size_t decoded_bytes(const char* _begin, const char* _end){

            std::size_t len = std::distance(_begin, _end);
            std::size_t n_paddings = 0;

            if(len > 3)
                n_paddings = std::count(_end - 4, _end,'=');

            std::size_t value = 3*len/4;
            return value - n_paddings;
        }


        // repeating the base64 characters in the index array avoids the  077 &  operations
        void byte3_to_char4(const unsigned char* input, char* output){

            output[ 0 ] =  codes[       input[0] >> 2                     ];
            output[ 1 ] =  codes[077 & ((input[0] << 4)| (input[1] >> 4)) ];
            output[ 2 ] =  codes[077 & ((input[1] << 2)| (input[2] >> 6)) ];
            output[ 3 ] =  codes[077 & input[2]                           ];

            return ;
        }


        /**
         *  \brief implementation of base64 encoding
         *
         *  using the boost serialisation archive iterators,
         *  function assumes that [_begin,_end) and that _dst has size len*4/3 where len = _end - _begin
         *
         *  \param _begin char* pointer to the begin of the input sequence
         *  \param _end char* pointer to the end+1 of the input sequence
         *  \param _dst char* pointer to the begin of the output sequence (it's assumed that the memory allocated satisfies encoded_bytes)
         *  \return char*
         *  \retval pointer to the last element plus 1 written to _dst
         */
        static char* encode_impl(const char* _begin, const char* _end, char* _dst){

            using namespace boost::archive::iterators;

            const std::size_t len = std::distance(_begin,_end);
            const std::size_t rest = len % 3;
            const std::size_t olen = encoded_bytes(len);

            using It = base64_from_binary<transform_width<const char*, 6, 8>>;

            char* out_iter = std::copy(It(_begin), It(_end), _dst);

            if(rest){
                std::fill(_dst+(olen)-(3-rest),_dst+olen,'=');
                out_iter = _dst+olen;
            }

            return out_iter;
        }

        /**
         *  \brief fast implementation of base64 encoding
         *
         *  using the implementation suggested on https://en.wikipedia.org/wiki/Base64#C_encoder_efficiency_con
         *  function assumes that [_begin,_end) and that _dst has size len*4/3 where len = _end - _begin
         *
         *  \param _begin char* pointer to the begin of the input sequence
         *  \param _end char* pointer to the end+1 of the input sequence
         *  \param _dst char* pointer to the begin of the output sequence (it's assumed that the memory allocated satisfies encoded_bytes)
         *  \return char*
         *  \retval pointer to the last element plus 1 written to _dst
         */

        static char* fast_encode_impl(const char* _begin, const char* _end, char* _dst){

            const std::size_t len = std::distance(_begin,_end);
            const std::size_t rest = len % 3;
            const std::size_t cropped_len = len - (rest);


            for(std::size_t i = 0; i < cropped_len;i+=3,_dst+=4,_begin+=3){
                byte3_to_char4((const unsigned char*)(_begin), _dst);
            }

            char* output_iter = _dst;

            if(cropped_len != len){
                std::string padded = "    ";
                byte3_to_char4((const unsigned char*)(_begin), &padded[0]);
                std::fill(padded.rbegin(),padded.rbegin()+(3-rest),'=');
                std::copy(padded.data(),padded.data()+padded.size(),_dst);
                output_iter += padded.size();
            }
            return output_iter;
        }


        /**
         *  \brief fast implementation of base64 decoding
         *
         *  using the  boost serialisation archive iterators,
         *  function assumes that [_begin,_end) and that _dst has size len*4/3 where len = _end - _begin
         *
         *  \param _begin char* pointer to the begin of the input sequence
         *  \param _end char* pointer to the end+1 of the input sequence
         *  \param _dst char* pointer to the begin of the output sequence (it's assumed that the memory allocated satisfies encoded_bytes)
         *  \return char*
         *  \retval pointer to the last element plus 1 written to _dst
         */
        static char* decode_impl(const char* _begin, const char* _end, char* _dst){

            using namespace boost::archive::iterators;

            using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

            // char* return_itr = std::copy(It(_begin), It(_end),_dst);

            auto ops = [](char c) {
                    return c == '\0';
            };

            std::string val = boost::algorithm::trim_right_copy_if(std::string(It(_begin), It(_end)), ops);

            char* return_itr = std::copy(val.begin(), val.end(),_dst);
            return return_itr;
        }

    }
}

#endif /* BASE64_H */
