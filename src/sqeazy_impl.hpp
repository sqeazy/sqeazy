#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_
#include <functional>
#include <algorithm>
#include <sstream>
#include <climits>
#include "sqeazy_common.hpp"
#include "sqeazy_traits.hpp"
#include "neighborhood_utils.hpp"
#include "background_scheme_utils.hpp"
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

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
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
    /**
     * @brief encoding the diff scheme, i.e. the output value for input intensity I is equal to the sum 
     * of the neighborhood divided by the number of pixels traversed in the neighborhood
     * 
     * @param _width width of input stack
     * @param _height height of the input stack
     * @param _depth depth of the input stack
     * @param _input input stack of type raw_type
     * @param _output output stack of type compressed_type but same extent than the input
     * @return sqeazy::error_code
     */
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


        size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
        if(offsets.size()==1)//no offsets in other dimensions than x
        {
            halo_size_x = length - offsets[0];
        }
        sum_type local_sum = 0;
        size_type local_index = 0;
        const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();
        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
            for(unsigned long index = 0; index < halo_size_x; ++index) {

                local_index = index + *offsetsItr;
                local_sum = naive_sum<Neighborhood>(_input,local_index,_width, _height,_depth);
                _output[local_index] = _input[local_index] - local_sum/n_traversed_pixels;

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
    /**
     * @brief reconstructing data that was encoded by this diff scheme
     * 
     * @return sqeazy::error_code
     */
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

        size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
        if(offsets.size()==1)//no offsets in other dimensions than x
        {
            halo_size_x = length - offsets[0];
        }
        sum_type local_sum = 0;
        const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
            for(unsigned long index = 0; index < halo_size_x; ++index) {

                const size_type local_index = index + *offsetsItr;
                local_sum = naive_sum<Neighborhood>(_output,local_index,_width, _height,_depth);
                _output[local_index] = _input[local_index] + local_sum/n_traversed_pixels;

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

    /**
         * @brief producing the name of this scheme and return it as a string
         *
         * @return const std::string
         */
    static const std::string name() {

        std::ostringstream val("");
        val << "bswap" << raw_type_num_bits_per_segment;
        return val.str();

    }

    template <typename S>
    /**
     * @brief performs bitplane encoding, every bit of any value from _input is written to the respective 
     * compartment in _output, the granularity of the bit decomposition is given by raw_type_num_bits_per_segment
     * that means if raw_type_num_bits_per_segment = 1 and the input type is 8-bit wide then the output buffer is 
     * divided in 8 compartments in which the values of the nth bit of every input value are written to
     * 
     * @param _input 3D input stack
     * @param _output 3D output stack of same type and extent as _input
     * @param _length vector of extents
     * @return sqeazy::error_code
     */
    static const error_code encode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<S>& _length)
    {
        typename sqeazy::twice_as_wide<S>::type total_length = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<S>());
        return encode(_input, _output, total_length);
    }

    
    /**
     * @brief performs bitplane encoding, every bit of any value from _input is written to the respective 
     * compartment in _output, the granularity of the bit decomposition is given by raw_type_num_bits_per_segment
     * that means if raw_type_num_bits_per_segment = 1 and the input type is 8-bit wide then the output buffer is 
     * divided in 8 compartments in which the values of the nth bit of every input value are written to
     * 
     * @param _input 3D input stack
     * @param _output 3D output stack of same type and extent as _input
     * @param _length as a scalar
     * @return sqeazy::error_code
     */
    static const error_code encode(const raw_type* _input,
                                   raw_type* _output,
                                   const size_type& _length)
    {

        const unsigned segment_length = _length/num_segments;
        const raw_type mask = ~(~0 << (raw_type_num_bits_per_segment));
        raw_type value = 0;

        for(size_type seg_index = 0; seg_index<num_segments; ++seg_index) {

            size_type input_bit_offset = seg_index*raw_type_num_bits_per_segment;

            for(size_type index = 0; index < _length; ++index) {

                value = xor_if_signed(_input[index]);
                value = rotate_left<1>(value);
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

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string name() {


        return std::string("rmbkrd");

    }


/**
     * @brief [length given as vector] removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _output 3D output stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
    static const error_code encode(raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<size_type>& _data,
				   const raw_type& _threshold
				  )
    {

        unsigned long length = std::accumulate(_data.begin(), _data.end(), 1, std::multiplies<size_type>());

        return encode(_input, _output, length, _threshold);

    }

    /**
     * @brief [length given as scalar] removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _output 3D output stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
    static const error_code encode(raw_type* _input,
                                   raw_type* _output,
                                   const size_type& _length,
                                   const raw_type& _threshold)
    {
        if(_output)
            return encode_out_of_place(_input, _output, _length, _threshold);
        else
            return encode_inplace(_input, _length, _threshold);

    }



    /**
     * @brief clipping removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _output 3D output stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
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



    /**
     * @brief clipping removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
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
    /**
    * @brief reconstructing the background removal from an estimate is impossible (so far),
    * therefor the input buffer is copied to the output buffer of size given by dimensionality
    *
    * @return sqeazy::error_code
    */
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
        std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }

    template <typename SizeType>
    /**
    * @brief reconstructing the background removal from an estimate is impossible (so far),
    * therefor the input buffer is copied to the output buffer of size given by dimensionality
    *
    * @return sqeazy::error_code
    */
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<SizeType>& _length)
    {
        unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());

        return decode(_input, _output, total_size);
    }

};



