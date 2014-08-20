#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_

#include <algorithm>
#include <sstream>
#include <climits>
#include "sqeazy_common.hpp"
#include "sqeazy_traits.hpp"
#include "diff_scheme_utils.hpp"
#include "bitswap_scheme_utils.hpp"
#include "hist_impl.hpp"
#include "huffman_utils.hpp"

namespace sqeazy {



template <typename T, typename Neighborhood = last_plane_neighborhood<3> >
struct diff_scheme {

    typedef T raw_type;
    typedef typename remove_unsigned<T>::type compressed_type;

    typedef typename add_unsigned<typename twice_as_wide<T>::type >::type sum_type;
    static const bool is_compressor = false;

    static const std::string name() {

        //TODO: add name of Neighborhood
      std::ostringstream msg;
      msg << "diff" 
	  << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
	  << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x" 
	  << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ; 
        return msg.str();

    }


    template <typename size_type>
    static const error_code encode(const size_type& _width,
                                   const size_type& _height,
                                   const size_type& _depth,
                                   const raw_type* _input,
                                   compressed_type* _output)
    {

        unsigned long length = _width*_height*_depth;
        std::copy(_input, _input + length, _output);//crossing fingers due to possible type mismatch

        std::vector<size_type> offsets;
        sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
        geometry.compute_offsets_in_x(offsets);
        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

        const size_type end_ = geometry.non_halo_end(0)-1;
        sum_type local_sum = 0;

        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
            for(unsigned long index = 0; index < end_; ++index) {

                const size_type local_index = index + *offsetsItr;
                local_sum = naive_sum<Neighborhood>(_input,local_index,_width, _height,_depth);
                _output[local_index] = _input[local_index] - local_sum/Neighborhood::traversed;

            }
        }

        return SUCCESS;
    }


    template <typename size_type>
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   size_type& _dim
                                  )
    {

        return encode(_dim, 1, 1, _input, _output);
    }

    template <typename size_type>
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   std::vector<size_type>& _dims
                                  )
    {
        return encode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);
    }

    template <typename size_type>
    static const error_code decode(const size_type& _width,
                                   const size_type& _height,
                                   const size_type& _depth,
                                   const compressed_type* _input,
                                   raw_type* _output)
    {
        unsigned long length = _width*_height*_depth;
        std::copy(_input,_input + length, _output);

        std::vector<size_type> offsets;
        sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
        geometry.compute_offsets_in_x(offsets);
        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

        const size_type end_ = geometry.non_halo_end(0)-1;
        sum_type local_sum = 0;

        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
            for(unsigned long index = 0; index < end_; ++index) {

                const size_type local_index = index + *offsetsItr;
                local_sum = naive_sum<Neighborhood>(_output,local_index,_width, _height,_depth);
                _output[local_index] = _input[local_index] + local_sum/Neighborhood::traversed;

            }
        }

        return SUCCESS;
    }


    template <typename size_type>
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   std::vector<size_type>& _dims
                                  ) {

        return decode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);

    }

    template <typename size_type>
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   size_type& _dim
                                  ) {
      
        const size_type one = 1;
        return decode(_dim, one, one, _input, _output);

    }

};


template < typename T, const unsigned raw_type_num_bits_per_segment = 1  >
struct bitswap_scheme {

    typedef T raw_type;
    typedef T compressed_type;
    typedef unsigned size_type;
    static const bool is_compressor = false;

    static const unsigned raw_type_num_bits = sizeof(T)*CHAR_BIT;
    static const unsigned num_segments = raw_type_num_bits/raw_type_num_bits_per_segment;
//     static const unsigned raw_type_num_bits_per_segment = raw_type_num_bits/num_segments;

    static const std::string name() {

        std::ostringstream val("");
        val << "bswap" << raw_type_num_bits_per_segment;
        return val.str();

    }

