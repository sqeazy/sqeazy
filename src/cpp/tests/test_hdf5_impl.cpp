#define BOOST_TEST_MODULE TEST_HDF5_IMPLEMENTATION

#ifdef _WIN32
#ifdef _SQY_DEBUG
#define _CRTDBG_MAP_ALLOC 1
#endif
#endif

#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdint>

#include <cstdlib>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

#include "hdf5_utils.hpp"
#include "hdf5_test_utils.hpp"
#include "hdf5_fixtures.hpp"

#include "sqeazy_pipelines.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

namespace bfs = boost::filesystem;
namespace sqy = sqeazy;

static const std::string default_pipeline = "bitswap1->lz4";

static sqy::dypeline<std::uint16_t> pipe16 = sqy::dypeline<std::uint16_t>::from_string(default_pipeline);
static sqy::dypeline_from_uint8 pipe8 = sqy::dypeline_from_uint8::from_string(default_pipeline);

BOOST_FIXTURE_TEST_SUITE( hdf5_utils, helpers_fixture )

BOOST_AUTO_TEST_CASE( pipes_are_filled ){

  BOOST_CHECK_NE(pipe16.size(),0);
  BOOST_CHECK_NE(pipe8.size(),0);

}

  
BOOST_AUTO_TEST_CASE( static_type_instance_works ){

  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<std::uint16_t>::instance().getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<std::int16_t>::instance().getSize() == 2);

  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<unsigned char>::instance().getSize() == 1);
  BOOST_CHECK(sqeazy::hdf5_compiletime_dtype<char>::instance().getSize() == 1);

}

BOOST_AUTO_TEST_CASE( runtime_type_instance_works ){

  BOOST_CHECK_NO_THROW(sqeazy::hdf5_runtime_dtype::instance(typeid(std::int16_t).name()));
  BOOST_CHECK_NO_THROW(sqeazy::hdf5_runtime_dtype::instance(typeid(std::uint16_t).name()));

  BOOST_CHECK(sqeazy::hdf5_runtime_dtype::instance(typeid(std::int16_t).name()).getSize() == 2);
  BOOST_CHECK(sqeazy::hdf5_runtime_dtype::instance(typeid(std::uint16_t).name()).getSize() == 2);

}

BOOST_AUTO_TEST_CASE( extract_group ){
  
  std::string path = "/my_group/dataset_name";
  std::string grp = sqeazy::extract_group_path(path);

  BOOST_REQUIRE(!grp.empty());
  BOOST_CHECK(grp[0] == '/');
  BOOST_CHECK(grp.find("my_group")!=std::string::npos);
  BOOST_CHECK_EQUAL(grp,"/my_group");
  BOOST_CHECK_NE(grp[grp.size()-1],'/');

  grp = sqeazy::extract_group_path("dataset_name");
  BOOST_REQUIRE(!grp.empty());
  BOOST_CHECK(grp[0] == '/');
  BOOST_CHECK_EQUAL(grp.size(),1);

  path = path.substr(1);
  grp = sqeazy::extract_group_path(path);

  BOOST_REQUIRE(!grp.empty());
  BOOST_CHECK(grp[0] == '/');
  BOOST_CHECK(grp.find("my_group")!=std::string::npos);
  BOOST_CHECK_EQUAL(grp,"/my_group");
  BOOST_CHECK_NE(grp[grp.size()-1],'/');
}


BOOST_AUTO_TEST_CASE( open_close ){
  
  
  sqeazy::h5_file testme(tpath);
  
  BOOST_CHECK(testme.ready());
  H5::DataSet tds = testme.load_h5_dataset(dname);
  BOOST_REQUIRE_NE(tds.getStorageSize(),0);
  BOOST_REQUIRE_NE(tds.getId(),0);

  testme.close();
  BOOST_CHECK(testme.has_h5_item(dname)==false);

  testme.open(tfile);
  
  sqeazy::h5_file testme_ro = testme;
  BOOST_CHECK(testme_ro.ready());
  H5::DataSet tds_ro = testme_ro.load_h5_dataset(dname);
  BOOST_REQUIRE_NE(tds_ro.getStorageSize(),0);
  BOOST_REQUIRE_NE(tds_ro.getId(),0);

}

