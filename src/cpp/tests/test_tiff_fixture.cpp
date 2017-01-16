#define BOOST_TEST_MODULE TEST_TIFF_FIXTURE
#include "boost/test/unit_test.hpp"
#include "tiff_utils.hpp"
#include "tiff_fixtures.hpp"
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
  sqeazy::data_interface* ptr = new sqeazy::tiff_fixture<unsigned short>();
  BOOST_CHECK_EQUAL(ptr->size_in_byte(),0);
}

BOOST_AUTO_TEST_CASE( loads_through_interface )
{
  sqeazy::data_interface* ptr = new sqeazy::tiff_fixture<unsigned char>();
  ptr->fill_from(path_to_8bit_stack);
  BOOST_CHECK_NE(ptr->size_in_byte(),0);
}

BOOST_AUTO_TEST_CASE( throws_through_interface  )
{
    sqeazy::data_interface* ptr = new sqeazy::tiff_fixture<unsigned short>();

  BOOST_CHECK_THROW(  ptr->fill_from(path_to_8bit_stack), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( tiff_utils )

BOOST_AUTO_TEST_CASE( gives_right_bits_per_sample )
{

  TIFF* handle = open_tiff_at(path_to_8bit_stack);
  
  int nbits = sqeazy::get_tiff_bits_per_sample(handle);

  TIFFClose(handle);

  BOOST_CHECK_EQUAL(nbits,8);

  
  handle = open_tiff_at(path_to_16bit_stack);
  
  nbits = sqeazy::get_tiff_bits_per_sample(handle);

  TIFFClose(handle);

  BOOST_CHECK_EQUAL(nbits,16);
}

BOOST_AUTO_TEST_CASE( void_input )
{
  BOOST_CHECK_EQUAL(0,sqeazy::get_tiff_bits_per_sample(0));
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( tiff_facet )

BOOST_AUTO_TEST_CASE( instantiated )
{
  sqeazy::tiff_facet handle;
  BOOST_CHECK_EQUAL(handle.empty(),true);
}

BOOST_AUTO_TEST_CASE( given_filename )
{
  sqeazy::tiff_facet handle( path_to_8bit_stack );
  BOOST_CHECK_EQUAL(handle.empty(),false);
}

BOOST_AUTO_TEST_CASE( bits_per_sample )
{
  sqeazy::tiff_facet handle( path_to_8bit_stack );
  BOOST_CHECK_NE(handle.bits_per_sample(),0);
}

BOOST_AUTO_TEST_CASE( dims_extracted )
{
  sqeazy::tiff_facet handle( path_to_8bit_stack );
  std::vector<int> dims;
  handle.dimensions(dims);
  BOOST_CHECK_EQUAL(dims.size(),3);
  BOOST_CHECK_EQUAL(dims[0],256);
  BOOST_CHECK_EQUAL(dims[1],256);
  BOOST_CHECK_EQUAL(dims[2],256);
}


BOOST_AUTO_TEST_CASE( dims_not_extracted_on_void )
{
  sqeazy::tiff_facet handle;
  std::vector<int> dims;
  handle.dimensions(dims);
  BOOST_CHECK_EQUAL(dims.size(),0);

}


BOOST_AUTO_TEST_CASE( reload )
{
  sqeazy::tiff_facet handle;

  handle.load(path_to_8bit_stack);

  BOOST_CHECK_EQUAL(handle.empty(),false);
  BOOST_CHECK_EQUAL(handle.size_in_byte(),256*256*256);

  handle.load("");
  BOOST_CHECK_EQUAL(handle.empty(),true);
  BOOST_CHECK_EQUAL(handle.size_in_byte(),0);
  
}

BOOST_AUTO_TEST_CASE( load_data_to_buffer_8bit )
{
  sqeazy::tiff_facet handle;

  handle.load(path_to_8bit_stack);
  
  sqeazy::tiff_fixture<char> reference(path_to_8bit_stack);  
  BOOST_CHECK_EQUAL_COLLECTIONS(handle.data(), handle.data() + handle.size_in_byte(),
			       reference.data(), reference.data() + handle.size_in_byte());
}

BOOST_AUTO_TEST_CASE( load_data_to_buffer_16bit )
{
  sqeazy::tiff_facet handle;

  handle.load(path_to_16bit_stack);
  
  sqeazy::tiff_fixture<unsigned short> reference(path_to_16bit_stack);  
  const char* ref_data = reinterpret_cast<const char*>(reference.data());
  BOOST_CHECK_EQUAL_COLLECTIONS(handle.data(), handle.data() + handle.size_in_byte(),
			       ref_data, ref_data + handle.size_in_byte());
}

BOOST_AUTO_TEST_CASE( write_to_tiff_native_type )
{
  sqeazy::tiff_fixture<unsigned short> reference(path_to_16bit_stack);  

  sqeazy::tiff_facet handle(reference.data(), reference.data() + reference.size(),
				  reference.axis_lengths);

  boost::filesystem::path name = path_to_16bit_stack;
  unsigned long long size_16bit_stack = boost::filesystem::file_size(name);
  name.replace_extension("_native_test.tif");
 
  handle.write(name.string(), 16);
  
  double similar = boost::filesystem::file_size(name)/double(size_16bit_stack);
  BOOST_CHECK_GT(similar,.95);

}

BOOST_AUTO_TEST_CASE( write_to_tiff_8bit_type )
{
  sqeazy::tiff_fixture<unsigned char> reference(path_to_8bit_stack);  

  sqeazy::tiff_facet handle(reference.data(), reference.data() + reference.size(),
				  reference.axis_lengths);

  boost::filesystem::path name = path_to_8bit_stack;
  unsigned long long size_8bit_stack = boost::filesystem::file_size(name);
  name.replace_extension("_native_test.tif");
 
  handle.write(name.string(), 8);
  
  double similar = boost::filesystem::file_size(name)/double(size_8bit_stack);
  BOOST_CHECK_GT(similar,.95);

}
BOOST_AUTO_TEST_SUITE_END()


