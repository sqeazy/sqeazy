#define BOOST_TEST_MODULE TEST_MEMORY_REORDER_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
// #include "encoders/memory_reorder_utils.hpp"
#include "encoders/raster_reorder_scheme_impl.hpp"
#include "traits.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint16_t,32> uint16_cube_of_32;

namespace sqy = sqeazy;
namespace sqyd = sqy::detail;

template <typename iterator_t, typename shape_t>
void label_stack_by_tile(iterator_t _begin,
             const shape_t& _shape,
             std::uint32_t tile_size){

  typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
  typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

  shape_t n_tiles_per_dim = _shape;
  for(shape_value_t& el : n_tiles_per_dim){
    el = el/tile_size;
  }

  shape_value_t ztile = 0;
  shape_value_t ytile = 0;
  shape_value_t xtile = 0;

  shape_value_t tile_id = 0;
  for(shape_value_t z = 0;z<_shape[sqeazy::row_major::z];++z){
    ztile = z / tile_size;

    for(shape_value_t y = 0;y<_shape[sqeazy::row_major::y];++y){
      ytile = y / tile_size;

      for(shape_value_t x = 0;x<_shape[sqeazy::row_major::x];++x){
    xtile = x/ tile_size;

    tile_id = ztile*n_tiles_per_dim[sqeazy::row_major::y]*n_tiles_per_dim[sqeazy::row_major::x]
      + ytile*n_tiles_per_dim[sqeazy::row_major::x]
      + xtile;

    *(_begin++) = tile_id;

      }
    }
  }

}

template <typename iterator_t, typename shape_t>
void encoded_tile_labels(iterator_t _begin,
             const shape_t& _shape,
             std::uint32_t tile_size){

  typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
  typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

  for(shape_value_t el : _shape){
    if(el % tile_size != 0){
      std::cerr << "unable to generate encoded_tile_labels for shape ("
        << _shape[sqeazy::row_major::z] << "x" << _shape[sqeazy::row_major::y] << "x" << _shape[sqeazy::row_major::x]
        << ") at tile_size = " << tile_size << "\n";
      return;
    }

  }

  shape_t n_tiles_per_dim = _shape;
  for(shape_value_t& el : n_tiles_per_dim){
    el = el/tile_size;
  }

  shape_value_t n_tiles_size = std::accumulate(n_tiles_per_dim.begin(), n_tiles_per_dim.end(),1,std::multiplies<shape_value_t>());
  shape_value_t n_elements_per_tile = std::pow(tile_size,_shape.size());
  shape_value_t n_elements = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<shape_value_t>());

  iterator_t _end = _begin + n_elements;
  shape_value_t tile_id =0;
  for(;_begin!=_end;_begin+=n_elements_per_tile){
    std::fill(_begin, _begin + n_elements_per_tile, tile_id);
    tile_id+=1;
  }

}


BOOST_FIXTURE_TEST_SUITE( basic , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( constructs )
{

  sqyd::reorder in_tiles_of(4);
  BOOST_CHECK(in_tiles_of.tile_size == 4);


}

BOOST_AUTO_TEST_CASE( has_0_remainder )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),0);

  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( has_non0_remainder )
{

  sqyd::reorder in_tiles_of(6);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),2);

  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( on_const_value_buffer )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(constant_cube.cbegin(), constant_cube.cend(),
                   to_play_with.begin(),
                   dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                to_play_with.begin(), to_play_with.end());


}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( on_ramp_forward , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  label_stack_by_tile(incrementing_cube.begin(),dims,2);

  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = constant_cube;
  encoded_tile_labels(expected.begin(),
              dims,
              in_tiles_of.tile_size);

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                expected.begin(), expected.end());


}



BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                   to_play_with.begin(),
                   dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = {0,1,2,3,
                     8,9,10,11,
                     16,17,18,19,
                     24,25,26,27,
                     64,65,66,67,
                     72,73,74,75,
                     80,81,82,83,
                     88,89,90,91
  };

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( tile_of_4_labelled )
{

  sqyd::reorder in_tiles_of(4);
  label_stack_by_tile(incrementing_cube.begin(),dims,in_tiles_of.tile_size);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                   to_play_with.begin(),
                   dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  // std::vector<std::uint16_t> expected = {0,1,2,3,
  //                     8,9,10,11,
  //                     16,17,18,19,
  //                     24,25,26,27,
  //                     64,65,66,67,
  //                     72,73,74,75,
  //                     80,81,82,83,
  //                     88,89,90,91
  // };
  auto expected = constant_cube;
  encoded_tile_labels(expected.begin(),
              dims,
              in_tiles_of.tile_size);

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                expected.begin(), expected.end());


}


