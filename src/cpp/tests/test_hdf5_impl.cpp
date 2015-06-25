#define BOOST_TEST_MODULE TEST_HDF5_IMPLEMENTATION
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>
#include <stdexcept>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

#include "hdf5_utils.hpp"
#include "hdf5_test_utils.hpp"
#include "hdf5_fixtures.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

namespace bfs = boost::filesystem;


BOOST_FIXTURE_TEST_SUITE( hdf5_utils, helpers_fixture )

BOOST_AUTO_TEST_CASE( static_type_instance_works ){

  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<unsigned short>::instance().getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<short>::instance().getSize() == 2);

  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<unsigned char>::instance().getSize() == 1);
  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<char>::instance().getSize() == 1);

}

BOOST_AUTO_TEST_CASE( runtime_type_instance_works ){

  BOOST_CHECK_NO_THROW(sqeazy::hdf5_runtime_dtype::instance(typeid(short).name()));
  BOOST_CHECK_NO_THROW(sqeazy::hdf5_runtime_dtype::instance(typeid(unsigned short).name()));

  BOOST_CHECK(sqeazy::hdf5_runtime_dtype::instance(typeid(short).name()).getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_runtime_dtype::instance(typeid(unsigned short).name()).getSize() == 2);

}

BOOST_AUTO_TEST_CASE( load_dataset_anew ){
  
  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_REQUIRE_EQUAL(testme.load_h5_dataset(dname),0);
  
}

BOOST_AUTO_TEST_CASE( load_dataset_wrong_name ){
  
  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.load_h5_dataset(dname + "_foo"));
  BOOST_CHECK(!testme.dataset_);
  
}

BOOST_AUTO_TEST_CASE( query_for_dataset ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.has_dataset(dname));
    
}

BOOST_AUTO_TEST_CASE( query_for_dataset_sizeof ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.type_size_in_byte(dname) == 4);
  
}

BOOST_AUTO_TEST_CASE( query_for_dataset_is_float ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(!testme.is_float(dname));
  
}

BOOST_AUTO_TEST_CASE( query_for_dataset_is_int ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.is_integer(dname));
  
}

BOOST_AUTO_TEST_CASE( query_for_dataset_shape ){

  std::vector<int> shape;
  
  sqeazy::h5_file testme(tfile);
  testme.shape(shape,dname);
  
  BOOST_REQUIRE_EQUAL(shape.size(), 2);
  BOOST_CHECK_EQUAL(shape[0], 5);
  BOOST_CHECK_EQUAL(shape[1], 6);
}



BOOST_AUTO_TEST_CASE( load_dataset ){

  std::vector<int> retrieved;
  std::vector<unsigned int> dims;
  
  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());

  int rvalue = testme.read_nd_dataset(dname,retrieved,dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_CHECK(dims[0] == 5);
  BOOST_CHECK(dims[1] == 6);
  BOOST_CHECK(retrieved[1] == 1);
  BOOST_CHECK(retrieved[retrieved.size()-1] == 9);
  BOOST_CHECK(retrieved[12] == 2);
}


BOOST_AUTO_TEST_CASE( write_dataset ){

  std::vector<int> retrieved(64,42);
  std::vector<unsigned int> dims(3,4);
  
  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  int rvalue = testme.write_nd_dataset(dname,retrieved,dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);
}


BOOST_AUTO_TEST_CASE( write_dataset_with_filter ){

  std::vector<unsigned short> retrieved(64,42);
  std::vector<unsigned int> dims(3,4);

  bfs::path no_filter_path = "no_filter.h5";
  sqeazy::h5_file no_filter(no_filter_path.string(), H5F_ACC_TRUNC);
  int rvalue = no_filter.write_nd_dataset(dname,
					   retrieved,
					   dims);

  BOOST_REQUIRE(rvalue == 0);
  //does the write occur here? or at destruction of the object
  
  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);

  rvalue = testme.write_nd_dataset(dname,
				   retrieved,
				   dims,
				   sqeazy::bswap1_lz4_pipe());
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path), bfs::file_size(test_output_path));

  std::vector<unsigned short> written(64,128);
  std::vector<unsigned int> written_shape;
  rvalue = testme.read_nd_dataset(dname,
				  written,
				  written_shape);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_EQUAL(written_shape.size(),dims.size());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_shape.begin(),written_shape.end(),dims.begin(),dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written.begin(),written.end(),retrieved.begin(),retrieved.end());
  
  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(no_filter_path))
    bfs::remove(no_filter_path);
}