/**
* @brief this implements a shot noise type removal scheme in static member function encode, i.e. inside the neighborhood of a pixel
* in _input, the number of pixels are counted that fall under a certain threshold, if this count exceeds
* the limit "percentage_below" (or the one given at runtime), the central pixel
* (around which the neighborhood is located) is set to 0 as well.
*
* this scheme cannot be run inplace.
* this scheme is not reversable.
*
*/
template < typename T,
         typename Neighborhood = cube_neighborhood<3>,
         short percentage_below = 75 >
struct flatten_to_neighborhood {

    typedef T raw_type;
    typedef T compressed_type;


    static const bool is_compressor = false;
    static const float fraction_below = percentage_below/100.f;

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string name() {

        std::ostringstream msg;
        msg << "rmbkrd_neighbor"
            << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
            << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
            << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ;

        return msg.str();

    }


    template <typename size_type>
    /**
     * @brief encoding the flatten_to_neighborhood scheme
     *
     * @param _input Input 3D image stack of raw_type
     * @param _output Output 3D image stack of raw_type
     * @param _dims std::vector<int> that contains the dimensionality of _input and _output
     * @param _threshold the threshold under which a pixel is considered noise
     * @param _frac_neighb_to_null fraction of neighboring pixels (exclusive neighborhood) that decided
     * if the central pixel is to be set to 0 (count > _frac_neighb_to_null) or kept as is (count <= _frac_neighb_to_null)

     * @return sqeazy::error_code
     */
    static const error_code encode(raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<size_type>& _dims,
                                   const raw_type& _threshold,
                                   float _frac_neighb_to_null = fraction_below)
    {


        unsigned long length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<raw_type>());

        std::vector<size_type> offsets;
        sqeazy::halo<Neighborhood, size_type> geometry(_dims.begin(), _dims.end());
        geometry.compute_offsets_in_x(offsets);

        size_type halo_size_x = length - offsets[0];

        //no offsets in other dimensions than x
        if(offsets.size()!=1)
        {
            halo_size_x = geometry.non_halo_end(0) - geometry.non_halo_begin(0) + 1;
        }

        unsigned long local_index=0;
        unsigned n_neighbors_below_threshold = 0;
        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();


