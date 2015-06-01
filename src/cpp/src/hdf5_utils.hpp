#ifndef _HDF5_UTILS_H_
#define _HDF5_UTILS_H_

#include <limits>
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

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

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U16LE; }
    
  };

  template <>
  struct hdf5_dtype<short>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I16LE; }
    
  };

  template <>
  struct hdf5_dtype<unsigned char>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U8LE; }
    
  };
  
  template <>
  struct hdf5_dtype<char>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I8LE; }
    
  };

  template <>
  struct hdf5_dtype<int>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I32LE; }
    
  };

  template <>
  struct hdf5_dtype<unsigned int>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U32LE; }
    
  };


  
  template <typename T>
  bool h5_read_type_matches(const H5::DataSet& _ds){

    bool value = false;
    int score = 0;

    H5T_class_t type_class = _ds.getTypeClass();

    if(!(_ds.getIntType().getSize() == sizeof(T)))
      score += 1;

    if(!(type_class == H5T_INTEGER && std::numeric_limits<T>::is_integer))
      score += 1;
    
    const H5T_sign_t fsign = _ds.getIntType().getSign();
    if(!((fsign != 0) == std::numeric_limits<T>::is_signed))
      score += 1;

    
    return score == 0;
  }
  
  struct h5_file
  {
    bfs::path		path_;
    H5::H5File*		file_;
    H5::DataSpace*	dataspace_;
    H5::DataSet*	dataset_;

    H5::Exception	error_;
    
    bool		ready_;

   
    
    h5_file(const std::string& _fname, unsigned _flag = H5F_ACC_RDONLY):
      path_(_fname),
      file_(0),
      dataspace_(0),
      dataset_(0),
      ready_(false){

      ready_ = open(_fname, _flag);
      
      
    }


    bool open(const std::string& _fname, unsigned flag){

      try {
	file_ = new H5::H5File(_fname, flag);
      }
      catch(H5::FileIException & local_error)
      {
	error_ = local_error;
	return false;
      }
      catch(H5::Exception & local_error){
	error_ = local_error;
	return false;
      }
      
      return true;
    }
    
    ~h5_file(){
      if(dataspace_)
	delete dataspace_;

      if(dataset_)
	delete dataset_;
      
      file_->close();
      delete file_;
    }

    bool ready() const {
      return ready_ && (file_ != 0);
    }

    bool has_dataset(const std::string& _dname){

      //TODO: is there any way to query the h5file for an object?
      try {
	H5::DataSet ds(file_->openDataSet( _dname ));
      }
      catch(H5::DataSetIException & error){
	return false;
      }
      catch(H5::Exception & error){
	return false;
      }
      
      return true;
    }

    template <typename T, typename U>
    int load_nd_dataset(const std::string& _dname, std::vector<T>& _payload, std::vector<U>& _shape){

      int rvalue = 1;
      if(!_payload.empty())
	_payload.clear();

      if(!_shape.empty())
	_shape.clear();

      if(ready() && has_dataset(_dname))
	dataset_ = new H5::DataSet(file_->openDataSet( _dname ));

      if(!dataset_)
	return 1;

      try{
	dataspace_ = new H5::DataSpace(dataset_->getSpace());
      }
      catch(H5::DataSpaceIException & local_error)
	{
	  error_ = local_error;
	  return 1;
	}
      
      unsigned rank = dataspace_->getSimpleExtentNdims();
      std::vector<hsize_t> retrieved_dims(rank);
      int ndims = dataspace_->getSimpleExtentDims( (hsize_t*)&retrieved_dims[0], NULL);

      hsize_t nelements = std::accumulate(retrieved_dims.begin(), retrieved_dims.end(),1.,std::multiplies<hsize_t>());

      //check for type match
      if(!h5_read_type_matches<T>(*dataset_)){
	rvalue = 1;
	return rvalue;
      }
	
      _payload.resize(nelements);
      try{
	dataset_->read(&_payload[0], hdf5_dtype<T>::instance() );
      }
      catch(H5::Exception & local_error)
	{
	  error_ = local_error;
	  return 1;
	}
      
      _shape.resize(rank);
      std::copy(retrieved_dims.begin(), retrieved_dims.end(), _shape.begin());

      if(!_shape.empty() && !_payload.empty())
	rvalue = 0;
      
      return rvalue;
    }

    template <typename T, typename U>
    int store_nd_dataset(const std::string& _dname, const std::vector<T>& _payload, const std::vector<U>& _shape){

      int rvalue = 1;
      std::vector<hsize_t> dims(_shape.begin(), _shape.end());
      std::vector<hsize_t> chunk_shape(dims);
      
      if(!dataspace_)
	delete dataspace_;

      dataspace_ = new H5::DataSpace(dims.size(), &dims[0]);
      H5::DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);

      if(!dataset_)
	delete dataset_;

      dataset_ = new H5::DataSet(file_->createDataSet( _dname, 
						       hdf5_dtype<T>::instance(),
						       *dataspace_, 
						       plist) );

      dataset_->write(&_payload[0],
		     hdf5_dtype<T>::instance()
		     );

      rvalue = 0;
      return rvalue;
      
    }
    
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

  /**
     \brief extract shape information from H5::DataSpace
     
     \param[in] _dsp pointer to H5::DataSpace
     \param[out] _shape vector of arbitrary type to fill (will return empty if nothing is found)
     
     \return 
     \retval 
     
  */
  template <typename T>
  void read_h5_shape(const H5::DataSpace& _dsp, std::vector<T>& _shape){

    _shape.clear();
    std::vector<hsize_t> temp_shape;

    try {
      temp_shape.resize(_dsp.getSimpleExtentNdims());
      _dsp.getSimpleExtentDims( &temp_shape[0]);
    }
    catch( H5::DataSpaceIException & error ){
      error.printError();
      return;
    }

    _shape.resize(temp_shape.size());
    std::copy(temp_shape.begin(), temp_shape.end(),_shape.begin());

    
  }

  

  
  /**
     \brief function to read nD array of given shape to hdf5 file 
     
     \param[in] _fname file path and name to write to
     \param[in] _dname dataset name inside hdf5 file to store payload under
     \param[inout] _payload pointer of correct type pointing to nD array in memory
     \param[inout] _shape std::vector that defines the shape of _payload in its native data type
     
     \return 
     \retval 
     
  */
  template <typename data_type, typename size_type>
  int read_h5(const std::string& _fname,
	      const std::string& _dname,
	      std::vector<data_type>& _payload,
	      std::vector<size_type>& _shape){

    int value = 1;
    bfs::path h5file = _fname;

    if(!boost::filesystem::exists(h5file)){
      std::cerr << "input file " << h5file << " does not exist!\n";
      return value;
    }

    try{
      H5::H5File file(_fname, H5F_ACC_RDONLY);
      H5::DataSet ds(file.openDataSet( _dname ));

      //check if type found is correct
      if(h5_read_type_matches<data_type>(ds)){
	value = 0;
      }

      H5::DataSpace dsp(ds.getSpace( ));
      
      //extract the shape of the dataset
      read_h5_shape(dsp,_shape);
      value += _shape.size() ? 0 : 10;

      //resize payload
      unsigned long nelements = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());
      _payload.clear();
      _payload.resize(nelements);

      //perform the read
      H5::DataSpace memspace( dsp );
      ds.read( &_payload[0], hdf5_dtype<data_type>::instance() , memspace, dsp );
      
      file.close();
    }
    catch( H5::Exception & e){
      e.printError();
      return -1;
    }
    
    
    return value;
  }

  
}


#endif /* _HDF5_UTILS_H_ */
