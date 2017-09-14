#define BOOST_TEST_MODULE TEST_HDF5_INTERFACE
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"

extern "C" {
#include "sqeazy.h"
}

//taken from
//http://www.hdfgroup.org/ftp/HDF5/current/src/unpacked/c++/examples/h5tutr_cmprss.cpp
#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif


#include "hdf5_fixtures.hpp"



static const std::string default_filter_name = "bitswap1->lz4";

// static sqeazy::loaded_hdf5_plugin always_load;
/*
  as the loaded_hdf5_plugin class is instantiated as a static object,
  the sqy h5 filter will be registered at the start of any program that
  includes this (either by code or by linking against the library).
  The filter will be unregistered once the app/lib finishes running
  or being called.
*/




BOOST_FIXTURE_TEST_SUITE( hdf5_inference_queries, helpers_fixture )

BOOST_AUTO_TEST_CASE( sizeof_dataset ){

  unsigned size_in_byte = 0;
  int rvalue = SQY_h5_query_sizeof(tfile.c_str(), dname.c_str(), &size_in_byte);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(size_in_byte,4u);

}

BOOST_AUTO_TEST_CASE( sizeof_dataset_wrong_input ){


  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned size_in_byte = 0;

  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0u);

  rvalue = SQY_h5_query_sizeof(tfile.c_str(), wrong_dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0u);

  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), wrong_dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0u);

}

BOOST_AUTO_TEST_CASE( dtype_dataset ){

  unsigned dtype = 0;
  int rvalue = SQY_h5_query_dtype(tfile.c_str(), dname.c_str(), &dtype);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(dtype,2u);

}

BOOST_AUTO_TEST_CASE( dtype_dataset_wrong_input ){


  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned dtype = 0;

  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), dname.c_str(), &dtype);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(dtype,0u);


}

BOOST_AUTO_TEST_CASE( rank_dataset ){

  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(ndims,2u);

}

BOOST_AUTO_TEST_CASE( rank_dataset_wrong_input ){

  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned ndims = 0;

  rvalue = SQY_h5_query_ndims(wrong_tfile.c_str(), dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0u);

  rvalue = SQY_h5_query_ndims(tfile.c_str(), wrong_dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0u);

  rvalue = SQY_h5_query_ndims(wrong_tfile.c_str(), wrong_dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0u);

}

BOOST_AUTO_TEST_CASE( shape_dataset ){


  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);
  std::vector<unsigned> dims(ndims);

  rvalue = SQY_h5_query_shape(tfile.c_str(), dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,0);
  BOOST_CHECK_EQUAL(dims[0],5u);
  BOOST_CHECK_EQUAL(dims[1],6u);

}

BOOST_AUTO_TEST_CASE( shape_dataset_wrong_input ){

  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);
  std::vector<unsigned> dims(ndims,0u);


  rvalue = SQY_h5_query_shape(wrong_tfile.c_str(), dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0u);

  rvalue = SQY_h5_query_shape(tfile.c_str(), wrong_dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0u);

  rvalue = SQY_h5_query_shape(wrong_tfile.c_str(), wrong_dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0u);

}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( hdf5_file_io, helpers_fixture )

BOOST_AUTO_TEST_CASE( write_no_filter ){


  uint16_cube_of_8 data;

  int rvalue = 0;

  BOOST_CHECK_NO_THROW(rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
						  dname.c_str(),
						  &data.constant_cube[0],
						  data.dims.size(),
						  &data.dims[0],
                          "")
    );

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0u);
  BOOST_REQUIRE_GT(float(bfs::file_size(test_output_path)),float(data.size_in_byte));


}

BOOST_AUTO_TEST_CASE( roundtrip_no_filter ){


  uint16_cube_of_8 data;

  int rvalue = 0;

  BOOST_CHECK_NO_THROW(rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
						  dname.c_str(),
						  &data.constant_cube[0],
						  data.dims.size(),
						  &data.dims[0],
						  ""));

  BOOST_REQUIRE_EQUAL(rvalue,0);

  data.to_play_with.clear();
  data.to_play_with.resize(data.constant_cube.size());

  BOOST_CHECK_NO_THROW(rvalue = SQY_h5_read_UI16(test_output_name.c_str(),
						 dname.c_str(),
						 &data.to_play_with[0]));

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(), data.to_play_with.end(),
				  data.constant_cube.begin(), data.constant_cube.end());

}

BOOST_AUTO_TEST_CASE( write_filter ){


  uint16_cube_of_8 data;


  int rvalue = SQY_h5_write_UI16(no_filter_path.string().c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 "");

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(no_filter_path));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path),0u);

  rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
			     dname.c_str(),
			     &data.constant_cube[0],
			     data.dims.size(),
			     &data.dims[0],
			     default_filter_name.c_str());

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0u);
  BOOST_REQUIRE_LT(bfs::file_size(test_output_path),bfs::file_size(no_filter_path));

}

BOOST_AUTO_TEST_CASE( roundtrip_filter ){


  uint16_cube_of_8 data;

  int rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 default_filter_name.c_str());

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0u);

  data.to_play_with.clear();
  data.to_play_with.resize(data.constant_cube.size());

  rvalue = SQY_h5_read_UI16(test_output_name.c_str(),
			    dname.c_str(),
			    &data.to_play_with[0]);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(), data.to_play_with.end(),
				  data.constant_cube.begin(), data.constant_cube.end());




}

