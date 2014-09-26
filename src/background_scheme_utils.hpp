#ifndef _BACKGROUND_SCHEME_UTILS_H_
#define _BACKGROUND_SCHEME_UTILS_H_
#include <limits>
#include <climits>
#include <iostream>
#include <vector>
#include "sqeazy_traits.hpp"
#include "neighborhood_utils.hpp"
#include "hist_impl.hpp"

namespace sqeazy {

  
    template <typename raw_type, typename size_type>
    /**
     * @brief search for the place with the lowest mean value (median would be too expensive).
     * the current implementation loops through the first & last two x-y planes
     * as well as the first & last z-x plane
     *
     * @param _input 3D stack that is to be parsed
     * @param _dims dimensionality of the input
     * @param _darkest_face (inout type) this is the vector that will contain the result
     * @return const void
     */
    static const void extract_darkest_face(const raw_type* _input,
                                           const std::vector<size_type>& _dims,
                                           std::vector<raw_type>& _darkest_face) {

        typedef typename add_unsigned<typename twice_as_wide<size_type>::type >::type index_type;

        raw_type median = std::numeric_limits<raw_type>::max();
        index_type input_index =0;
        const index_type frame_size = _dims[1]*_dims[0];
        index_type face_index =0;
	sqeazy::histogram<raw_type> running_histo;

        //faces with z (first 2 and last 2)
        const size_type indices[4] = {0,1,_dims[2]-2,_dims[2]-1};

        for(size_type z_idx = 0, z_idx_ctr = 0;
                z_idx < 4;
                ++z_idx_ctr, z_idx = indices[z_idx_ctr]) {

            input_index = z_idx*(frame_size);
            const raw_type* begin = _input + input_index;
	    running_histo.fill_from_image(begin, begin + (frame_size));
            raw_type temp = running_histo.median()
	      ;// std::accumulate(begin, begin + (frame_size),0 )/(frame_size)
            if(temp < median) {
                if(_darkest_face.size()<frame_size)
                    _darkest_face.resize(frame_size);
                std::copy(begin, begin + (frame_size),_darkest_face.begin());
                median = temp;
            }
        }

        //faces with y
        face_index =0;
	raw_type temp;
	std::vector<raw_type> face(_dims[2]*_dims[0]);
        for(size_type y_idx = 0; y_idx < _dims[1]; y_idx+=(_dims[1]-1)) {

            for(size_type z_idx = 0; z_idx < _dims[2]; ++z_idx) {
	      input_index = z_idx*(frame_size)+y_idx*_dims[0];
	      std::copy(_input + input_index, _input+input_index+_dims[0], face.begin() + (z_idx*_dims[0]));
            }

	    running_histo.fill_from_image(face.begin(), face.end());
            temp = running_histo.median();

            if(temp < median) {
                if(_darkest_face.size()<_dims[2]*_dims[0])
                    _darkest_face.resize(_dims[2]*_dims[0]);

		std::copy(face.begin(), face.end(),_darkest_face.begin());
		
                median = temp;
            }
        }

    }

  
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
