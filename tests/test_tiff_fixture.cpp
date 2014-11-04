#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_TIFF_FIXTURE
#include "boost/test/unit_test.hpp"
#include "bench_fixtures.hpp"
#include "tiff_utils.h"
#include "boost/filesystem.hpp"

//FIXME: Refactor this with CMAKE
const static std::string path_to_8bit_stack = "/scratch/steinbac/sqeasy_data/original/Volvis_arteries256.tif";
const static std::string path_to_16bit_stack = "/scratch/steinbac/sqeasy_data/original/Svoboda_Colon_small.tif";

TIFF* open_tiff_at(const std::string& _path){
  
  boost::filesystem::path incoming = _path;
  if(!boost::filesystem::exists(incoming)){
      std::stringstream msg("");
      msg << "unable to load file at path " << _path << "\n";
      throw std::runtime_error(msg.str().c_str());
    }

  return TIFFOpen( _path.c_str() , "r" );
  
}

BOOST_AUTO_TEST_SUITE( tiff_fixture_interface )

BOOST_AUTO_TEST_CASE( castable )
{
  sqeazy_bench::data_interface* ptr = new sqeazy_bench::tiff_fixture<unsigned short>();
  BOOST_CHECK_EQUAL(ptr->size_in_byte(),0);
}

BOOST_AUTO_TEST_CASE( loads_through_interface )
{
  sqeazy_bench::data_interface* ptr = new sqeazy_bench::tiff_fixture<unsigned char>();
  ptr->fill_from(path_to_8bit_stack);
  BOOST_CHECK_NE(ptr->size_in_byte(),0);
}

BOOST_AUTO_TEST_CASE( throws_through_interface  )
{
    sqeazy_bench::data_interface* ptr = new sqeazy_bench::tiff_fixture<unsigned short>();

  BOOST_CHECK_THROW(  ptr->fill_from(path_to_8bit_stack), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( tiff_utils )

BOOST_AUTO_TEST_CASE( gives_right_bits_per_sample )
{

  TIFF* handle = open_tiff_at(path_to_8bit_stack);
  
  int nbits = sqeazy_bench::get_tiff_bits_per_sample(handle);

  TIFFClose(handle);

  BOOST_CHECK_EQUAL(nbits,8);

  
  handle = open_tiff_at(path_to_16bit_stack);
  
  nbits = sqeazy_bench::get_tiff_bits_per_sample(handle);

  TIFFClose(handle);

  BOOST_CHECK_EQUAL(nbits,16);
}

BOOST_AUTO_TEST_CASE( void_input )
{
  BOOST_CHECK_EQUAL(0,sqeazy_bench::get_tiff_bits_per_sample(0));
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( tiff_facet )

BOOST_AUTO_TEST_CASE( instantiated )
{
  sqeazy_bench::tiff_facet handle;
  BOOST_CHECK_EQUAL(handle.empty(),true);
}

BOOST_AUTO_TEST_CASE( given_filename )
{
  sqeazy_bench::tiff_facet handle( path_to_8bit_stack );
  BOOST_CHECK_EQUAL(handle.empty(),false);
}

BOOST_AUTO_TEST_CASE( bits_per_sample )
{
  sqeazy_bench::tiff_facet handle( path_to_8bit_stack );
  BOOST_CHECK_NE(handle.bits_per_sample(),0);
}

BOOST_AUTO_TEST_CASE( dims_extracted )
{
  sqeazy_bench::tiff_facet handle( path_to_8bit_stack );
  std::vector<int> dims;
  handle.dimensions(dims);
  BOOST_CHECK_EQUAL(dims.size(),3);
  BOOST_CHECK_EQUAL(dims[0],256);
  BOOST_CHECK_EQUAL(dims[1],256);
  BOOST_CHECK_EQUAL(dims[2],256);
}


BOOST_AUTO_TEST_CASE( dims_not_extracted_on_void )
{
  sqeazy_bench::tiff_facet handle;
  std::vector<int> dims;
  handle.dimensions(dims);
  BOOST_CHECK_EQUAL(dims.size(),0);

}


BOOST_AUTO_TEST_CASE( reload )
{
  sqeazy_bench::tiff_facet handle;

  handle.reload(path_to_8bit_stack);

  BOOST_CHECK_EQUAL(handle.empty(),false);
  BOOST_CHECK_EQUAL(handle.size_in_byte(),256*256*256);

  handle.reload("");
  BOOST_CHECK_EQUAL(handle.empty(),true);
  BOOST_CHECK_EQUAL(handle.size_in_byte(),0);
  
}

BOOST_AUTO_TEST_SUITE_END()


