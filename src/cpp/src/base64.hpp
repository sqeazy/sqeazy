#ifndef BASE64_H
#define BASE64_H

#include <cmath>
#include <string>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>

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

        // repeating the base64 characters in the index array avoids the  077 &  operations
        void padded_byte3_to_char4(const unsigned char* input, const unsigned char* input_end, char* output){

            std::size_t len = std::distance(input,input_end);

            if(len == 0){
                //unlikely, but possible ... doing nothing
                return;
            }

            if(len == 3){
                byte3_to_char4(input,output);
                return;
            }

            output[ 0 ] =  codes[       input[0] >> 2                     ];

            if(len <= 1){
                output[ 1 ] =  codes[077 & ((input[0] << 4)| (0 >> 4)) ];
                std::fill(output+2,output+4,'=');
                return;
            }

            output[ 1 ] =  codes[077 & ((input[0] << 4)| (input[1] >> 4)) ];
            output[ 2 ] =  codes[077 & ((input[1] << 2)| (0 >> 6)) ];
            std::fill(output+3,output+4,'=');
            return;

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

            if(rest > 0){

                padded_byte3_to_char4((const unsigned char*)(_begin),
                                    (const unsigned char*)(_end),
                                    _dst);


                // std::string input(3,(char)0);
                // std::copy(_begin,_end,input.begin());
                // byte3_to_char4((const unsigned char*)input.data(),_dst);

                output_iter += 4;
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

            using It = transform_width<binary_from_base64<const char*>, 8, 6>;

            const std::size_t n_decoded_bytes = decoded_bytes(_begin,_end);

            std::string temp(n_decoded_bytes+6, ' ');
            std::copy(It(_begin), It(_end),(char*)temp.data());

            // auto ops = [](char c) {
            //         return c == '\0';
            // };

            // auto seq =  std::string(It(_begin), It(_end)) ;
            // std::string val = boost::algorithm::trim_right_copy_if(seq, ops);

            // char* return_itr = std::copy(val.begin(), val.end(),_dst);

            //NOTE: boost tends to padd the output string by '\0' bytes upon decoding,
            //      as the main purpose of this is encoding arrays of integers/floats,
            //      they are likely to have \0 in them; therefor I must crop the output
            char* return_itr = std::copy(temp.data(),temp.data()+n_decoded_bytes,_dst);
            return return_itr;
        }

    }
}

#endif /* BASE64_H */