    template <typename S>
    static const error_code encode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<S>& _length)
    {
        typename sqeazy::twice_as_wide<S>::type total_length = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<S>());
        return encode(_input, _output, total_length);
    }

    static const error_code encode(const raw_type* _input,
                                   raw_type* _output,
                                   const size_type& _length)
    {

        const unsigned segment_length = _length/num_segments;
        const raw_type mask = ~(~0 << (raw_type_num_bits_per_segment));

        for(size_type seg_index = 0; seg_index<num_segments; ++seg_index) {

            size_type input_bit_offset = seg_index*raw_type_num_bits_per_segment;

            for(size_type index = 0; index < _length; ++index) {

                raw_type value = rotate_left<1>(xor_if_signed(_input[index]));
                raw_type extracted_bits = (value >> input_bit_offset) & mask;
                size_type output_bit_offset = ((index % num_segments)*raw_type_num_bits_per_segment);
                size_type output_index = ((num_segments-1-seg_index)*segment_length) + (index/num_segments);

                _output[output_index] = setbits_of_integertype(_output[output_index],extracted_bits,
                                        output_bit_offset,
                                        raw_type_num_bits_per_segment);
            }
        }

        return SUCCESS;
    }

    template <typename S>
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<S>& _length)
    {
        typename sqeazy::twice_as_wide<S>::type total_length = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<S>());
        return decode(_input, _output, total_length);
    }

    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const size_type& _length)
    {

        const unsigned segment_length = _length/num_segments;
        const raw_type mask = ~(~0 << (raw_type_num_bits_per_segment));

        for(size_type seg_index = 0; seg_index<num_segments; ++seg_index) {
            for(size_type index = 0; index < _length; ++index) {

                size_type input_bit_offset = (index % num_segments)*raw_type_num_bits_per_segment;

                size_type input_index = ((num_segments-1-seg_index)*segment_length) + index/num_segments;
                raw_type extracted_bits = (_input[input_index] >> input_bit_offset) & mask;

                size_type output_bit_offset = seg_index*raw_type_num_bits_per_segment;



                _output[index] = setbits_of_integertype(_output[index],extracted_bits,
                                                        output_bit_offset,raw_type_num_bits_per_segment);
            }
        }

        for(size_type index = 0; index < _length; ++index) {
            _output[index] = xor_if_signed(rotate_right<1>(_output[index]));
        }

        return SUCCESS;
    }


};


template < typename T, typename S = long >
struct remove_background {

    typedef T raw_type;
    typedef T compressed_type;
    typedef S size_type;
    static const bool is_compressor = false;

    static const std::string name() {


        return std::string("rmbkrd");

    }



    static const error_code encode(raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<size_type>& _data)
    {

        if(_data.size() == 2)
            return encode(_input, _output, _data.at(0), _data.at(1));
        else
            return FAILURE;

    }

    static const error_code encode(raw_type* _input,
                                   raw_type* _output,
                                   const size_type& _length,
                                   const raw_type& _epsilon)
    {
        histogram<raw_type> incoming(_input, _length);
        raw_type threshold = incoming.mode() + _epsilon;

        if(_output)
            return encode_out_of_place(_input, _output, _length, threshold);
        else
            return encode_inplace(_input, _length, threshold);

    }



    static const error_code encode_out_of_place(raw_type* _input,
            raw_type* _output,
            const size_type& _length,
            const raw_type& _threshold)
    {

        for(unsigned long vox = 0; vox<_length; ++vox) {
            _output[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
        }

        return SUCCESS;
    }



    static const error_code encode_inplace(raw_type* _input,
                                           const size_type& _length,
                                           const raw_type& _threshold)
    {

        for(unsigned long vox = 0; vox<_length; ++vox) {
            _input[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
        }


        return SUCCESS;
    }

    
    template <typename SizeType>
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
	std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }
    
    template <typename SizeType>
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<SizeType>& _length)
    {
	unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());
	
        return decode(_input, _output, total_size);
    }

};


template < typename T >
struct remove_estimated_background {

    typedef T raw_type;
    typedef T compressed_type;
    static const bool is_compressor = false;

    static const std::string name() {


        return std::string("rmestbkrd");

    }

