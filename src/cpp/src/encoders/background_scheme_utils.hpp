#ifndef _BACKGROUND_SCHEME_UTILS_H_
#define _BACKGROUND_SCHEME_UTILS_H_


#include "traits.hpp"
#include "neighborhood_utils.hpp"
#include "hist_impl.hpp"
#include "compass.hpp"

#ifdef _OPENMP
#include "omp.h"
#endif

#include <limits>
#include <climits>
#include <iostream>
#include <vector>

namespace sqeazy {

  template <typename raw_type, typename size_type>
  /**
   * @brief calculate the support (99% quantile) of 4 planes
   - z=0 (x & y free)
   - z=last (x & y free)
   - y=0 (x free, z = {1,mid,last} )
   - y=last (x free, free, z = {1,mid,last} )
   and return them
   *
   * @param _input 3D stack that is to be parsed
   * @param _dims dimensionality of the input complying to c_storage_order _dims[] = {z-shape,y-shape,x-shape}
   * @param _darkest_face (inout type) this is the vector that will contain the result
   * @return const void
   */
  static std::vector<float> extract_darkest_face_supports(const raw_type* _input,
                                                          const std::vector<size_type> & _dims,
                                                          const float& _support = 0.99f,
                                                          int nthreads = 1) {

    std::vector<float> value(4,0.f);
    typedef typename add_unsigned<typename twice_as_wide<size_type>::type >::type index_type;


    const index_type frame_size = _dims[row_major::y]*_dims[row_major::x];
    index_type frame_portion = frame_size > compass::runtime::size::cache::level(2) ? compass::runtime::size::cache::level(2)*.75 : frame_size ;

    float* results_ptr = value.data();
    const size_type* shape = _dims.data();
    std::vector<sqeazy::histogram<raw_type>> hists(2);
    sqeazy::histogram<raw_type>* hists_ptr = hists.data();
    //faces with z,  i.e. z = const (y & x vary)
    size_type indices[2] = {0,_dims[row_major::z]-1};

#pragma omp parallel for \
  shared(results_ptr) \
  firstprivate(_input, indices, shape, hists_ptr)         \
  num_threads(nthreads)
    for(int i = 0;i < 2;++i ) {

      auto z_idx = indices[i];
      if(!(z_idx < shape[row_major::z])){
        continue;
      }

      index_type input_index = z_idx*(frame_size);
      const raw_type* begin = _input + input_index;
      const raw_type* end = begin + frame_portion;

      hists_ptr[i].add_from_image(begin, end);

      results_ptr[i] = hists_ptr[i].calc_support(_support);

      hists_ptr[i].clear();
    }

    indices[1] = shape[row_major::y]-1;

    //faces with y,  i.e. y = const (x varies, z = {1,mid,last} to sample cache efficient)
#pragma omp parallel for                        \
  shared(results_ptr) \
  firstprivate(_input, indices, shape, hists_ptr)         \
  num_threads(nthreads)
    for(int i = 0;i < 2;++i ) {

      auto y_idx = indices[i];
      if(!(y_idx < shape[row_major::y])){
        continue;
      }

      const size_type z_offsets[3] = {1,shape[row_major::z]/2,shape[row_major::z]-2};

      for(size_type z_idx : z_offsets)
      {
        index_type input_index = z_idx*(frame_size)+y_idx*shape[row_major::x];
        const raw_type* begin = _input + input_index;
        const raw_type* end = begin + shape[row_major::x];

        hists_ptr[i].add_from_image(begin, end);
      }

      results_ptr[i+2] = hists_ptr[i].calc_support(_support);
    }

    return value;
  }

  template <typename raw_type, typename size_type>
  /**
   * @brief calculate the support (99% quantile) of 4 planes
   - z=0 (x & y free)
   - z=last (x & y free)
   - y=0 (x free, z = {1,mid,last} )
   - y=last (x free, free, z = {1,mid,last} )
   and return the intensities of the subvolume with the lowest
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

#ifdef _WIN32
    raw_type support = (std::numeric_limits<raw_type>::max)();
#else
    raw_type support = std::numeric_limits<raw_type>::max();
#endif
    index_type input_index =0;
    const index_type frame_size = _dims[row_major::y]*_dims[row_major::x];

    sqeazy::histogram<raw_type> running_histo;
    float temp = 0;

#ifdef _SQY_VERBOSE_
    std::cout << "[SQY_VERBOSE]\t extract_darkest_face on image ";
    for(auto i = 0;i<_dims.size();++i){
      std::cout << _dims[i] << ((_dims[i]!=_dims.back()) ? "x" : " ");
    }
    std::cout << "\n";
#endif
    //faces with z,  i.e. z = const (y & x vary)
    const size_type indices[2] = {0,_dims[row_major::z]-1};
    size_type z_idx = 0;

    for(size_type z_idx_ctr = 0;
        z_idx_ctr < 2;
        ++z_idx_ctr ) {

      z_idx = indices[z_idx_ctr];
      if(!(z_idx < _dims[row_major::z])){
        continue;
      }

      input_index = z_idx*(frame_size);
      const raw_type* begin = _input + input_index;
      running_histo.clear();

      running_histo.add_from_image(begin, begin + (frame_size));

      temp = running_histo.calc_support();

#ifdef _SQY_VERBOSE_
      running_histo.fill_stats();
      std::cout << "[SQY_VERBOSE]\t face z = " << z_idx << " / " << _dims[row_major::z] <<" , support = " << temp << ", mean = " << running_histo.calc_mean() << "\n";

#endif


      if(temp < support) {
        if(_darkest_face.size()<frame_size)
          _darkest_face.resize(frame_size);

        //FIXME: do we really need to copy the face out?
        std::copy(begin, begin + (frame_size),_darkest_face.begin());
        support = temp;
      }
    }

    //faces with y, i.e. y = const (z & x vary)
    std::vector<size_type> z_planes = {1,_dims[row_major::z]/2,_dims[row_major::z]-1};
    std::vector<raw_type> face(_dims[row_major::x]*z_planes.size());

    for(size_type y_idx = 0; y_idx < _dims[row_major::y]; y_idx+=(_dims[row_major::y]-1)) {

      running_histo.clear();

      auto face_itr = face.begin();
      for(size_type z_idx : z_planes) {
        input_index = z_idx*(frame_size)+y_idx*_dims[row_major::x];
        face_itr = std::copy(_input + input_index,
                             _input+input_index+_dims[row_major::x],
                             face_itr);
        running_histo.add_from_image(_input + input_index, _input+input_index+_dims[row_major::x]);

      }

      temp = running_histo.calc_support();
#ifdef _SQY_VERBOSE_
      running_histo.fill_stats();
      std::cout << "[SQY_VERBOSE]\t face y = " << y_idx << " / "<< _dims[row_major::y] << " , support = " << temp << ", mean = " << running_histo.calc_mean() << "\n";

#endif

      if(temp < support) {
        if(_darkest_face.size()<face.size())
          _darkest_face.resize(face.size());
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
