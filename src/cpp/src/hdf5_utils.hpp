#ifndef _HDF5_UTILS_H_
#define _HDF5_UTILS_H_

#include "H5Cpp.h"
// #include "sqeazy_impl.hpp"
// #include "pipeline.hpp"
#include "sqeazy_predef_pipelines.hpp"

namespace sqeazy {

  /**
     \brief compile-time utility to handle datatype instantiation at runtime
     
     \param[in] T type of data 
     
     \return instance of h5 datatype class to hand over to other functions
     \retval 
     
  */
  template <typename T>
  struct hdf5_dtype{
    
    // will throw an error at compilation as no default value is defined
    
  };

  template <>
  struct hdf5_dtype<unsigned short>{

    static H5::PredType instance() { return H5::PredType::STD_U16LE; }
    
  };

  template <>
  struct hdf5_dtype<short>{

    static H5::PredType instance() { return H5::PredType::STD_I16LE; }
    
  };

  template <>
  struct hdf5_dtype<unsigned char>{

    static H5::PredType instance() { return H5::PredType::STD_U8LE; }
    
  };
  
  template <>
  struct hdf5_dtype<char>{

    static H5::PredType instance() { return H5::PredType::STD_I8LE; }
    
  };

  /**
     \brief function to write nD array of given shape to hdf5 file using sqeazy pipeline
     
     \param[in] _fname file path and name to write to
     \param[in] _dname dataset name inside hdf5 file to store payload under
     \param[in] _payload pointer of correct type pointing to nD array in memory
     \param[in] _shape std::vector that defines the shape of _payload in its native data type
     \param[in] _pipe sqeazy compression pipeline to use

     \return 
     \retval 
     
  */
  template <typename data_type, typename size_type, typename pipe_type>
  int write_h5(const std::string& _fname,
	       const std::string& _dname,
	       const data_type* _payload,
	       const std::vector<size_type>& _shape,
	       pipe_type _pipe){

    std::vector<hsize_t> dims(_shape.begin(), _shape.end());
    std::vector<hsize_t> chunk_shape(dims);
    static const bool _use_filter = pipe_type::name().size() > 0;
    int rvalue = 0;
    
    try
      {
	// Turn off the auto-printing when failure occurs so that we can
	// handle the errors appropriately

	// Create a new file using the default property lists. 
	H5::H5File file(_fname, H5F_ACC_TRUNC);

	// Create the data space for the dataset.
	H5::DataSpace dataspace(_shape.size(), &dims[0]);

	// // Modify dataset creation property to enable chunking
	H5::DSetCreatPropList  plist;
	plist.setChunk(chunk_shape.size(), &chunk_shape[0]);
      
	// std::vector<unsigned> cd_values(1,42);

	// if(_use_filter)
	// 	plist.setFilter(H5Z_FILTER_SQY,
	// 			 H5Z_FLAG_MANDATORY,
	// 			 cd_values.size(),
	// 			 &cd_values[0]);
	// Create the dataset.      
	H5::DataSet*  dataset = new H5::DataSet(file.createDataSet( _dname, 
								hdf5_dtype<data_type>::instance(),
								dataspace, 
								plist) );

	// Write data to dataset.
	dataset->write(&_payload[0],
		       hdf5_dtype<data_type>::instance()
		       );

	// Close objects and file.  Either approach will close the HDF5 item.
	//    delete dataspace;
	delete dataset;
	//    delete plist;
	file.close();
      }
    // catch failure caused by the H5File operations
    catch(H5::FileIException & error)
      {
	error.printError();
	std::cout << __LINE__ << ": caught FileIException("<< _fname << ":" << _dname
		  <<"):\t" << error.getDetailMsg() << "\n";
	
	return 1;
      }
    
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException & error)
      {
	error.printError();
	std::cout << __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		  <<"):\t" << error.getDetailMsg() << "\n";
	return 1;
      }
    
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException & error)
      {
	error.printError();
	std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		  <<"):\t" << error.getDetailMsg() << "\n";
	return 1;
      }
    
    catch(H5::Exception & error){
      error.printError();
      std::cout << __LINE__ << ": caught Exception("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
      
    }
    
    return rvalue;
  }
  
  /**
     \brief function to write nD array of given shape to hdf5 file 
     
     \param[in] _fname file path and name to write to
     \param[in] _dname dataset name inside hdf5 file to store payload under
     \param[in] _payload pointer of correct type pointing to nD array in memory
     \param[in] _shape std::vector that defines the shape of _payload in its native data type
     
     \return 
     \retval 
     
  */
  template <typename data_type, typename size_type>
  int write_h5(const std::string& _fname,
	       const std::string& _dname,
	       const data_type* _payload,
	       const std::vector<size_type>& _shape){

    static const passthrough_pipe default_pipe;
    
    return write_h5(_fname, _dname, _payload, _shape, default_pipe);
  }
}


#endif /* _HDF5_UTILS_H_ */