    template <typename size_type>
    static const void extract_darkest_face(const raw_type* _input,
                                           const std::vector<size_type>& _dims,
                                           std::vector<raw_type>& _darkest_face) {

        typedef typename add_unsigned<typename twice_as_wide<size_type>::type >::type index_type;

        raw_type mean = std::numeric_limits<raw_type>::max();
        index_type input_index =0;
        const index_type frame_size = _dims[1]*_dims[0];
        index_type face_index =0;


        //faces with z
        for(size_type z_idx = 0; z_idx < _dims[2]; z_idx+=(_dims[2]-1)) {
            input_index = z_idx*(frame_size);
            const raw_type* begin = _input + input_index;
            raw_type temp = std::accumulate(begin, begin + (frame_size),0 )/(frame_size);
            if(temp < mean) {
                if(_darkest_face.size()<frame_size)
                    _darkest_face.resize(frame_size);
                std::copy(begin, begin + (frame_size),_darkest_face.begin());
                mean = temp;
            }
        }

        //faces with y
        face_index =0;
        for(size_type y_idx = 0; y_idx < _dims[1]; y_idx+=(_dims[1]-1)) {
            raw_type temp = 0;
            for(size_type z_idx = 0; z_idx < _dims[2]; ++z_idx) {
                for(size_type x_idx = 0; x_idx < _dims[0]; ++x_idx) {
                    input_index = z_idx*(frame_size)+y_idx*_dims[0]+x_idx;
                    temp += _input[input_index];
                }
            }
            temp/=_dims[2]*_dims[0];
            if(temp < mean) {
                if(_darkest_face.size()<_dims[2]*_dims[0])
                    _darkest_face.resize(_dims[2]*_dims[0]);
                for(size_type z_idx = 0; z_idx < _dims[2]; ++z_idx) {
                    for(size_type x_idx = 0; x_idx < _dims[0]; ++x_idx) {
                        input_index = z_idx*(frame_size)+y_idx*_dims[0]+x_idx;
                        _darkest_face[face_index++] = _input[input_index];
                    }
                }
                mean = temp;
            }
        }

        //faces with x
        face_index =0;
        for(size_type x_idx = 0; x_idx < _dims[0]; x_idx+=(_dims[0]-1)) {
            raw_type temp = 0;
            for(size_type z_idx = 0; z_idx < _dims[2]; ++z_idx) {
                for(size_type y_idx = 0; y_idx < _dims[1]; ++y_idx) {
                    input_index = z_idx*(frame_size)+y_idx*_dims[0]+x_idx;
                    temp += _input[input_index];
                }
            }
            temp/=_dims[2]*_dims[1];
            if(temp < mean) {
                if(_darkest_face.size()<_dims[2]*_dims[1])
                    _darkest_face.resize(_dims[2]*_dims[1]);
                for(size_type z_idx = 0; z_idx < _dims[2]; ++z_idx) {
                    for(size_type y_idx = 0; y_idx < _dims[0]; ++y_idx) {
                        input_index = z_idx*(frame_size)+y_idx*_dims[0]+x_idx;
                        _darkest_face[face_index++] = _input[input_index];
                    }
                }
                mean = temp;
            }
        }
    }

    template <typename size_type>
    static const error_code encode(raw_type* _input,
                                   compressed_type* _output,
                                   const std::vector<size_type>& _dims)
    {


        std::vector<raw_type> darkest_face;
        extract_darkest_face((const raw_type*)_input, _dims, darkest_face);
        histogram<raw_type> h_darkest_facet(darkest_face.begin(), darkest_face.end());
        const raw_type median_deviation = mpicbg_median_variation(darkest_face.begin(),darkest_face.end());
        const raw_type median = h_darkest_facet.median();
        const float alpha = 1.f;
        size_type input_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<size_type>());

        if(_output)
            remove_background<raw_type>::encode_out_of_place(_input, _output, input_length, median+(alpha*median_deviation));
        else
            remove_background<raw_type>::encode_inplace(_input, input_length, median+(alpha*median_deviation));

        return SUCCESS;
    }

    template <typename SizeType>
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
	std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }
    
    template <typename SizeType>
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const std::vector<SizeType>& _length)
    {
	unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());
	
        return decode(_input, _output, total_size);
    }
    
    
};
} //sqeazy

#endif /* _SQEAZY_IMPL_H_ */