BOOST_AUTO_TEST_CASE( write_compressed_data ){


  uint16_cube_of_8 data;

  int rvalue = SQY_h5_write_UI16(no_filter_path.string().c_str(),
                                 dname.c_str(),
                                 &data.constant_cube[0],
                                 data.dims.size(),
                                 &data.dims[0],
                                 "lz4");

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(no_filter_path));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path),0u);

  long size = data.size_in_byte;
  SQY_Pipeline_Max_Compressed_Length_UI16("lz4",&size);
  std::vector<char> compressed(size);

  std::vector<long> ldims(data.dims.begin(), data.dims.end());
  rvalue = SQY_PipelineEncode_UI16("lz4",
                                   (const char*)&data.constant_cube[0],
                                   &ldims[0],
                                   ldims.size(),
                                   &compressed[0],
                                   &size,
                                   1
    );


  rvalue = SQY_h5_write(test_output_name.c_str(),
                        dname.c_str(),
                        &compressed[0],
                        size);


  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0u);
  BOOST_REQUIRE_CLOSE_FRACTION(float(bfs::file_size(test_output_path)),float(bfs::file_size(no_filter_path)),.4);

  rvalue = SQY_h5_read_UI16(test_output_name.c_str(),
                            dname.c_str(),
                            &data.to_play_with[0]);

  BOOST_REQUIRE_EQUAL(rvalue,0);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(),  data.to_play_with.begin()+32,
                                  data.constant_cube.begin(), data.constant_cube.begin()+32);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(), data.to_play_with.end(),
                                  data.constant_cube.begin(), data.constant_cube.end());



}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( index_files, indexed_helpers )

BOOST_AUTO_TEST_CASE( fixture_correct ){

  int rvalue = SQY_h5_write_UI16(dataset_paths[0].string().c_str(),
				 dataset_names[0].c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 default_filter_name.c_str());
  BOOST_CHECK_MESSAGE(rvalue == 0, "failed to write " << dataset_paths[0].string() <<":"<<dataset_names[0]);

  rvalue = SQY_h5_write_UI16(dataset_paths[1].string().c_str(),
			     dataset_names[1].c_str(),
			     &data.incrementing_cube[0],
			     data.dims.size(),
			     &data.dims[0],
			     default_filter_name.c_str());
  BOOST_CHECK_MESSAGE(rvalue == 0, "failed to write " << dataset_paths[1].string() <<":"<<dataset_names[1]);


  BOOST_REQUIRE(!bfs::exists(index_file_path));
  for(unsigned i = 0;i<dataset_paths.size();++i){
    BOOST_REQUIRE(bfs::exists(dataset_paths[i]));
    BOOST_REQUIRE(bfs::file_size(dataset_paths[i])>1000u);
  }


  clean_up();
}

BOOST_AUTO_TEST_CASE( index_file_exists ){

  int rvalue = SQY_h5_write_UI16(dataset_paths[0].string().c_str(),
				 dataset_names[0].c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 default_filter_name.c_str());
  BOOST_CHECK_MESSAGE(rvalue == 0, "failed to write " << dataset_paths[0].string() <<":"<<dataset_names[0]);

  std::string link_tail = dataset_names[0].substr(dataset_names[0].rfind("/")+1);
  std::string link_head = dataset_names[0].substr(0,dataset_names[0].rfind("/"));

  std::string dest_tail = link_tail;
  std::string dest_head = link_head;

  SQY_h5_link(index_file_path.string().c_str(),
	      link_head.c_str(),
	      link_tail.c_str(),
	      dataset_paths[0].string().c_str(),
	      dest_head.c_str(),
	      dest_tail.c_str()
	      );

  BOOST_REQUIRE(bfs::exists(index_file_path));

  clean_up();
}

BOOST_AUTO_TEST_CASE( roundtrip_through_index_file ){

  int rvalue = SQY_h5_write_UI16(dataset_paths[0].string().c_str(),
				 dataset_names[0].c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 default_filter_name.c_str());


  BOOST_CHECK_MESSAGE(rvalue == 0, "failed to write " << dataset_paths[0].string() <<":"<<dataset_names[0]);

  std::string link_tail = dataset_names[0].substr(dataset_names[0].rfind("/")+1);
  std::string link_head = dataset_names[0].substr(0,dataset_names[0].rfind("/"));

  std::string dest_tail = link_tail;
  std::string dest_head = link_head;

  rvalue = SQY_h5_link(index_file_path.string().c_str(),
	      link_head.c_str(),
	      link_tail.c_str(),
	      dataset_paths[0].string().c_str(),
	      dest_head.c_str(),
	      dest_tail.c_str()
	      );

  BOOST_REQUIRE(bfs::exists(index_file_path));
  BOOST_CHECK_MESSAGE(rvalue == 0, "failed to create link from " << index_file_path.string() <<":"<< link_head << link_tail << " to " << dataset_paths[0].string() << ":" << dest_head << dest_tail);
  rvalue = SQY_h5_read_UI16(index_file_path.string().c_str(),
			     dataset_names[0].c_str(),
			     &data.to_play_with[0]);

  BOOST_REQUIRE_MESSAGE(rvalue == 0, "failed to read " << index_file_path.string() <<":"<<dataset_names[0]);
  clean_up();
}
BOOST_AUTO_TEST_SUITE_END()
