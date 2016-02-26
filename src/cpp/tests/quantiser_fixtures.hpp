#ifndef _QUANTISER_FIXTURES_H_
#define _QUANTISER_FIXTURES_H_
#include <iostream>
#include <vector>
#include <climits>
#include <cstdint>
#include <array>
#include <random>

#include "boost/align/aligned_allocator.hpp"



namespace sqeazy {

  template <typename from_type>
  struct quantise_fixture
  {
    static const uint32_t central_value = 12345;
    static const uint32_t size = 1 << 12;//image 64x64 = 4096 pixels 

    using aligned_vector = std::vector<from_type,
				       boost::alignment::aligned_allocator<from_type, 32>
				       >;

    std::random_device rd_;
    std::mt19937 gen_;
  
  
    //I(x,y) =  { 12345+ rnd(0,255) }
    aligned_vector shifted_;

    //I(x,y) =  { 12345+ 5*rnd(0,255) }
    aligned_vector shifted_wide_;

    //I(x,y) =  { 12345+ 5*gaussian(0,128) }
    aligned_vector shifted_normal_;

    //I(x,y) =  { 12345+ 5*gaussian(0,128) }
    aligned_vector strong_0_;

    //I(x,y) =  { |gaussian(0,1500)| }
    aligned_vector realistic_;

  
    quantise_fixture():
      rd_(),
      gen_(rd_()),
      shifted_(size,central_value),
      shifted_wide_(size,central_value),
      shifted_normal_(size,central_value),
      strong_0_(4*size,0),
      realistic_(size,0)
    {
      gen_.seed(12);
      std::uniform_int_distribution<> all_chars(0,255);
      std::normal_distribution<> normal(0,128);
      std::normal_distribution<> real_norm(0,500);
      
      for(uint32_t i = 0;i<size;++i){
	shifted_[i] += all_chars(gen_);
	shifted_wide_[i] += 5*all_chars(gen_);
	shifted_normal_[i] += 5*normal(gen_);
	realistic_[i] = std::abs(real_norm(gen_));
      }

      
      
      const uint32_t max_filled = strong_0_.size()/8;
      std::uniform_int_distribution<> wide(-255,255);
      for(uint32_t i = 0;i<max_filled;++i)
	strong_0_[i] = central_value + wide(gen_);
      
    }
  
  };
}//sqeazy namespace

#endif