BOOST_AUTO_TEST_CASE( tile_of_3 )
{

  sqyd::reorder in_tiles_of(3);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                   to_play_with.begin(),
                   dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = {0,1,2,
                     8,9,10,
                     16,17,18,
                     64,65,66,
                     72,73,74,
                     80,81,82
  };

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
                expected.begin(), expected.end());


}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( on_ramp_backward , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  std::vector<std::uint16_t> expected = {0,1,
                     8,9,
                     64,65,
                     72,73};
  std::fill(constant_cube.begin(), constant_cube.end(),0);
  std::copy(expected.begin(), expected.end(),constant_cube.begin());

  auto rem = in_tiles_of.decode(constant_cube.cbegin(), constant_cube.cend(),
                to_play_with.begin(),
                dims);

  BOOST_REQUIRE(rem == to_play_with.end());

  for(int i = 0;i<8;++i)
    BOOST_CHECK_EQUAL(to_play_with[expected[i]],expected[i]);


}

BOOST_AUTO_TEST_CASE( tile_of_2_labelled )
{

  sqyd::reorder in_tiles_of(2);
  auto encoded = constant_cube;
  std::fill(encoded.begin(), encoded.end(),0);
  encoded_tile_labels(encoded.begin(),
              dims,
              in_tiles_of.tile_size);

  auto expected = constant_cube;
  std::fill(expected.begin(), expected.end(),0);
  label_stack_by_tile(expected.begin(),dims,in_tiles_of.tile_size);


  auto rem = in_tiles_of.decode(encoded.cbegin(), encoded.cend(),
                to_play_with.begin(),
                dims);

  BOOST_REQUIRE(rem == to_play_with.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.end(),
                expected.begin(), expected.end());

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( on_ramp_roundtrip , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
               decoded.begin(),
               dims);
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
               decoded.begin(),
               dims);
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( tile_of_3 )
{

  sqyd::reorder in_tiles_of(3);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                to_play_with.begin(),
                dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
               decoded.begin(),
               dims);
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( scheme_tile_of_4 )
{


  sqeazy::raster_reorder_scheme<value_type> scheme("tile_size=4");

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  auto rem = scheme.encode(incrementing_cube.data(),
               to_play_with.data(),
               shape);
  BOOST_REQUIRE(rem == (to_play_with.data() + to_play_with.size()));

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  auto rv = scheme.decode(to_play_with.data(),
              decoded.data(),
              shape);

  BOOST_REQUIRE(rv == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

// BOOST_AUTO_TEST_CASE( tile_of_3 )
// {

//   sqyd::reorder in_tiles_of(3);
//   auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
//              to_play_with.begin(),
//              dims);
//   BOOST_REQUIRE(rem == to_play_with.end());

//   auto decoded = constant_cube;
//   std::fill(decoded.begin(), decoded.end(),0);

//   rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
//             decoded.begin(),
//             dims);
//   BOOST_REQUIRE(rem == decoded.end());
//   BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
//              decoded.begin(), decoded.end());


// }


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( big_roundtrips , uint16_cube_of_32 )
BOOST_AUTO_TEST_CASE( tile_of_simd_fitted )
{

  sqyd::reorder in_tiles_of(16/sizeof(value_type));
  for(unsigned dim : dims)
    BOOST_REQUIRE_EQUAL(dim % in_tiles_of.tile_size,0u);

  auto rem = in_tiles_of.encode_full_simd(incrementing_cube.data(),
                      incrementing_cube.data()+size,
                      to_play_with.data(),
                      dims);
  BOOST_CHECK(rem == (to_play_with.data()+size));

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  auto rv_decode = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
                      decoded.begin(),
                      dims);
  BOOST_CHECK(rv_decode == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( parallel_on_ramp_roundtrip , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                to_play_with.begin(),
                                dims,
                                std::thread::hardware_concurrency());
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
                           decoded.begin(),
                           dims,
                           std::thread::hardware_concurrency()
    );
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                to_play_with.begin(),
                                dims,
                                std::thread::hardware_concurrency());

  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
                           decoded.begin(),
                           dims,
                           std::thread::hardware_concurrency());
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( tile_of_3 )
{

  sqyd::reorder in_tiles_of(3);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
                                to_play_with.begin(),
                                dims,
                                std::thread::hardware_concurrency());
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
                           decoded.begin(),
                           dims,
                           std::thread::hardware_concurrency());
  BOOST_REQUIRE(rem == decoded.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}

BOOST_AUTO_TEST_CASE( scheme_tile_of_4 )
{


  sqeazy::raster_reorder_scheme<value_type> scheme("tile_size=4");
  scheme.set_n_threads(std::thread::hardware_concurrency());

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  auto rem = scheme.encode(incrementing_cube.data(),
               to_play_with.data(),
               shape);
  BOOST_REQUIRE(rem == (to_play_with.data() + to_play_with.size()));

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  auto rv = scheme.decode(to_play_with.data(),
              decoded.data(),
              shape);

  BOOST_REQUIRE(rv == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(incrementing_cube.begin(), incrementing_cube.end(),
                decoded.begin(), decoded.end());


}


BOOST_AUTO_TEST_SUITE_END()
