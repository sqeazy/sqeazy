#define BOOST_TEST_MODULE TEST_ZCURVE_REORDER_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
// #include "encoders/memory_reorder_utils.hpp"
#include "encoders/zcurve_reorder_scheme_impl.hpp"
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

  sqy::zcurve_reorder_scheme<value_type> morton_of("tile_size=4");
  BOOST_CHECK(morton_of.tile_size == 4);


}


BOOST_AUTO_TEST_CASE( on_buffer )
{

  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqy::zcurve_reorder_scheme<value_type> morton_of4("tile_size=4");
  auto rem = morton_of4.encode(constant_cube.data(),
                               to_play_with.data(),
                               shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,to_play_with.data()+to_play_with.size());

  sqy::zcurve_reorder_scheme<value_type> morton_of1;
  rem = morton_of1.encode(constant_cube.data(),
              to_play_with.data(),
              shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,to_play_with.data()+to_play_with.size());

}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( rt_on_ramp , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  // label_stack_by_tile(incrementing_cube.begin(),dims,2);

  sqy::zcurve_reorder_scheme<value_type> morton_of;
  auto rem = morton_of.encode(incrementing_cube.data(),
           to_play_with.data(),
           shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,to_play_with.data()+to_play_with.size());

  auto res = morton_of.decode(to_play_with.data(),
           constant_cube.data(),
           shape);

  BOOST_REQUIRE_EQUAL(res,0);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.begin()+16,
                                  incrementing_cube.begin(), incrementing_cube.begin()+16);

  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                incrementing_cube.begin(), incrementing_cube.end());


}


BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  // label_stack_by_tile(incrementing_cube.begin(),dims,2);

  sqy::zcurve_reorder_scheme<value_type> morton_of("tile_size=4");
  auto rem = morton_of.encode(incrementing_cube.data(),
           to_play_with.data(),
           shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,to_play_with.data()+to_play_with.size());

  auto res = morton_of.decode(to_play_with.data(),
           constant_cube.data(),
           shape);

  BOOST_REQUIRE_EQUAL(res,0);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.begin()+16,
                                  incrementing_cube.begin(), incrementing_cube.begin()+16);

  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                incrementing_cube.begin(), incrementing_cube.end());


}

BOOST_AUTO_TEST_CASE( tile_of_8 )
{

  std::vector<std::size_t> shape(dims.begin(), dims.end());
  // label_stack_by_tile(incrementing_cube.begin(),dims,2);

  sqy::zcurve_reorder_scheme<value_type> morton_of("tile_size=8");
  auto rem = morton_of.encode(incrementing_cube.data(),
           to_play_with.data(),
           shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,to_play_with.data()+to_play_with.size());

  auto res = morton_of.decode(to_play_with.data(),
           constant_cube.data(),
           shape);

  BOOST_REQUIRE_EQUAL(res,0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                incrementing_cube.begin(), incrementing_cube.end());


}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( odd_shapes  )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  std::vector<std::size_t> shape = {8,16,8};
  std::size_t len = std::accumulate(shape.begin(), shape.end(),
            1.,
            std::multiplies<std::size_t>()
            );

  std::vector<std::uint16_t> src(len,0);
  label_stack_by_tile(src.begin(),shape,2);

  std::vector<std::uint16_t> enc(len,0);
  std::vector<std::uint16_t> dec(len,0);


  sqy::zcurve_reorder_scheme<std::uint16_t> morton_of;
  auto rem = morton_of.encode(src.data(),
          enc.data(),
          shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,enc.data()+enc.size());

  auto res = morton_of.decode(enc.data(),
          dec.data(),
          shape);

  BOOST_REQUIRE_EQUAL(res,0);
  for(std::size_t i = 0;i<src.size();++i)
    BOOST_REQUIRE_MESSAGE(src[i]==dec[i], "odd_shapes::tile_of_2 failed at item "<< i << ", obs: " << dec[i] << " exp: " << src[i]);


}

BOOST_AUTO_TEST_CASE( tile_of_2_prime )
{

  std::vector<std::size_t> shape = {7,16,7};
  std::size_t len = std::accumulate(shape.begin(), shape.end(),
            1.,
            std::multiplies<std::size_t>()
            );

  std::vector<std::uint16_t> src(len,0);
  label_stack_by_tile(src.begin(),shape,2);

  std::vector<std::uint16_t> enc(len,0);
  std::vector<std::uint16_t> dec(len,0);


  sqy::zcurve_reorder_scheme<std::uint16_t> morton_of;
  auto rem = morton_of.encode(src.data(),
          enc.data(),
          shape);
  BOOST_REQUIRE(rem != nullptr);
  BOOST_REQUIRE_EQUAL(rem,enc.data()+enc.size());

  auto res = morton_of.decode(enc.data(),
          dec.data(),
          shape);

  BOOST_REQUIRE_EQUAL(res,0);
  for(std::size_t i = 0;i<src.size();++i)
    BOOST_REQUIRE_MESSAGE(src[i]==dec[i], "odd_shapes::tile_of_2_prime failed at item "<< i << ", obs: " << dec[i] << " exp: " << src[i]);


}

BOOST_AUTO_TEST_SUITE_END()
