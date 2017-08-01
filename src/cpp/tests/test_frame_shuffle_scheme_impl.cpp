#define BOOST_TEST_MODULE TEST_FRAME_SHUFFLE_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <string>

#include "array_fixtures.hpp"

#include "encoders/frame_shuffle_scheme_impl.hpp"
#include "traits.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint16_t,32> uint16_cube_of_32;

namespace sqy = sqeazy;
namespace sqyd = sqy::detail;

/**
   \brief function to create a stack where each frame yields the same voxel intensity
   e.g. in a 4x4x4 3D stack this would  look like from the top:
   -  front  -
   [ 0 0 0 0 ]
   [ 1 1 1 1 ]
   [ 2 2 2 2 ]
   [ 3 3 3 3 ]
   -  back  -
   \param[in] 

   \return 
   \retval 

*/
template <typename iterator_t, typename shape_t>
void label_stack_by_frame(iterator_t _begin,
                          const shape_t& _shape,
                          std::uint32_t frame_chunk_size=1){

  typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
  typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

  auto itr = _begin;
  const std::size_t frame_size = _shape[sqeazy::row_major::y]*_shape[sqeazy::row_major::x];
  std::size_t frame_id = 0;
  for(shape_value_t z = 0;z<_shape[sqeazy::row_major::z];z+=frame_chunk_size){

    std::fill(itr, itr + frame_chunk_size*frame_size, frame_id++);
    itr += frame_size*frame_chunk_size;
  }

}

/**
   \brief function to create a stack where each frame yields the same voxel intensity
   e.g. in a 4x4x4 3D stack this would  look like from the top:
   -  front  -
   [ 3 3 3 3 ]
   [ 2 2 2 2 ]
   [ 1 1 1 1 ]
   [ 0 0 0 0 ]
   -  back  -

   \param[in] 

   \return 
   \retval 

*/
template <typename iterator_t, typename shape_t>
void label_stack_by_frame_reverse(iterator_t _begin,
                                  const shape_t& _shape,
                                  std::uint32_t frame_chunk_size=1){

  typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
  typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

  auto itr = _begin;
  const std::size_t frame_size = _shape[sqeazy::row_major::y]*_shape[sqeazy::row_major::x];
  std::size_t frame_id = _shape[sqeazy::row_major::z]/frame_chunk_size;

  for(shape_value_t z = 0;z<_shape[sqeazy::row_major::z];z+=frame_chunk_size){

    std::fill(itr, itr + frame_chunk_size*frame_size, --frame_id);
    itr += frame_size*frame_chunk_size;

  }
}

BOOST_FIXTURE_TEST_SUITE( reverse_stack_labels , uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( first_frame_correct )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,1);

  BOOST_CHECK_EQUAL(incrementing_cube.data()[0],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3],7);

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,2);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[0],3);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1],3);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2],3);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3],3);

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,4);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[0],1);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1],1);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2],1);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3],1);

}

BOOST_AUTO_TEST_CASE( occurance_of_label )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,2);

  auto cnt_3 = std::count(incrementing_cube.begin(), incrementing_cube.end(),3);

  BOOST_CHECK_EQUAL(cnt_3,2*dims[sqeazy::row_major::y]*dims[sqeazy::row_major::x]);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( basic , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( constructs )
{

  sqyd::frame_shuffle in_frames_of(4);
  BOOST_CHECK(in_frames_of.frame_chunk_size == 4);


}

BOOST_AUTO_TEST_CASE( has_0_remainder )
{

  sqyd::frame_shuffle in_frames_of(4);
  auto rem = in_frames_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),0);

  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
                                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( has_non0_remainder )
{

  sqyd::frame_shuffle in_frames_of(6);
  auto rem = in_frames_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),0);
  expected[sqeazy::row_major::z] = 8 % 6;

  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
                                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( on_const_value_buffer )
{

  sqyd::frame_shuffle in_frames_of;
  auto rem = in_frames_of.encode(constant_cube.cbegin(), constant_cube.cend(),
                                 to_play_with.begin(),
                                 dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.begin()+0.1*constant_cube.size(),
                                  to_play_with.begin(), to_play_with.begin()+0.1*to_play_with.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                to_play_with.begin(), to_play_with.end());


}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( just_encode , uint16_cube_of_8 )



BOOST_AUTO_TEST_CASE( identity )
{
  auto expected = incrementing_cube;
  auto exp_itr = expected.data();
  std::size_t n_elements_per_frame = std::pow(4,3);
  for(std::uint16_t i = 0;i<8;++i,exp_itr+=n_elements_per_frame){

      std::fill(exp_itr,exp_itr+n_elements_per_frame,i);
  }

  label_stack_by_frame(incrementing_cube.begin(),dims);

  sqyd::frame_shuffle in_frames_of;

  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                 to_play_with.begin(),
                                 dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_REQUIRE_NE(to_play_with.front(), to_play_with.back());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+4,
                                  expected.begin(), expected.begin()+4);
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                                expected.begin(), expected.end());

}

