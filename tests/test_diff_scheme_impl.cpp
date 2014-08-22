#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::diff_scheme<unsigned short> local_diff_scheme ;


BOOST_FIXTURE_TEST_SUITE( offset_calculation, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( offset_called )
{
  const unsigned axis_size = axis_length;
  std::vector<unsigned> offsets;

  sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
  geometry.compute_offsets_in_x(offsets);
  
  BOOST_CHECK_GT(offsets.size(),0);
  unsigned expected = (axis_size-1)*(axis_size-1);
  BOOST_CHECK_EQUAL(offsets.size(),expected);
}


BOOST_AUTO_TEST_CASE( offset_exact )
{
  const unsigned axis_size = axis_length;
  std::vector<unsigned> offsets;
  
  sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
  geometry.compute_offsets_in_x(offsets);

  for(int dim_idx = 0;dim_idx<3;++dim_idx){
    BOOST_CHECK_EQUAL(geometry.non_halo_begin(dim_idx),1);
    BOOST_CHECK_EQUAL(geometry.non_halo_end(dim_idx),axis_size);
  }

  BOOST_CHECK_EQUAL(offsets.at(0),axis_size*axis_size + axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.at(1),axis_size*axis_size + 2*axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.back(),(axis_size-1)*axis_size*axis_size + ((axis_size-1))*axis_size + 1);
}

BOOST_AUTO_TEST_CASE( offset_exact_last_plane )
{
  const unsigned axis_size = axis_length;
  std::vector<unsigned> offsets;
  
  sqeazy::halo<sqeazy::last_plane_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
  geometry.compute_offsets_in_x(offsets);

  for(int dim_idx = 0;dim_idx<2;++dim_idx){
    try{
      BOOST_REQUIRE_EQUAL(geometry.non_halo_begin(dim_idx),1);
    }
    catch(...){
      std::cerr << "offset_exact_last_plane failed!\n"
		<< "non_halo_begin(" << dim_idx << "): \t " << geometry.non_halo_begin(dim_idx)
		<< ",\texpected " << 1 << "\n";
    }
    
    try{
      BOOST_REQUIRE_EQUAL(geometry.non_halo_end(dim_idx),axis_size-1);
    }
    catch(...){
      std::cerr << "offset_exact_last_plane failed!\n"
		<< "non_halo_end(" << dim_idx << "): \t " << geometry.non_halo_end(dim_idx)
		<< ",\texpected " << axis_size-1 << "\n";
    }
  }

  BOOST_CHECK_EQUAL(geometry.non_halo_begin(2),1);
  BOOST_CHECK_EQUAL(geometry.non_halo_end(2),axis_size);
  

  BOOST_CHECK_EQUAL(offsets.at(0),axis_size*axis_size + axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.size(),(axis_size-1)*(axis_size-2));
  BOOST_CHECK_EQUAL(offsets.at(1),axis_size*axis_size + 2*axis_size + 1);
  BOOST_CHECK_EQUAL(offsets.back(),(axis_size-1)*axis_size*axis_size + ((axis_size-1-1))*axis_size + 1);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( offset_calculation, uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( Neighborhood_size )
{
//   sqeazy::last_plane_neighborhood<3> local;
  unsigned traversed_pixels = sqeazy::num_traversed_pixels<sqeazy::last_plane_neighborhood<3> >();
  BOOST_CHECK_GT(traversed_pixels,0);
  BOOST_CHECK_EQUAL(traversed_pixels,9);
  
}
BOOST_AUTO_TEST_SUITE_END()