#ifndef BASE64_H
#define BASE64_H

#include <cmath>
#include <string>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace sqeazy {

    static const char base64set[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" ;

    // repeating the base64 characters in the index array avoids the  077 &  operations
    void byte3_to_char4(const unsigned char* input, char* output){

        output[ 0 ] =  base64set[       input[0] >> 2                     ];
        output[ 1 ] =  base64set[077 & ((input[0] << 4)| (input[1] >> 4)) ];
        output[ 2 ] =  base64set[077 & ((input[1] << 2)| (input[2] >> 6)) ];
        output[ 3 ] =  base64set[077 & input[2]                           ];

        return ;
    }

    /*
      implementation of base64 encoding

      - function assumes that [_begin,_end) and that _dst has size len*4/3 where len = _end - _begin


    */
    static void base64_impl(const char* _begin, const char* _end, char* _dst){

        using namespace boost::archive::iterators;

        const std::size_t len = std::distance(_begin,_end);
        const std::size_t rest = len % 3;
        const std::size_t olen = 4*std::ceil(len/3.);

        using It = base64_from_binary<transform_width<const char*, 6, 8>>;

        std::copy(It(_begin), It(_end), _dst);

        if(rest)
            std::fill(_dst+(olen)-(3-rest),_dst+olen,'=');
    }

    /*
      implementation of base64 encoding

      - function assumes that [_begin,_end) and that _dst has size len*4/3 where len = _end - _begin


     */
    static void my_base64_impl(const char* _begin, const char* _end, char* _dst){

        const std::size_t len = std::distance(_begin,_end);
        const std::size_t rest = len % 3;
        const std::size_t cropped_len = len - (rest);

        for(std::size_t i = 0; i < cropped_len;i+=3,_dst+=4,_begin+=3){
            byte3_to_char4((const unsigned char*)(_begin), _dst);
        }

        if(cropped_len != len){
            std::string padded = "    ";
            byte3_to_char4((const unsigned char*)(_begin), &padded[0]);
            std::fill(padded.rbegin(),padded.rbegin()+(3-rest),'=');
            std::copy(padded.data(),padded.data()+padded.size(),_dst);
        }
        return ;
    }


    /*
      implementation of base64 deencoding

      - function assumes that [_begin,_end) and that _dst has size len*3/4 where len = _end - _begin


    */
    static void debase64_impl(const char* _begin, const char* _end, char* _dst){
        using namespace boost::archive::iterators;

        using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

        auto ops = [](char c) {
                return c == '\0';
        };

        //boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), ops);
        std::copy(It(_begin), It(_end),_dst);

        return ;
    }


}

#endif /* BASE64_H */
