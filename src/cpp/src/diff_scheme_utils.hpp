#ifndef _DIFF_SCHEME_UTILS_H_
#define _DIFF_SCHEME_UTILS_H_

#include <vector>
#include <numeric>

#include "traits.hpp"


namespace sqeazy {

template <typename Neighborhood, typename T, typename U>
T halo_aware_sum(const T* _ptr, const U& _index,
                 const unsigned& _width,
                 const unsigned& _height,
                 const unsigned& _depth) {

    typedef typename remove_unsigned<T>::type coord_t;

    U length = _width*_height*_depth;
    U frame = _width*_height;
    U sum_index = 0;
    unsigned long z_sum_index = 0;
    unsigned long y_sum_index = 0;
    unsigned long x_sum_index = 0;
    T sum = 0;

    coord_t z_pos = _index/frame;

    U frame_index = _index - z_pos*frame;

    coord_t y_pos = frame_index/_width;
    coord_t x_pos = _index - (z_pos*frame + y_pos*_width);

    for(long z_offset = Neighborhood::z_offset_begin; z_offset<Neighborhood::z_offset_end; ++z_offset) {

        if((z_pos + z_offset)>-1 && (z_pos + z_offset)<_depth)
            z_sum_index =  (z_pos + z_offset)*frame ;
        else
            z_sum_index = length;

        for(long y_offset = Neighborhood::y_offset_begin; y_offset<Neighborhood::y_offset_end; ++y_offset) {

            if((y_pos + y_offset)>-1 && (y_pos + y_offset)<_height)
                y_sum_index =  (y_pos + y_offset)*_width ;
            else
                y_sum_index = length;

            for(long x_offset = Neighborhood::x_offset_begin; x_offset<Neighborhood::x_offset_end; ++x_offset) {

                if((x_pos + x_offset)>-1 && (x_pos + x_offset)<_width)
                    x_sum_index = x_pos + x_offset;
                else
                    x_sum_index = length;

                sum_index = z_sum_index + y_sum_index + x_sum_index;

                if(sum_index<length)
                    sum += _ptr[sum_index];
                else
                    sum += 0;

            }
        }
    }

    return sum;
}

template <typename Neighborhood, typename T, typename U>
T naive_sum(const T* _ptr, const U& _index,
            const unsigned& _width,
            const unsigned& _height,
            const unsigned& _depth) {

    typedef typename remove_unsigned<T>::type coord_t;

    U frame = _width*_height;

    T sum = 0;

    coord_t z_pos = _index/frame;

    U frame_index = _index - z_pos*frame;

    coord_t y_pos = frame_index/_width;
    coord_t x_pos = _index - (z_pos*frame + y_pos*_width);

    for(long z_offset = Neighborhood::z_offset_begin; z_offset<Neighborhood::z_offset_end; ++z_offset) {
        for(long y_offset = Neighborhood::y_offset_begin; y_offset<Neighborhood::y_offset_end; ++y_offset) {
            for(long x_offset = Neighborhood::x_offset_begin; x_offset<Neighborhood::x_offset_end; ++x_offset) {
                U element_index = (z_pos+z_offset)*frame + (y_pos+y_offset)*_width + x_pos + x_offset;
                sum += _ptr[element_index];
            }
        }
    }

    return sum;
}


} //sqeazy
#endif /* _DIFF_SCHEME_UTILS_H_ */
