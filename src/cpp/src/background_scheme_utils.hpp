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
     * @param _dims dimensionality of the input complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
     * @param _darkest_face (inout type) this is the vector that will contain the result
     * @return const void
     */
    static const void extract_darkest_face(const raw_type* _input,
                                           const std::vector<size_type>& _dims,
                                           std::vector<raw_type>& _darkest_face) {

        typedef typename add_unsigned<typename twice_as_wide<size_type>::type >::type index_type;

        raw_type support = std::numeric_limits<raw_type>::max();
        index_type input_index =0;
        const index_type frame_size = _dims[_dims.size()-2]*_dims[_dims.size()-1];
        index_type face_index =0;
	sqeazy::histogram<raw_type> running_histo;
	float temp = 0;
	
	#ifdef _SQY_VERBOSE_
	std::cout << "[SQY_VERBOSE]\t extract_darkest_face on image ";
	for(auto i = 0;i<_dims.size();++i){
	  std::cout << _dims[i] << ((_dims[i]!=_dims.back()) ? "x" : " ");
	}
	std::cout << "\n";
#endif
        //faces with z
        const size_type indices[2] = {0,_dims[0]-1};
	size_type z_idx = 0;
	
        for(size_type z_idx_ctr = 0;
	    z_idx_ctr < 2;
	    ++z_idx_ctr ) {

	  z_idx = indices[z_idx_ctr];
	  if(!(z_idx < _dims[0])){
	    continue;
	  }
	  
	  input_index = z_idx*(frame_size);
	  const raw_type* begin = _input + input_index;
	  running_histo.clear();

	  running_histo.add_from_image(begin, begin + (frame_size));

	  temp = running_histo.calc_support();

#ifdef _SQY_VERBOSE_
	  running_histo.fill_stats();
	  std::cout << "[SQY_VERBOSE]\t face z = " << z_idx << " / " << _dims[0] <<" , support = " << temp << ", mean = " << running_histo.calc_mean() << "\n";
	    
#endif


	  if(temp < support) {
	    if(_darkest_face.size()<frame_size)
	      _darkest_face.resize(frame_size);

	    //FIXME: do we really need to copy the face out?
	    std::copy(begin, begin + (frame_size),_darkest_face.begin());
	    support = temp;
	  }
        }

        //faces with y
        face_index =0;
	std::vector<raw_type> face(_dims[2]*_dims[0]);
        for(size_type y_idx = 0; y_idx < _dims[1]; y_idx+=(_dims[1]-1)) {

	  running_histo.clear();

            for(size_type z_idx = 0; z_idx < _dims[0]; ++z_idx) {
	      input_index = z_idx*(frame_size)+y_idx*_dims[2];
	      std::copy(_input + input_index, _input+input_index+_dims[2], face.begin() + (z_idx*_dims[2]));
	      running_histo.add_from_image(_input + input_index, _input+input_index+_dims[2]);
	      
            }
	    
            temp = running_histo.calc_support();
	    #ifdef _SQY_VERBOSE_
	    running_histo.fill_stats();
	    std::cout << "[SQY_VERBOSE]\t face y = " << y_idx << " / "<< _dims[1] << " , support = " << temp << ", mean = " << running_histo.calc_mean() << "\n";
	    
#endif

            if(temp < support) {
	      if(_darkest_face.size()<(size_t)(_dims[2]*_dims[0]))
                    _darkest_face.resize(_dims[2]*_dims[0]);
		//FIXME: do we really need to copy the face out?
		std::copy(face.begin(), face.end(),_darkest_face.begin());
		
                support = temp;
            }
        }

    }

  /**
     \brief 
     
     \param[in] _input payload
     \param[in] _dims dimensionality of the input complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
     \param[in] _pred predicate unary functor
     
     \return 
     \retval 
     
  */
template <typename Neighborhood, typename Value_type, typename Size_type, typename Pred>
static unsigned  count_neighbors_if(Value_type* _input,
                                    const std::vector<Size_type>& _dims,
                                    Pred _pred) {


    typedef typename remove_unsigned<Size_type>::type coord_t;


    const coord_t row_size = _dims[2];
    const coord_t frame_size = _dims[1]*row_size;

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
