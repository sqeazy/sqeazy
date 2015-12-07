#ifndef _IMAGE_STACK_HPP_
#define _IMAGE_STACK_HPP_

#include <cstdint>

#include "boost/multi_array.hpp"
#include "boost/align/aligned_allocator.hpp"

namespace sqeazy {

  //using 32byte ordering for AVX support, if buffer is 32byte aligned, it is 16 byte aligned as well
  
  typedef boost::multi_array<float, 3, boost::alignment::aligned_allocator<float, 32> > float32_image_stack;
  typedef boost::multi_array_ref<float, 3 > float32_image_stack_ref;
  typedef boost::const_multi_array_ref<float, 3> float32_image_stack_cref;
  typedef float32_image_stack::array_view<3>::type float32_image_stack_view;
  typedef float32_image_stack::array_view<2>::type float32_image_stack_frame;
  typedef float32_image_stack::array_view<1>::type float32_image_stack_line;

  typedef boost::multi_array<uint16_t, 3, boost::alignment::aligned_allocator<uint16_t, 32> > uint16_image_stack;
  typedef boost::multi_array_ref<uint16_t, 3> uint16_image_stack_ref;
  typedef boost::const_multi_array_ref<uint16_t, 3> uint16_image_stack_cref;
  typedef uint16_image_stack::array_view<3>::type uint16_image_stack_view;
  typedef uint16_image_stack::array_view<2>::type uint16_image_stack_frame;
  typedef uint16_image_stack::array_view<1>::type uint16_image_stack_line;

  typedef boost::multi_array<uint8_t, 3, boost::alignment::aligned_allocator<uint8_t, 32> > uint8_image_stack;
  typedef boost::multi_array_ref<uint8_t, 3> uint8_image_stack_ref;
  typedef boost::const_multi_array_ref<uint8_t, 3> uint8_image_stack_cref;
  typedef uint8_image_stack::array_view<3>::type uint8_image_stack_view;
  typedef uint8_image_stack::array_view<2>::type uint8_image_stack_frame;
  typedef uint8_image_stack::array_view<1>::type uint8_image_stack_line;
  
  typedef boost::multi_array<float, 2, boost::alignment::aligned_allocator<float, 32> > image_frame;
  typedef boost::multi_array_ref<float, 2 > image_frame_ref;
  typedef boost::const_multi_array_ref<float, 2> image_frame_cref;
    
  typedef boost::multi_array_types::index_range range;
  typedef boost::general_storage_order<3> storage;

}

#endif /* _IMAGE_STACK_H_ */
