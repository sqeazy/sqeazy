#ifndef _BITSWAP_SCHEME_SCALAR_H_
#define _BITSWAP_SCHEME_SCALAR_H_
#include <limits>
#include <climits>
#include <iostream>
#include "traits.hpp"
#include "sqeazy_common.hpp"
#include "scalar_utils.hpp"

#ifdef _OPENMP
#include "omp.h"
#endif

namespace sqeazy {

  namespace detail {


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //scalar implementation
    template <const unsigned num_bits_per_plane, 
              typename raw_type ,
              typename size_type
              >
    static const error_code scalar_bitplane_reorder_encode(const raw_type* _input,
                                                           raw_type* _output,
                                                           const size_type& _length,
                                                           int num_threads = 1)
    {

      typedef typename std::make_signed<size_type>::type loop_size_type;//boiler plat required for MS VS 14 2015 OpenMP implementation


      static const unsigned type_width = CHAR_BIT*sizeof(raw_type);
      static const unsigned num_planes = type_width/num_bits_per_plane;

      const unsigned segment_length = _length/num_planes;
      const loop_size_type len = _length;

      const raw_type mask = ~(~0 << (num_bits_per_plane));//mask to extract all last bits up to bit at num_bits_per_plane
      raw_type value = 0;


#pragma omp parallel for                        \
  private(value)                                \
  shared(_input, _output)                       \
  num_threads(num_threads)
      for(loop_size_type index = 0; index < len; ++index) {


        value = xor_if_signed(_input[index]);
        value = rotate_left<1>(value);

        size_type output_bit_offset = (type_width - num_bits_per_plane) - ((index % num_planes)*num_bits_per_plane);

        for(unsigned plane_index = 0; plane_index<num_planes; ++plane_index) {

          size_type input_bit_offset = plane_index*num_bits_per_plane;
          raw_type extracted_bits = (value >> input_bit_offset) & mask;

          size_type output_index = ((num_planes-1-plane_index)*segment_length) + (index/num_planes);

          _output[output_index] = setbits_of_integertype(_output[output_index],
                                                         extracted_bits,
                                                         output_bit_offset,
                                                         num_bits_per_plane);
        }
      }

      return SUCCESS;

    }


    template <const unsigned num_bits_per_plane,
              typename raw_type ,
              typename size_type
              >
    static const error_code scalar_bitplane_reorder_decode(const raw_type* _input,
                                                           raw_type* _output,
                                                           const size_type& _length,
                                                           int num_threads = 1){

      typedef typename std::make_signed<size_type>::type loop_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation

      static const unsigned raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;
      static const unsigned num_planes = raw_type_num_bits/num_bits_per_plane;

      const loop_size_type len = _length;
      const loop_size_type signed_num_planes = num_planes;
      const unsigned segment_length = _length/num_planes;
      const raw_type mask = ~(~0 << (num_bits_per_plane));
      static const unsigned type_width = CHAR_BIT*sizeof(raw_type);

#pragma omp parallel for shared(_input, _output)    \
  num_threads(num_threads)
      for(loop_size_type plane_index = 0; plane_index<signed_num_planes; ++plane_index) {

        size_type output_bit_offset = (plane_index*num_bits_per_plane);

        for(size_type index = 0; index < _length; ++index) {

          size_type input_bit_offset = (type_width - num_bits_per_plane) - (index % num_planes)*num_bits_per_plane;
          size_type input_index = ((num_planes-1-plane_index)*segment_length) + index/num_planes;
          raw_type extracted_bits = (_input[input_index] >> input_bit_offset) & mask;

          _output[index] = setbits_of_integertype(_output[index],extracted_bits,
                                                  output_bit_offset,num_bits_per_plane);
        }
      }

#pragma omp parallel for shared(_input, _output)    \
  num_threads(num_threads)
      for(loop_size_type index = 0; index < len; ++index) {
        _output[index] = xor_if_signed(rotate_right<1>(_output[index]));
      }

      return SUCCESS;
    }



  };
}; //sqeazy
#endif /* _BITSWAP_SCHEME_SCALAR_H_ */