        const float cut_fraction = _frac_neighb_to_null*(size<Neighborhood>()-1);
        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
            for(unsigned long index = 0; index < halo_size_x; ++index) {

                local_index = index + *offsetsItr;
                n_neighbors_below_threshold = count_neighbors_if<Neighborhood>(_input + local_index,
                                              _dims,
                                              std::bind2nd(std::less<raw_type>(), _threshold)
                                                                              );
                if(n_neighbors_below_threshold>cut_fraction)
                    _output[local_index] = 0;
                else
                    _output[local_index] = _input[local_index];

            }
        }

        return SUCCESS;
    }


    template <typename SizeType>
    /**
     * @brief decoding the _input to _output given just the length of the buffers as scalar
     * here: not operation except copying is performed
     *
     * @return sqeazy::error_code
     */
    static const error_code decode(const raw_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
        if(_input!=_output )
            std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }

    template <typename SizeType>
    /**
     * @brief decoding the _input to _output given just the length of the buffers as vector (dimensionality)
     * here: not operation except copying is performed
     *
     *
     * @return sqeazy::error_code
     */
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

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string name() {


        return std::string("rmestbkrd");

    }

    template <typename ItrT>
    /**
     * @brief Calculate the mean and standard deviation of the memory region between [begin,end)
     * in one go and write it to _mean and _var (this approach assumes a high statistic
     * sample contained in [begin,end))
     *
     * @param begin begin of buffer
     * @param end exclusive end of buffer
     * @param _mean float that the resulting mean is written to
     * @param _var float that the resulting standard deviation is written to
     * @return const void
     */
    static const void mean_and_var(ItrT begin, ItrT end, float& _mean, float& _var) {

        unsigned long length = end - begin;
        float sum = 0.f;
        float sum_of_squares = 0.f;

        for(; begin!=end; ++begin) {
            sum += float(*begin);
            sum_of_squares += float(*begin) * float(*begin);
        }

        _mean = sum/length;
        _var = std::sqrt((sum_of_squares/length) - (_mean*_mean));

    }

    template <typename size_type>
    /**
     * @brief applying the background/noise removal from an estimate of the noise level
     * given the darkest faces of the volume. if the out-of-place operation is requested (input buffer is
     * at a different memory location than the output buffer) lonely peaks on a noisy neighborhood
     * are removed from the sample
     *
     * @param _input input 3D stack encoded as raw_type
     * @param _output output 3D stack encoded as raw_type (must have same dimensionality than _input)
     * @param _dims dimensionality of input, i.e. the extents along each dimension
     * @return sqeazy::error_code
     */
    static const error_code encode(raw_type* _input,
                                   compressed_type* _output,
                                   const std::vector<size_type>& _dims)
    {


        std::vector<raw_type> darkest_face;
        extract_darkest_face((const raw_type*)_input, _dims, darkest_face);

        float sd = 0.f;
        float mean = 0.f;

        mean_and_var(darkest_face.begin(), darkest_face.end(), mean, sd);

        const float alpha = 1.f;
        size_type input_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<size_type>());
        const float reduce_by = mean+(alpha*sd);

        if(_output) {
            flatten_to_neighborhood<raw_type>::encode(_input, _output, _dims, reduce_by);
            remove_background<raw_type>::encode_inplace(_output, input_length, reduce_by);
        }
        else {
            std::cerr << "WARNING ["<< name() <<"::encode]\t inplace operation requested, flatten_to_neighborhood skipped\n";
            remove_background<raw_type>::encode_inplace(_input, input_length, reduce_by);

        }




        return SUCCESS;
    }

    template <typename SizeType>
    /**
     * @brief reconstructing the background removal from an estimate is impossible (so far),
     * therefor the input buffer is copied to the output buffer of size _length
     *
     * @return sqeazy::error_code
     */
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
        std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }

    template <typename SizeType>
    /**
    * @brief reconstructing the background removal from an estimate is impossible (so far),
    * therefor the input buffer is copied to the output buffer of size given by dimensionality
    *
    * @return sqeazy::error_code
    */
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
