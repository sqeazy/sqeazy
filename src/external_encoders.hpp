#ifndef _EXTERNAL_ENCODERS_HPP_
#define _EXTERNAL_ENCODERS_HPP_
#include <map>
#include <string>
#include <typeinfo>
#include "sqeazy.h"
#include "sqeazy_common.hpp"
#include "sqeazy_traits.hpp"

#ifndef LZ4_VERSION_MAJOR
#include "lz4.h"
#endif
namespace sqeazy {

template <char delimiter, typename T>
void split_string_to_vector(const std::string& _buffer, std::vector<T>& _v) {

    int num_del = std::count(_buffer.begin(), _buffer.end(), delimiter);
    if(_v.size() != num_del + 1)
        _v.resize(num_del+1);

    size_t begin = 0;
    size_t end = _buffer.find(delimiter);

    std::string token;

    for(int id = 0; id<_v.size(); ++id) {
        std::istringstream converter(_buffer.substr(begin,end - begin));
        converter >> _v[id];
        begin = end +1;
        end = _buffer.find(delimiter,begin);
    }

}

static unsigned sizeof_typeinfo(const std::string& _lit) {

    static std::map<std::string,int> type_size_map;
    type_size_map[typeid(short).name()] = sizeof(short);
    type_size_map[typeid(unsigned short).name()] = sizeof(unsigned short);
    type_size_map[typeid(char).name()] = sizeof(char);
    type_size_map[typeid(unsigned char).name()] = sizeof(unsigned char);
    type_size_map[typeid(int).name()] = sizeof(int);
    type_size_map[typeid(unsigned int).name()] = sizeof(unsigned int);
    type_size_map[typeid(long).name()] = sizeof(long);
    type_size_map[typeid(unsigned long).name()] = sizeof(unsigned long);

    std::map<std::string,int>::const_iterator found = type_size_map.find(_lit);
    if(found!=type_size_map.end())
        return found->second;
    else
        return 0;
}

template <typename T,char major_delim = ',', char minor_delim='x', char header_end_delim = '|'>
struct image_header {

    std::string header;
    std::vector<unsigned> dims;

    template <typename S>
    image_header(const std::vector<S>& _dims):
        header(pack(_dims)),
        dims(_dims.begin(), _dims.end()) {


    }


    image_header(const std::string& _str):
        header(),
        dims() {

        size_t header_end = _str.find(header_end_delim);
        header = _str.substr(0,header_end+1);
        dims = unpack(header);
    }

    image_header(const char* _begin, const char* _end):
        header(_begin,_end),
        dims() {

        size_t header_end = header.find(header_end_delim);
        header = header.substr(0,header_end+1);
        dims = unpack(header);


    }

    unsigned size() const {

        return header.size();
    }

    std::string str() const {

        return header;

    }

    unsigned long payload_size() const {

        unsigned long value = std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<unsigned>());
        return value;
    }

    unsigned long payload_size_byte() const {

        unsigned long size_of_type = sizeof(T);
        if(!header.empty()) {
            std::string typeinfo_literal = header.substr(0,header.find(major_delim));
            unsigned found_size_byte = sizeof_typeinfo(typeinfo_literal);
            if(found_size_byte && found_size_byte!=size_of_type)
                size_of_type = found_size_byte;
        }
        return payload_size()*size_of_type;
    }

    template <typename vtype>
    static const std::string pack(const std::vector<vtype>& _dims) {

        std::stringstream header("");
        header << typeid(T).name() << ","
               << _dims.size() << ",";
        for(unsigned i = 0; i<_dims.size(); ++i) {
            if(i!=(_dims.size()-1))
                header << _dims[i] << "x";
            else
                header << _dims[i];
        }

        header << header_end_delim;
        return header.str();
    }

    static const std::vector<unsigned> unpack(const std::string& _buffer) {

        return unpack(&_buffer[0], _buffer.size());
    }

    static const std::vector<unsigned> unpack(const char* _buffer, const unsigned& _size) {

        std::vector<unsigned> value;

        std::string in_buffer(_buffer, _buffer+_size);

        size_t header_end = in_buffer.find(header_end_delim);
        size_t last_comma = in_buffer.rfind(major_delim,header_end);
        size_t begin = 0;
        if(last_comma!=std::string::npos)
            begin = last_comma+1;

        std::string dim_fields(in_buffer,begin,header_end-begin);

        split_string_to_vector<minor_delim>(dim_fields, value);
        return value;
    }

    static const int unpack_num_dims(const char* _buffer, const unsigned& _size) {

        std::string in_buffer(_buffer, _buffer+_size);

        size_t dims_begin = in_buffer.find(major_delim)+1;
        size_t dims_end = in_buffer.find(major_delim,dims_begin+1);

        std::istringstream conv(in_buffer.substr(dims_begin, dims_end - dims_begin));

        int value = 0;
        conv >> value;
        return value;
    }
};