BOOST_AUTO_TEST_CASE( load_dataset_anew ){
  
  sqeazy::h5_file testme(tpath);
  BOOST_CHECK(testme.ready());
  H5::DataSet tds = testme.load_h5_dataset(dname);
  BOOST_REQUIRE_NE(tds.getStorageSize(),0);
  
}

BOOST_AUTO_TEST_CASE( load_dataset_wrong_name ){
  
  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());

  std::stringstream newname("");
  newname << dname << "_foo";
  
  H5::DataSet tds = testme.load_h5_dataset(newname.str());

  if(H5_VERSION_GE(1,8,14))
    BOOST_CHECK_LT(tds.getId(),0);
  else
    BOOST_CHECK_EQUAL(tds.getId(),0);

  
}

BOOST_AUTO_TEST_CASE( query_for_dataset ){

  sqeazy::h5_file testme(tfile);
  BOOST_CHECK(testme.ready());
  BOOST_CHECK(testme.has_h5_item(dname));
    
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

  std::vector<int> from_disk(30,0);
  
  sqeazy::h5_file testme(tpath);
  BOOST_CHECK(testme.ready());

  int rvalue = testme.read_nd_dataset(dname,from_disk,dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_CHECK(dims[0] == 5);
  BOOST_CHECK(dims[1] == 6);
  BOOST_CHECK(from_disk[1] == 1);
  BOOST_CHECK(from_disk[from_disk.size()-1] == 9);
  BOOST_CHECK(from_disk[12] == 2);
}


BOOST_AUTO_TEST_CASE( write_dataset ){

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  sqeazy::h5_file testme(test_output_path, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  int rvalue = testme.write_nd_dataset(dname,retrieved,dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));

}

BOOST_AUTO_TEST_CASE( write_dataset_to_group ){

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  std::string dname_with_group = "/new_level/";
  dname_with_group += dname;
  int rvalue = testme.write_nd_dataset(dname_with_group,retrieved,dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname_with_group));

  
}

BOOST_AUTO_TEST_CASE( write_multiple_datasets ){

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  std::string dname2 = "AnyOther";

  int rvalue = testme.write_nd_dataset(dname2,retrieved,dims);

  std::vector<uint16_t> shuffled(retrieved);
  std::srand(shuffled.size());
  
  for(unsigned i = 0;i<shuffled.size();++i){
    unsigned ri = float(std::rand())*shuffled.size()/RAND_MAX;
    shuffled[ri] = retrieved[i];
  }
    
  rvalue = testme.write_nd_dataset(dname,shuffled,dims);
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname2));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));

}

BOOST_AUTO_TEST_CASE( write_multiple_datasets_to_groups ){

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  std::string dname2 = "/level2/AnyOther";
  std::string dname1 = "/level1/";dname1+=dname;

  int rvalue = testme.write_nd_dataset(dname2,retrieved,dims);

  std::vector<uint16_t> shuffled(retrieved);
  std::srand(shuffled.size());
  
  for(unsigned i = 0;i<shuffled.size();++i){
    unsigned ri = float(std::rand())*shuffled.size()/RAND_MAX;
    shuffled[ri] = retrieved[i];
  }
    
  rvalue = testme.write_nd_dataset(dname1,shuffled,dims);
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname2));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname1));

}

