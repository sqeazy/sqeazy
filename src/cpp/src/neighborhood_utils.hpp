#ifndef _NEIGHBORHOOD_UTILS_H_
#define _NEIGHBORHOOD_UTILS_H_

#include <vector>
#include <numeric>

#include "sqeazy_traits.hpp"


namespace sqeazy {

  
  
template <typename Neighborhood>
static const int offset_begin_on_axis(const int& _dim_number) {

    static int begins[Neighborhood::num_dims] = {Neighborhood::x_offset_begin,
            Neighborhood::y_offset_begin,
            Neighborhood::z_offset_begin
                                                };

    if(_dim_number<(int)Neighborhood::num_dims) {
        return begins[_dim_number];
    }
    else
        return -1;

}

template <typename Neighborhood>
static const int offset_end_on_axis(const int& _dim_number) {

    static int ends[Neighborhood::num_dims] = {Neighborhood::x_offset_end,
            Neighborhood::y_offset_end,
            Neighborhood::z_offset_end
                                              };

    if(_dim_number<(int)Neighborhood::num_dims) {
        return ends[_dim_number];
    }
    else
        return -1;

}

template <typename Neighborhood>
static const unsigned num_traversed_pixels() {

    unsigned value = 1;
    for(unsigned dim_id = 0; dim_id<Neighborhood::num_dims; ++dim_id) {

        value *= offset_end_on_axis<Neighborhood>(dim_id) - offset_begin_on_axis<Neighborhood>(dim_id);

    }

    return value;

}

template <typename Neighborhood>
static const unsigned size() {

    unsigned value = 1;
    for(unsigned dim_id = 0; dim_id<Neighborhood::num_dims; ++dim_id) {

        value *= offset_end_on_axis<Neighborhood>(dim_id) - offset_begin_on_axis<Neighborhood>(dim_id);

    }

    return value;

}

template < unsigned extent = 3>
struct last_plane_neighborhood {

    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
    static const unsigned num_dims    = 3;


    //this is the inclusive start
    static const int z_offset_begin = -1;
    static const int y_offset_begin = -axis_half;
    static const int x_offset_begin = -axis_half;


    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = z_offset_begin+1;
    static const int y_offset_end = axis_half+1;
    static const int x_offset_end = axis_half+1;

    
};

template < unsigned num_pixels = 8>
struct last_pixels_on_line_neighborhood {

    static const unsigned axis_length = num_pixels;
    static const unsigned axis_half   = axis_length/2;
    static const unsigned num_dims    = 3;


    //this is the inclusive start
    static const int z_offset_begin = 0;
    static const int y_offset_begin = 0;
    static const int x_offset_begin = -num_pixels;


    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = z_offset_begin+1;
    static const int y_offset_end = y_offset_begin+1;
    static const int x_offset_end = 0;

    
};



template < unsigned extent = 3>
struct last_pixels_in_cube_neighborhood {

    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
    static const unsigned num_dims    = 3;

    //this is the inclusive start
    static const int z_offset_begin = -axis_half;
    static const int y_offset_begin = -axis_half;
    static const int x_offset_begin = -axis_half;


    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = 1;
    static const int y_offset_end = 1;
    static const int x_offset_end = 1;

};

template < unsigned extent = 3>
struct cube_neighborhood {

    static const unsigned axis_length = extent;
    static const unsigned axis_half   = extent/2;
    static const unsigned num_dims    = 3;

    //this is the inclusive start
    static const int z_offset_begin = -axis_half;
    static const int y_offset_begin = -axis_half;
    static const int x_offset_begin = -axis_half;


    //this is the exclusive end, so the index one past the last element
    static const int z_offset_end = axis_half + ((extent % 2 == 1) ? 1 : 0);
    static const int y_offset_end = axis_half + ((extent % 2 == 1) ? 1 : 0);
    static const int x_offset_end = axis_half + ((extent % 2 == 1) ? 1 : 0);

};

template <typename Neighborhood, typename U>
struct halo {

    typedef typename sqeazy::twice_as_wide<U>::type length_type;
    std::vector<U> world;

    halo(const U& _w, const U& _h, const U& _d) {
        world.resize(3);
        world[0] = _w;
        world[1] = _h;
        world[2] = _d;
    }

    template <typename Itr>
    halo(Itr begin, Itr end) :
        world(begin, end)
    {

    }


    U non_halo_begin(int dim_id) const  {
        U begin = 0;
	if(offset_begin_on_axis<Neighborhood>(dim_id) < 0) 
	  begin = -1*offset_begin_on_axis<Neighborhood>(dim_id) ;
        return begin;
    }



    U non_halo_end(int dim_id) const  {
        U end = 0;
	if(offset_end_on_axis<Neighborhood>(dim_id) > 0)
	  end = world.at(dim_id) - offset_end_on_axis<Neighborhood>(dim_id) +1;
	else
	  end = world.at(dim_id);
        return end;
    }

    template <typename T>
    void compute_offsets_in_x(std::vector<T>& _offsets) const {



        length_type num_offsets_required = 1;
        int non_halo_length = 0;
        for(int i = world.size()-1; i>0; --i) {
            non_halo_length = non_halo_end(i) - non_halo_begin(i);
            if(non_halo_length!=(int)world.at(i))
                num_offsets_required *= non_halo_length;
        }

        _offsets.clear();
        _offsets.reserve(num_offsets_required);
        std::fill(_offsets.begin(), _offsets.end(),0);

        if(num_offsets_required>1) {

            length_type offset = 0;
            length_type length = std::accumulate(world.begin(), world.end(), 1, std::multiplies<U>());

            for(T z_index = non_halo_begin(2); z_index<non_halo_end(2); ++z_index) {
                for(T y_index = non_halo_begin(1); y_index<non_halo_end(1); ++y_index) {

                    offset = z_index*world[1]*world[0] + y_index*world[0] + non_halo_begin(0);
                    if(offset<length)
                        _offsets.push_back(offset);

                }
            }
        }
        else {
	    _offsets.resize(1);
            _offsets[0] =(non_halo_begin(0));

        }


    }

};




} //sqeazy
#endif /* _NEIGHBORHOOD_UTILS_H_ */
