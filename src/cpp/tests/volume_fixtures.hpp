#ifndef _VOLUME_FIXTURES_H_
#define _VOLUME_FIXTURES_H_
#include <iostream>
#include <vector>
#include <climits>
#include <cstdint>
#include <array>
#include <random>

#include "boost/align/aligned_allocator.hpp"
#include "boost/multi_array.hpp"
#include "traits.hpp"


namespace sqeazy {

  template<typename T, uint32_t largest_axis = 120, int perc_of_max = 80>
  struct volume_fixture {


    // constexpr static const
    const std::array<uint32_t,3> shape_ = {{largest_axis, (largest_axis/3) + 4, (largest_axis/3) + 8 }};//z,y,x according to c_storage_order
    const std::array<uint32_t,3> half_shape_ = {{shape_[row_major::z]/2u, shape_[row_major::y]/2u , shape_[row_major::x]/2u}};
    using image_stack = boost::multi_array<T, 3, boost::alignment::aligned_allocator<T, 32>
					   >;
    typedef image_stack stack_t;
    typedef T pixel_t;

#ifdef _WIN32
    static const T signal_intensity_ = (std::numeric_limits<T>::max)()*perc_of_max/100.f;
#else
	static const T signal_intensity_ = std::numeric_limits<T>::max()*perc_of_max / 100.f;
#endif

    image_stack embryo_;
    image_stack noisy_embryo_;
    image_stack retrieved_;

    volume_fixture():
      embryo_(),
      noisy_embryo_(),
      retrieved_()
    {

      embryo_.resize(shape_);
      const uint32_t n_elems = embryo_.num_elements();
      std::fill(embryo_.data(), embryo_.data() + n_elems,0);

      std::array<float,3> denoms;
      std::copy(half_shape_.begin(), half_shape_.end(),denoms.begin());

      for ( float& item : denoms )
        item = 1.f/(.75*.75*item*item);

      float ellipsoid = 0.f;
      float dist_to_1 = 0.f;

      for(uint32_t z=0;z<shape_[row_major::z];z++)
        for(uint32_t y=0;y<shape_[row_major::y];y++)
          for(uint32_t x=0;x<shape_[row_major::x];x++){

            float x_2 = (float(x) - half_shape_[row_major::x]);x_2 *= x_2;
            float y_2 = (float(y) - half_shape_[row_major::y]);y_2 *= y_2;
            float z_2 = (float(z) - half_shape_[row_major::z]);z_2 *= z_2;

            ellipsoid = x_2* denoms[row_major::x];
            ellipsoid += y_2*denoms[row_major::y];
            ellipsoid += z_2*denoms[row_major::z];

            dist_to_1 = std::abs(1 - ellipsoid);
            if ( dist_to_1 < .07 ){

              embryo_[z][y][x] = signal_intensity_;

            }

          }


      std::random_device rd;
      std::mt19937 gen(rd());
      std::exponential_distribution<> dis(1.f/(.01f*signal_intensity_));

      noisy_embryo_.resize(shape_);
      noisy_embryo_ = embryo_;

      for(uint32_t idx = 0;idx < noisy_embryo_.num_elements();++idx)
        noisy_embryo_.data()[idx] += dis(gen);


    }

  };

}//sqeazy namespace

#endif