BOOST_AUTO_TEST_CASE( roundtrip_multiple_datasets_in_groups ){

	if(bfs::exists(test_output_path))
		bfs::remove(test_output_path);

  sqeazy::h5_file testme(test_output_name, H5F_ACC_TRUNC);
  BOOST_CHECK(testme.ready());

  std::string dname2 = "/level2/AnyOther";
  std::string dname1 = "/level1/";dname1+=dname;

  int rvalue = testme.write_nd_dataset(dname1,retrieved,dims);

  std::vector<uint16_t> shuffled(retrieved);
  std::srand(shuffled.size());
  
  for(unsigned i = 0;i<shuffled.size();++i){
    unsigned ri = float(std::rand())*shuffled.size()/RAND_MAX;
    shuffled[ri] = retrieved[i];
  }
    
  rvalue = testme.write_nd_dataset(dname2,shuffled,dims);
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname2));
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname1));

  testme.close();
  
  std::vector<uint16_t> reloaded_shuffled(shuffled.size(),0);  
  std::vector<uint16_t> reloaded_retrieved(retrieved.size(),0);  
  std::vector<uint32_t> reloaded_dims;

  testme = sqeazy::h5_file(test_output_name);
  
  rvalue = testme.read_nd_dataset(dname1,reloaded_retrieved,reloaded_dims);
  
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reloaded_dims.begin(), reloaded_dims.end(),
				  dims.begin(), dims.end()
				  );
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reloaded_retrieved.begin(), reloaded_retrieved.end(),
				  retrieved.begin(), retrieved.end()
				  );

  std::fill(reloaded_dims.begin(), reloaded_dims.end(),0);
  
  rvalue = testme.read_nd_dataset(dname2,reloaded_shuffled,reloaded_dims);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reloaded_dims.begin(), reloaded_dims.end(),
				  dims.begin(), dims.end()
				  );
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reloaded_shuffled.begin(), reloaded_shuffled.end(),
				  shuffled.begin(), shuffled.end()
				  );
  
  
}

BOOST_AUTO_TEST_CASE( write_dataset_with_filter ){

  bfs::path no_filter_path = "no_filter.h5";
  
  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(no_filter_path))
    bfs::remove(no_filter_path);
  

  sqeazy::h5_file no_filter(no_filter_path.string(), H5F_ACC_TRUNC);
  int rvalue = no_filter.write_nd_dataset(dname,
					   retrieved,
					   dims);

  BOOST_REQUIRE(rvalue == 0);
  //does the write occur here? or at destruction of the object
  
  sqeazy::h5_file testme(test_output_path, H5F_ACC_TRUNC);

  rvalue = testme.write_nd_dataset(dname,
				   retrieved,
				   dims,
				   pipe16);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path), bfs::file_size(test_output_path));

}

BOOST_AUTO_TEST_CASE( read_write_dataset_with_filter ){

	bfs::path no_filter_path = "no_filter.h5";
	if(bfs::exists(test_output_path))
		bfs::remove(test_output_path);

	if(bfs::exists(no_filter_path))
		bfs::remove(no_filter_path);


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
				   pipe16);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE(dataset_in_h5_file(test_output_name,dname));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path), bfs::file_size(test_output_path));

  std::vector<std::uint16_t> written(64,128);
  std::vector<unsigned int> written_shape;
  rvalue = testme.read_nd_dataset(dname,
				  written,
				  written_shape);
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_EQUAL(written_shape.size(),dims.size());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_shape.begin(),written_shape.end(),dims.begin(),dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written.begin(),written.end(),retrieved.begin(),retrieved.end());
  
  
}


BOOST_AUTO_TEST_CASE( write_compressed_dataset ){

	bfs::path one_go_path = "one_go_write.h5";

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(one_go_path))
    bfs::remove(one_go_path);


  //write in one go

  sqeazy::h5_file* one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_TRUNC);
  int rvalue = one_go->write_nd_dataset(dname,
				   retrieved,
				   dims,
				   pipe16);
  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);

  //write in 2 steps
  //1st compress
  sqeazy::image_header hdr(std::uint16_t(),
			   dims,
			   pipe16.name());
  
  unsigned long max_size_compressed = pipe16.max_encoded_size(retrieved.size()*sizeof(std::uint16_t));
  std::vector<char> compressed(max_size_compressed);

  char* encoded_end = pipe16.encode(&retrieved[0],
				    &compressed[0],
				    dims);
  unsigned long compressed_bytes = encoded_end - &compressed[0];
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


}

