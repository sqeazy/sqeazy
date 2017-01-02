#define BOOST_TEST_MODULE TEST_TILE_SHUFFLE_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>

#include "array_fixtures.hpp"

#include "encoders/tile_shuffle_scheme_impl.hpp"
#include "traits.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint16_t,32> uint16_cube_of_32;

namespace sqy = sqeazy;
namespace sqyd = sqy::detail;

/**
   \brief function to create a stack where each tile yields the same voxel intensity
   e.g. in 4x4 2D frame would then look like:
   [ 0 0 1 1 ]
   [ 0 0 1 1 ]
   [ 2 2 3 3 ]
   [ 2 2 3 3 ]
   here each tile is 2x2 and contains only one value, namely the tile id in row-major order

   \param[in] 

   \return 
   \retval 

*/
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

/**
   \brief function to create a stack where each tile yields the same voxel intensity
   e.g. in 4x4 2D frame would then look like:
   [ 3 3 2 2 ]
   [ 3 3 2 2 ]
   [ 1 1 0 0 ]
   [ 1 1 0 0 ]
   here each tile is 2x2 and contains only one value, namely the tile id in row-major order in reverse order

   \param[in] 

   \return 
   \retval 

*/
template <typename iterator_t, typename shape_t>
void label_stack_by_tile_reverse(iterator_t _begin,
				 const shape_t& _shape,
				 std::uint32_t tile_size){

  typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
  typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

  shape_t n_tiles_per_dim = _shape;
  for(shape_value_t& el : n_tiles_per_dim){
    el = el/tile_size;
  }

  shape_value_t n_tiles = std::accumulate(n_tiles_per_dim.begin(), n_tiles_per_dim.end(),1,std::multiplies<shape_value_t>());

  if(!n_tiles){
    std::cerr << "Yikes, no tiles can be created from [";
    for(shape_value_t& el : n_tiles_per_dim)
      std::cerr << el << " ";
    std::cerr << "]\n";
    return;
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

	*(_begin++) = n_tiles - tile_id - 1;
	
      }
    }
  }
	
}

BOOST_FIXTURE_TEST_SUITE( reverse_stack_labels , uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( first_tile_correct )
{

  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,4);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[0],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3],7);

  BOOST_CHECK_EQUAL(incrementing_cube.data()[0+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3+64],7);

  BOOST_CHECK_EQUAL(incrementing_cube.data()[0+8],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1+8],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2+8],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3+8],7);

  BOOST_CHECK_EQUAL(incrementing_cube.data()[0+8+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[1+8+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[2+8+64],7);
  BOOST_CHECK_EQUAL(incrementing_cube.data()[3+8+64],7);

}

BOOST_AUTO_TEST_CASE( occurance_of_label )
{

  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,2);

  auto cnt_3 = std::count(incrementing_cube.begin(), incrementing_cube.end(),3);
  BOOST_CHECK_EQUAL(cnt_3,8);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( basic , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( constructs )
{

  sqyd::tile_shuffle in_tiles_of(4);
  BOOST_CHECK(in_tiles_of.tile_size == 4);


}

BOOST_AUTO_TEST_CASE( has_0_remainder )
{

  sqyd::tile_shuffle in_tiles_of(4);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),0);
  
  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
				expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( has_non0_remainder )
{

  sqyd::tile_shuffle in_tiles_of(6);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),2);
  
  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
				expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( on_const_value_buffer )
{

  sqyd::tile_shuffle in_tiles_of(4);
  auto rem = in_tiles_of.encode(constant_cube.cbegin(), constant_cube.cend(),
			       to_play_with.begin(),
			       dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
				to_play_with.begin(), to_play_with.end()); 


}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( just_encode , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( reverse )
{
  auto expected = incrementing_cube;
  auto exp_itr = expected.data();
  std::size_t n_elements_per_tile = std::pow(4,3);
  for(std::uint16_t i = 0;i<8;++i,exp_itr+=n_elements_per_tile){
    
      std::fill(exp_itr,exp_itr+n_elements_per_tile,i);
  }
  
  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,4);
  sqyd::tile_shuffle in_tiles_of(4);
  BOOST_CHECK(in_tiles_of.tile_size == 4);

  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_REQUIRE_NE(to_play_with[0], to_play_with[n_elements_per_tile]);
 
  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+4,
				  expected.begin(), expected.begin()+4);
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
				expected.begin(), expected.end());

}

BOOST_AUTO_TEST_CASE( reverse_2 )
{
  auto expected = incrementing_cube;
  auto exp_itr = expected.data();
  std::size_t n_elements_per_tile = std::pow(2,3);
  for(std::uint16_t i = 0;i<64;++i,exp_itr+=n_elements_per_tile){
    
      std::fill(exp_itr,exp_itr+n_elements_per_tile,i);
  }
  
  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,2);
  sqyd::tile_shuffle in_tiles_of(2);

  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_REQUIRE_NE(to_play_with[0], to_play_with[n_elements_per_tile]);
  
  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+4,
				  expected.begin(), expected.begin()+4);
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
				expected.begin(), expected.end());

}

BOOST_AUTO_TEST_CASE( reverse_3 )
{

  // std::size_t n_elements_per_tile = std::pow(3,3);


  
  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,4);
  sqyd::tile_shuffle in_tiles_of(3);

  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  //check only the first tile
  std::vector<std::uint16_t> expected_last_tile(9,7);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_play_with.begin()+to_play_with.size()-4,
				  to_play_with.end(),
				  expected_last_tile.begin(), expected_last_tile.begin()+4);

  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin()+to_play_with.size()-expected_last_tile.size(),
				to_play_with.end(),
  				expected_last_tile.begin(), expected_last_tile.end());

  //check that zeros are at the beginning
  BOOST_CHECK_GT(std::count(to_play_with.begin(),
			    to_play_with.end(),0),0);

  //check that encoded data yields low values at the start and high ones in the back
  auto checksum_start = std::accumulate(to_play_with.begin(),to_play_with.begin()+8,0);
  auto checksum_mid = std::accumulate(to_play_with.begin()+to_play_with.size()/2,to_play_with.begin()+to_play_with.size()/2+8,0);
  auto checksum_end = std::accumulate(to_play_with.end()-8,to_play_with.end(),0);
  BOOST_CHECK_GT(checksum_end,checksum_start);
  BOOST_CHECK_GT(checksum_end,checksum_mid);
  BOOST_CHECK_GT(checksum_mid,checksum_start);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( roundtrips , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( reverse )
{

  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,4);
  auto expected = incrementing_cube;
  
  sqyd::tile_shuffle in_tiles_of(4);
  BOOST_CHECK(in_tiles_of.tile_size == 4);

  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_tiles_of.decode(to_play_with.begin(), to_play_with.end(),
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

  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,2);
  auto expected = incrementing_cube;
  
  sqyd::tile_shuffle in_tiles_of(2);


  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_tiles_of.decode(to_play_with.begin(), to_play_with.end(),
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

  label_stack_by_tile_reverse(incrementing_cube.begin(),dims,4);
  auto expected = incrementing_cube;
  
  sqyd::tile_shuffle in_tiles_of(3);


  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto dec_rem = in_tiles_of.decode(to_play_with.begin(), to_play_with.end(),
				    incrementing_cube.begin(),
				    dims);

  BOOST_REQUIRE(dec_rem == incrementing_cube.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.begin()+16,
				  incrementing_cube.begin(), incrementing_cube.begin()+16);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
				  incrementing_cube.begin(), incrementing_cube.end()); 

}


BOOST_AUTO_TEST_SUITE_END()