BOOST_AUTO_TEST_CASE( write_compressed_dataset ){

  std::vector<unsigned short> retrieved(504,42);
  std::vector<unsigned int> dims(3,8);
  dims[0] -= 1;
  dims[2] += 1;

  //write in one go
  bfs::path one_go_path = "one_go_write.h5";
  sqeazy::h5_file* one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_TRUNC);
  int rvalue = one_go->write_nd_dataset(dname,
				   retrieved,
				   dims,
				   sqeazy::bswap1_lz4_pipe());
  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);

  //write in 2 steps
  //1. compress
  sqeazy::image_header hdr(sqeazy::bswap1_lz4_pipe::raw_type(),
			   dims,
			   sqeazy::bswap1_lz4_pipe::name());
  unsigned long max_size_compressed = sqeazy::bswap1_lz4_pipe::max_bytes_encoded(retrieved.size()*sizeof(unsigned short),
										 hdr.size());
  std::vector<char> compressed(max_size_compressed);
  unsigned long compressed_bytes = 0;
  rvalue = sqeazy::bswap1_lz4_pipe::compress(&retrieved[0], &compressed[0], dims, compressed_bytes);
  compressed.resize(compressed_bytes);

  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_GT(compressed_bytes,0);
    
  sqeazy::h5_file* testme = new sqeazy::h5_file(test_output_name, H5F_ACC_TRUNC);

  rvalue = testme->write_compressed_buffer(dname,
					   &compressed[0],
					   compressed.size());
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));
  BOOST_REQUIRE_CLOSE_FRACTION(float(bfs::file_size(one_go_path)), float(bfs::file_size(test_output_path)),0.1);

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(one_go_path))
    bfs::remove(one_go_path);

}

BOOST_AUTO_TEST_CASE( roundtrip_compressed_dataset ){

  
  
  std::vector<unsigned short> retrieved(64,42);
  std::vector<unsigned short> to_store(64,0);
  for(unsigned i = 0;i<to_store.size();++i)
    to_store[i] = i;

  std::vector<unsigned int> dims(3,4);

  //write in one go
  bfs::path one_go_path = "one_go_write.h5";
  sqeazy::h5_file* one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_TRUNC);
  int rvalue = one_go->write_nd_dataset(dname,
				   retrieved,
				   dims,
				   sqeazy::bswap1_lz4_pipe());

  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);

  //write in 2 steps
  //1. compress
  sqeazy::image_header hdr(sqeazy::bswap1_lz4_pipe::raw_type(),
			   dims,
			   sqeazy::bswap1_lz4_pipe::name());
  unsigned long max_size_compressed = sqeazy::bswap1_lz4_pipe::max_bytes_encoded(retrieved.size()*sizeof(unsigned short),
										 hdr.size());
  std::vector<char> compressed(max_size_compressed);
  unsigned long compressed_bytes = 0;
  rvalue = sqeazy::bswap1_lz4_pipe::compress(&retrieved[0], &compressed[0], dims, compressed_bytes);
  compressed.resize(compressed_bytes);

  std::vector<char> to_write(compressed.begin(),compressed.begin() + compressed_bytes);
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_GT(compressed_bytes,0);
  
  sqeazy::h5_file* testme = new sqeazy::h5_file(test_output_name, H5F_ACC_TRUNC);

  rvalue = testme->write_compressed_buffer(dname,
					   &to_write[0],
					   to_write.size());
  delete testme;
  testme = 0;
  BOOST_REQUIRE(rvalue == 0);

  //read in written dataset with standard methods
  std::vector<unsigned short> written(64,0);
  std::vector<unsigned int> written_shape;
  testme = new sqeazy::h5_file(test_output_name, H5F_ACC_RDONLY);
  rvalue = testme->read_nd_dataset(dname,
				  written,
				  written_shape);
  delete testme;
  testme = 0;
  BOOST_REQUIRE(rvalue == 0);

  std::vector<unsigned short> expected(64,0);
  std::vector<unsigned int> expected_shape;
  one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_RDONLY);
  rvalue = one_go->read_nd_dataset(dname,
				  expected,
				  expected_shape);
  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);
  
  BOOST_REQUIRE_EQUAL(written_shape.size(),dims.size());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_shape.begin(),written_shape.end(),dims.begin(),dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_shape.begin(),written_shape.end(),expected_shape.begin(),expected_shape.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(retrieved.begin(),retrieved.end(),expected.begin(),expected.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written.begin(),written.end(),expected.begin(),expected.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written.begin(),written.end(),retrieved.begin(),retrieved.end());
  
  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(one_go_path))
    bfs::remove(one_go_path);

}