BOOST_AUTO_TEST_CASE( roundtrip_compressed_dataset ){

	bfs::path one_go_path = "one_go_write.h5";

  if(bfs::exists(test_output_path))
    bfs::remove(test_output_path);

  if(bfs::exists(one_go_path))
    bfs::remove(one_go_path);
  

  //write in one go

  sqeazy::h5_file* one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_TRUNC);
  int rvalue = one_go->write_nd_dataset(dname,
				   retrieved,
				   dims,
				   pipe16);

  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);

  //write in 2 steps
  //1. compress
  sqeazy::image_header hdr(value_type(),
			   dims,
			   pipe16.name());
  
  unsigned long max_size_compressed = pipe16.max_encoded_size(retrieved.size()*sizeof(std::uint16_t));
  std::vector<char> compressed(max_size_compressed);

  char* encoded_end = pipe16.encode(&retrieved[0],
				    &compressed[0],
				    dims);
  unsigned long compressed_bytes = encoded_end - &compressed[0];
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
  std::vector<std::uint16_t> written_compressed;
  std::vector<unsigned int> written_compressed_shape;
  testme = new sqeazy::h5_file(test_output_name, H5F_ACC_RDONLY);
  rvalue = testme->read_nd_dataset(dname,
				  written_compressed,
				  written_compressed_shape);
  delete testme;
  testme = 0;
  BOOST_REQUIRE(rvalue == 0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_compressed_shape.begin(),written_compressed_shape.end(),dims.begin(),dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_compressed.begin(),written_compressed.end(),retrieved.begin(),retrieved.end());
    
  std::vector<std::uint16_t> encoded_on_write;
  std::vector<unsigned int> encoded_on_write_shape;
  one_go = new sqeazy::h5_file(one_go_path.string(), H5F_ACC_RDONLY);
  rvalue = one_go->read_nd_dataset(dname,
				  encoded_on_write,
				  encoded_on_write_shape);
  delete one_go;
  one_go = 0;
  BOOST_REQUIRE(rvalue == 0);
  
  BOOST_REQUIRE_EQUAL(written_compressed_shape.size(),dims.size());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_compressed_shape.begin(),written_compressed_shape.end(),encoded_on_write_shape.begin(),encoded_on_write_shape.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(retrieved.begin(),retrieved.end(),encoded_on_write.begin(),encoded_on_write.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(written_compressed.begin(),written_compressed.end(),encoded_on_write.begin(),encoded_on_write.end());


}

BOOST_AUTO_TEST_CASE( roundtrip_bswap1_lz4_as_reference ){

  int rvalue = 0;
  sqeazy::image_header hdr(value_type(),
			   dims,
			   pipe16.name());
  unsigned long max_size_compressed = pipe16.max_encoded_size(retrieved.size()*sizeof(std::uint16_t));
  std::vector<char> compressed(max_size_compressed);

  char* encoded_end = pipe16.encode(&retrieved[0],
				    &compressed[0],
				    dims);
  unsigned long compressed_bytes = encoded_end - &compressed[0];
  compressed.resize(compressed_bytes);
  
  std::vector<char> to_decompress(compressed.begin(), compressed.begin() + compressed_bytes);
    
  BOOST_REQUIRE_EQUAL(rvalue,0);

  std::vector<std::uint16_t> decompressed(retrieved.size(),0);
  rvalue = pipe16.decode(to_decompress.data(),
			 decompressed.data(),
			 compressed_bytes);
  
  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(retrieved.begin(),retrieved.end(),decompressed.begin(),decompressed.end());

}


BOOST_AUTO_TEST_SUITE_END()
