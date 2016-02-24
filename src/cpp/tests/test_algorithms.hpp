#ifndef _TEST_ALGORITHMS_H_
#define _TEST_ALGORITHMS_H_

#include <cmath>
#include <numeric>
#include <vector>
//#include "image_stack_utils.h"
#include "traits.hpp"

namespace sqeazy {


  template <typename ImageStackT, typename DimT>
  typename ImageStackT::element sum_from_offset(const ImageStackT& _image,
						const std::vector<DimT>& _offset) {

    typename ImageStackT::element value = 0.f;

    // TODO: adapt loops to storage order for the sake of speed (if so, prefetch
    // line to use SIMD)
    for (int image_z = _offset[0]; image_z < int(_image.shape()[0] - _offset[0]);
	 ++image_z) {
      for (int image_y = _offset[1];
	   image_y < int(_image.shape()[1] - _offset[1]); ++image_y) {
	for (int image_x = _offset[2];
	     image_x < int(_image.shape()[2] - _offset[2]); ++image_x) {

	  value += _image[image_z][image_y][image_x];
	}
      }
    }

    return value;
  }

// #ifdef _OPENMP
// #include "omp.h"
// #endif

//   template <typename ValueT, typename DimT>
//   typename sqeazy::twice_as_wide<ValueT>::type l2norm(ValueT* _first, ValueT* _second, const DimT& _size) {

//     typedef typename sqeazy::twice_as_wide<ValueT>::type return_t;
    
//     return_t l2norm = 0.;

// #ifdef _OPENMP
//     int nthreads = omp_get_num_procs();
// #pragma omp parallel for num_threads(nthreads) shared(l2norm)
// #endif
//     for (std::size_t p = 0; p < _size; ++p){
//       return_t diff = _first[p] - _second[p];
//       l2norm += diff*diff;
//     }

//     return_t scale = 1/return_t(_size);
//     l2norm *= scale;
    
//     return l2norm;
//   }

template<typename in_type, typename out_type = in_type>
  struct diff_squared {

    out_type operator()(const in_type& _first, const in_type& _second){

      out_type value = _first - _second;
      return (value*value);
    
    }
  
  };

  

  template <typename stack_type>
  double l2norm(const stack_type& _reference, const stack_type& _data){
    double l2norm = std::inner_product(_data.data(),
				       _data.data() + _data.num_elements(),
				       _reference.data(),
				       0.,
				       std::plus<double>(),
				       diff_squared<float,double>()
				       );

    double value = std::sqrt(l2norm)/_data.num_elements();

    return value;
  }

  template<typename in_type, typename out_type = in_type>
  struct diff_fabs {

    out_type operator()(const in_type& _first, const in_type& _second){

      out_type value = _first - _second;
      return std::fabs(value);
    
    }
  
  };
  
  
  template <typename stack_type>
  double l1norm(const stack_type& _reference, const stack_type& _data){
    double l1norm = std::inner_product(_data.data(),
				       _data.data() + _data.num_elements(),
				       _reference.data(),
				       0.,
				       std::plus<double>(),
				       diff_fabs<float,double>()
				       );

    double value = std::sqrt(l1norm)/_data.num_elements();

    return value;
  }

  template <typename ValueT, typename DimT>
  ValueT max_value(ValueT* _data, const DimT& _size) {

    ValueT max = *std::max_element(_data, _data + _size);

    return max;
  }

  template <typename ValueT, typename DimT>
  ValueT min_value(ValueT* _data, const DimT& _size) {

    ValueT min = *std::min_element(_data, _data + _size);

    return min;
  }
}
#endif /* _TEST_ALGORITHMS_H_ */