BOOST_AUTO_TEST_CASE( reverse )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims);
  auto expected = incrementing_cube;
  label_stack_by_frame(expected.begin(),dims);

  sqyd::frame_shuffle in_frames_of;

  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                 to_play_with.begin(),
                                 dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_REQUIRE_NE(to_play_with.front(), to_play_with.back());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+4,
                                  expected.begin(), expected.begin()+4);
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                                expected.begin(), expected.end());

}


BOOST_AUTO_TEST_CASE( reverse_2 )
{
  auto expected = incrementing_cube;
  auto exp_itr = expected.data();

  std::size_t n_voxel_in_frame = frame*2;
  std::size_t n_chunks = (size/frame)/2;

  for(std::size_t i = 0;i<n_chunks;++i){

      std::fill(exp_itr,exp_itr+n_voxel_in_frame,i);
      exp_itr+=n_voxel_in_frame;
  }

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,2);
  sqyd::frame_shuffle in_frames_of(2);

  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_REQUIRE_NE(to_play_with[0], to_play_with[n_voxel_in_frame]);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+4,
                  expected.begin(), expected.begin()+4);
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                expected.begin(), expected.end());

}

BOOST_AUTO_TEST_CASE( reverse_3 )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,1);
  sqyd::frame_shuffle in_frames_of(3);

  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                 to_play_with.begin(),
                                 dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  //check only the first frame
  BOOST_CHECK_EQUAL(to_play_with[0],4);
  BOOST_CHECK_GT(to_play_with[3*frame],to_play_with[0]);

  //check that the last chunk is identical
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.end()-2*frame,to_play_with.end(),
                                incrementing_cube.end()-2*frame,incrementing_cube.end());

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( roundtrips , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( reverse )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,1);
  auto expected = incrementing_cube;

  sqyd::frame_shuffle in_frames_of;

  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                 to_play_with.begin(),
                                 dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_frames_of.decode(to_play_with.begin(), to_play_with.end(),
                                     incrementing_cube.begin(),
                                     dims);

  BOOST_REQUIRE(dec_rem == incrementing_cube.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
                                  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                incrementing_cube.begin(), incrementing_cube.end());

}

BOOST_AUTO_TEST_CASE( reverse_2 )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,2);
  auto expected = incrementing_cube;

  sqyd::frame_shuffle in_frames_of(2);


  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_frames_of.decode(to_play_with.begin(), to_play_with.end(),
                    incrementing_cube.begin(),
                    dims);

  BOOST_REQUIRE(dec_rem == incrementing_cube.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
                  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                  incrementing_cube.begin(), incrementing_cube.end());

}

BOOST_AUTO_TEST_CASE( reverse_3 )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,4);
  auto expected = incrementing_cube;

  sqyd::frame_shuffle in_frames_of(3);


  auto rem = in_frames_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_frames_of.decode(to_play_with.begin(), to_play_with.end(),
                    incrementing_cube.begin(),
                    dims);

  BOOST_REQUIRE(dec_rem == incrementing_cube.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
                  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                  incrementing_cube.begin(), incrementing_cube.end());

}


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( scheme , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( simple )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,4);
  auto expected = incrementing_cube;
  
  sqy::frame_shuffle_scheme<std::uint16_t> scheme;

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  auto rem = scheme.encode(incrementing_cube.data(),
               to_play_with.data(),
               shape);
  
  BOOST_REQUIRE(rem == (to_play_with.data()+to_play_with.size()));
  BOOST_CHECK_NE(scheme.serialized_reorder_map.empty(), true);

}

BOOST_AUTO_TEST_CASE( rt_4 )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,1);
  auto expected = incrementing_cube;

  sqy::frame_shuffle_scheme<std::uint16_t> scheme("frame_size=4");

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  auto rem = scheme.encode(incrementing_cube.data(),
               to_play_with.data(),
               shape);
  BOOST_REQUIRE(rem == (to_play_with.data()+to_play_with.size()));

  auto dec_rem = scheme.decode(to_play_with.data(),
                               incrementing_cube.data(),
                               shape);

  BOOST_REQUIRE_EQUAL(dec_rem, sqy::SUCCESS);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
                                  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                incrementing_cube.begin(), incrementing_cube.end());

}

BOOST_AUTO_TEST_CASE( rt_with_alien_config )
{

  label_stack_by_frame_reverse(incrementing_cube.begin(),dims,4);
  auto expected = incrementing_cube;
  
  sqy::frame_shuffle_scheme<std::uint16_t> scheme("frame_size=4");

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  auto rem = scheme.encode(incrementing_cube.data(),
               to_play_with.data(),
               shape);
  BOOST_REQUIRE(rem == (to_play_with.data()+to_play_with.size()));

  std::string config = scheme.config();
  sqy::frame_shuffle_scheme<std::uint16_t> another(config);
  
  auto dec_rem = another.decode(to_play_with.data(),
                   incrementing_cube.data(),
                   shape);

  BOOST_REQUIRE_EQUAL(dec_rem, sqy::SUCCESS);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
                  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                  incrementing_cube.begin(), incrementing_cube.end());

}


BOOST_AUTO_TEST_SUITE_END()
