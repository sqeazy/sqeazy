#ifndef _BACKGROUND_SCHEME_UTILS_H_
#define _BACKGROUND_SCHEME_UTILS_H_
#include <limits>
#include <climits>
#include <iostream>
#include "sqeazy_traits.hpp"
#include "neighborhood_utils.hpp"

namespace sqeazy {

template <typename Neighborhood, typename Value_type, typename Size_type, typename Pred>
static unsigned  count_neighbors_if(Value_type* _input,
                                    const std::vector<Size_type>& _dims,
                                    Pred _pred) {


    typedef typename remove_unsigned<Size_type>::type coord_t;


    const coord_t row_size = _dims[0];
    const coord_t frame_size = _dims[1]*row_size;
    //const Size_type stack_size = frame_size*_dims[2];

    coord_t neighbor_row_offset = 0;
    unsigned count = 0;
    const coord_t length_to_count = Neighborhood::x_offset_end - Neighborhood::x_offset_begin;
    Value_type* current_ = 0;

    for(coord_t z_offset = Neighborhood::z_offset_begin; z_offset<Neighborhood::z_offset_end; ++z_offset) {
        for(coord_t y_offset = Neighborhood::y_offset_begin; y_offset<Neighborhood::y_offset_end; ++y_offset) {

            neighbor_row_offset = z_offset*frame_size + y_offset*row_size ;
            current_ = _input + neighbor_row_offset + Neighborhood::x_offset_begin;

            if(current_!=(_input + Neighborhood::x_offset_begin)) {
                count += std::count_if(current_,
                                       current_ + length_to_count,
                                       _pred
                                      );
            }
            else {
	      current_ = _input + neighbor_row_offset ;
                for(coord_t x_offset = Neighborhood::x_offset_begin; x_offset<Neighborhood::x_offset_end; ++x_offset) {
		  Value_type* ptr = current_ + x_offset;
                    if(ptr != _input)
		      count += _pred(*ptr);
                    }
            }
        }
    }

    return count;

}

} //sqeazy
#endif /* _BACKGROUND_SCHEME_UTILS_H_ */