BOOST_AUTO_TEST_CASE( roundtrip_dataset_with_filter ){

  std::vector<unsigned short> to_store(64,0);
  for(unsigned i = 0;i<to_store.size();++i)
    to_store[i] = i;
  
  std::vector<unsigned int> dims(3,4);
  
  sqeazy::loaded_hdf5_plugin now;
  sqeazy::h5_file* testme = new sqeazy::h5_file(test_output_name, H5F_ACC_TRUNC);

  int rvalue = testme->write_nd_dataset(dname,
				       to_store,
				       dims,
				       sqeazy::bswap1_lz4_pipe());

  delete testme;
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));

  
  std::vector<unsigned short> retrieved;
  std::vector<unsigned int> shape;

  sqeazy::h5_file testme_ro(test_output_name);
  rvalue = testme_ro.read_nd_dataset(dname, retrieved, shape);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL(shape.size(), dims.size());
  BOOST_REQUIRE_EQUAL(retrieved.size(), to_store.size());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(to_store.begin(), to_store.end(), retrieved.begin(), retrieved.end());
  
  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

}

BOOST_AUTO_TEST_CASE( loading_to_wrong_type_produces_error ){
  std::vector<unsigned short> to_store(64,42);
  std::vector<unsigned int> dims(3,4);
    
  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);

  int rvalue = testme.write_nd_dataset(dname,
				       to_store,
				       dims,
				       sqeazy::bswap1_lz4_pipe());
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));

  
  std::vector<unsigned char> retrieved;
  std::vector<unsigned int> shape;

  sqeazy::h5_file testme_ro(test_output_name);
  rvalue = testme_ro.read_nd_dataset(dname, retrieved, shape);

  BOOST_REQUIRE_EQUAL(rvalue,1);
  
  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);
}

BOOST_AUTO_TEST_SUITE_END()


  
BOOST_FIXTURE_TEST_SUITE( hdf5_functions_on_uint16, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint16_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned short>(test_output_name.string(), dname, &constant_cube[0], dims);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_CHECK(bfs::exists(test_output_name));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(test_output_name.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(test_output_name.string(), dname) == 2);
  
  bfs::remove(test_output_name);
  
}

BOOST_AUTO_TEST_CASE( roundtrip_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint16_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned short>(test_output_name.string(), dname, &constant_cube[0], dims);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  
  std::vector<int> read_shape;
  to_play_with.clear();

  rvalue = sqeazy::read_h5<unsigned short>(test_output_name.string(), dname, to_play_with, read_shape);
  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  read_shape.begin(), read_shape.end());
  
  bfs::remove(test_output_name);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( hdf5_uint8_suite, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( write_h5_no_filter ){

  const bfs::path test_output_name = "hdf5_uint8_no_filter.h5";
  std::string dname = boost::unit_test::framework::current_test_case().p_name;

  int rvalue = sqeazy::write_h5<unsigned char>(test_output_name.string(), dname, &constant_cube[0], dims);


  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_CHECK(bfs::exists(test_output_name));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name.string(),dname));

  std::vector<int> stored_shape;
  
  dataset_shape_in_h5_file(test_output_name.string(),dname,stored_shape);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(dims.begin(), dims.end(),
				  stored_shape.begin(), stored_shape.end());
  BOOST_REQUIRE(stored_sizeof_in_h5_file(test_output_name.string(), dname) == 1);
  
  bfs::remove(test_output_name);
  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( hdf5_filter_available, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( filter_available ){

  sqeazy::loaded_hdf5_plugin now;
  htri_t avail;
  avail = H5Zfilter_avail(H5Z_FILTER_SQY);

  BOOST_CHECK(avail);
  
}

BOOST_AUTO_TEST_CASE( filter_supports_encoding ){

  sqeazy::loaded_hdf5_plugin now;
  unsigned filter_config = 0;
  H5Zget_filter_info (H5Z_FILTER_SQY, &filter_config);
  
  BOOST_CHECK(filter_config & H5Z_FILTER_CONFIG_ENCODE_ENABLED);
  
}

BOOST_AUTO_TEST_CASE( filter_supports_decoding ){

  sqeazy::loaded_hdf5_plugin now;
  unsigned filter_config = 0;
  H5Zget_filter_info (H5Z_FILTER_SQY, &filter_config);
  
  BOOST_CHECK(filter_config & H5Z_FILTER_CONFIG_DECODE_ENABLED);
  
}


BOOST_AUTO_TEST_SUITE_END()