template < typename T , typename S = unsigned long>
struct lz4_scheme {

    typedef T raw_type;
    typedef char compressed_type;
    typedef S size_type;

    static S last_num_encoded_bytes;

    static const bool is_compressor = true;

    static const std::string name() {

        return std::string("lz4");

    }





    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    template <typename SizeType>
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   std::vector<SizeType>& _dims,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {

        typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
        local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
        local_size_type total_length_in_byte = total_length*sizeof(raw_type);
	local_size_type max_payload_length_in_byte = LZ4_compressBound(total_length_in_byte);

        const compressed_type* input = reinterpret_cast<const compressed_type*>(_input);

        std::string header = image_header<raw_type>::pack(_dims);

        std::copy(header.begin(),header.end(),_output);

        compressed_type* output_begin = &_output[header.size()];
        size_type num_written_bytes = LZ4_compress_limitedOutput(input,output_begin,
                                total_length_in_byte,
                                max_payload_length_in_byte);

        if(num_written_bytes) {
            _bytes_written = num_written_bytes+header.size();
            last_num_encoded_bytes = _bytes_written;
            return SUCCESS;
        }
        else {
            _bytes_written = 0;
            last_num_encoded_bytes = 0;
            return FAILURE;
        }

    }


    /**
     * @brief encode _input using the lz4 library, the output contains the number of input elements encoded as 64bit signed integer (long) and the encoded data
     *
     * @param _input array/buffer that contains data of raw_type
     * @param _output the char buffer that contains the compressed data (not truncated, not owned)
     * @param _length size of _input (unmutable)
     * @param _bytes_written number of bytes/elements in _output that actually contain data
     * @return sqeazy::error_code SUCCESS if non-zero number of bytes have been encoded by lz4
     */
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   const size_type& _length,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {

        std::vector<size_type> artificial_dims(1);
        artificial_dims[0] = _length;

        return encode(_input, _output, artificial_dims, _bytes_written);

    }

    /**
     * @brief decode the input data stream _input with lz4 (the number of output elements is extracted from the first 64
     * bit of the input stream)
     *
     * @param _input input buffer as char
     * @param _output output buffer (needs to be allocated outside this function)
     * @param _length number of elements in _input
     * @return sqeazy::error_code
     */
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const size_type& _length
                                  ) {

        size_t delimiter_pos = std::find(_input, _input+_length,'|') - _input;
        if((delimiter_pos) >= _length)
            return FAILURE;

        image_header<raw_type> found_header(_input, _input + delimiter_pos + 1);

        long maxOutputSize = found_header.payload_size_byte();

        compressed_type* output = reinterpret_cast<compressed_type*>(_output);
        const compressed_type* begin_payload = _input + found_header.size();
        int num_bytes_decoded = LZ4_decompress_safe(begin_payload,output,_length - found_header.size(), maxOutputSize);

        return num_bytes_decoded == maxOutputSize ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long header_size(const std::vector<U>& _in) {

        image_header<raw_type> value(_in);
        return value.header.size();

    }

    template <typename U>
    static const unsigned long max_encoded_size(U _src_length_in_bytes) {

        long lz4_bound = LZ4_compressBound(_src_length_in_bytes);

        std::vector<unsigned> any(1,_src_length_in_bytes);
        return lz4_bound + header_size(any);

    }

    template <typename U>
    static const unsigned long max_encoded_size(const std::vector<U>& _src_dims) {

        image_header<raw_type> artificial_header(_src_dims);
        long lz4_bound = LZ4_compressBound(artificial_header.payload_size());

        return lz4_bound + artificial_header.size();

    }

    template <typename U>
    static const unsigned long decoded_size_byte(const compressed_type* _buf, const U& _size) {

        size_t delimiter_pos = std::find(_buf, _buf+_size,'|') - _buf;
        if((delimiter_pos) >= _size)
            return 0;

        image_header<raw_type> found_header(_buf, _buf + delimiter_pos+1);

        return found_header.payload_size_byte();

    }

    template <typename U>
    static const std::vector<unsigned> decode_dimensions(const compressed_type* _buf, const U& _size) {

        std::vector<unsigned> dims;

        size_t delimiter_pos = std::find(_buf, _buf+_size,'|') - _buf;
        if((delimiter_pos) >= _size)
            return dims;
        dims = image_header<raw_type>::unpack(_buf, delimiter_pos+1);


        return dims;

    }

    template <typename U>
    static const int decoded_num_dims(const compressed_type* _buf, const U& _size) {
        size_t delimiter_pos = std::find(_buf, _buf+_size,'|') - _buf;
        if((delimiter_pos) >= _size)
            return 0;
        return image_header<raw_type>::unpack_num_dims(_buf, delimiter_pos+1);


    }
};

template < typename T , typename S>
S lz4_scheme<T,S>::last_num_encoded_bytes = 0;


};

#endif /* _EXTERNAL_ENCODERS_H_ */
