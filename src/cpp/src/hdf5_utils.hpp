#ifndef _HDF5_UTILS_H_
#define _HDF5_UTILS_H_

#include <limits>
#include <string>
#include <sstream>
#include <map>
#include <cmath>
#include <vector>
#include <set>
#include <typeinfo>
#include "boost/filesystem.hpp"
#include "boost/static_assert.hpp"

namespace bfs = boost::filesystem;

#include "H5Cpp.h"
// #include "sqeazy_impl.hpp"
// #include "pipeline.hpp"
#include "sqeazy_predef_pipelines.hpp"
#include "sqeazy_h5_filter.hpp"
#include "sqeazy_hdf5_impl.hpp"

namespace H5{
  std::string get_ds_name(const H5::DataSet& _ds)
  {
    size_t len = H5Iget_name(_ds.getId(),NULL,0);
    std::string rvalue;rvalue.resize(len);
    H5Iget_name(_ds.getId(),&rvalue[0],len+1);
    return rvalue;
  }

  //TODO: replace by cast operator
  std::string get_id_name(const hid_t& _hid)
  {
    size_t len = H5Iget_name(_hid,NULL,0);
    std::string rvalue;rvalue.resize(len);
    H5Iget_name(_hid,&rvalue[0],len+1);
    return rvalue;
  }
}

    
namespace sqeazy {

  

  
  /**
     \brief compile-time utility to handle datatype instantiation at runtime
     
     \param[in] T type of data 
     
     \return instance of h5 datatype class to hand over to other functions
     \retval 
     
  */
  template <typename T>
  struct hdf5_compiletime_dtype{
    
    // will throw an error at compilation as no default value is defined
    
    
  };

  template <>
  struct hdf5_compiletime_dtype<unsigned short>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U16LE; }
    
  };

  template <>
  struct hdf5_compiletime_dtype<short>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I16LE; }
    
  };

  template <>
  struct hdf5_compiletime_dtype<unsigned char>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U8LE; }
    
  };
  
  template <>
  struct hdf5_compiletime_dtype<char>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I8LE; }
    
  };

  template <>
  struct hdf5_compiletime_dtype<int>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_I32LE; }
    
  };

  template <>
  struct hdf5_compiletime_dtype<unsigned int>{

    typedef H5::IntType stored_type;
    static H5::PredType instance() { return H5::PredType::STD_U32LE; }
    
  };


  struct hdf5_runtime_dtype{

    typedef H5::PredType (*f_type)();
    typedef std::map<std::string,f_type> map_t;
    
    hdf5_runtime_dtype(){
    }

    static void fill(map_t& _map){

      _map[typeid(short).name()] = &hdf5_compiletime_dtype<short>::instance;
      _map[typeid(unsigned short).name()] = &hdf5_compiletime_dtype<unsigned short>::instance;
      _map[typeid(char).name()] = &hdf5_compiletime_dtype<char>::instance;
      _map[typeid(unsigned char).name()] = &hdf5_compiletime_dtype<unsigned char>::instance;
      _map[typeid(int).name()] = &hdf5_compiletime_dtype<int>::instance;
      _map[typeid(unsigned int).name()] = &hdf5_compiletime_dtype<unsigned int>::instance;

    }
    
    static H5::PredType instance(const std::string& _id) {
      static map_t type_map;
      
      if(type_map.empty())
	fill(type_map);
      
      typename map_t::const_iterator found = type_map.find(_id);
      if(found!=type_map.end())
	return found->second();
      else{
	std::stringstream msg;
	msg << "[hdf5_runtime_dtype, "<< __FILE__":"<< __LINE__<<"]\t"
	    << "received unknown type id " << _id << "\n";
	throw std::runtime_error(msg.str());
      }
    }
    
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

    value = score == 0 ;
    return value;
  }

  
  struct h5_file
  {
    bfs::path		path_;
    H5::H5File*		file_;
    // H5::DataSpace*	dataspace_;
    // H5::DataSet*	dataset_;

    H5::Exception	error_;
    
    bool		ready_;
   
    
    h5_file(const std::string& _fname, unsigned _flag = H5F_ACC_RDONLY):
      path_(_fname),
      file_(0),
      // dataspace_(0),
      // dataset_(0),
      ready_(false)
    {

      ready_ = open(_fname, _flag);
      static sqeazy::loaded_hdf5_plugin now;
      
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
      // if(dataspace_)
      // 	delete dataspace_;

      // if(dataset_)
      // 	delete dataset_;

      if(file_)
	file_->close();
      
      delete file_;
    }

    bool ready() const {
      return ready_ && (file_ != 0);
    }

    /**
       \brief load hdf5 dataset from file file_ into return value
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    H5::DataSet load_h5_dataset(const std::string& _dname){

      H5::DataSet value;
      
      //dataset not present in file
      if(!this->has_dataset(_dname)){
	
	std::cerr << "[sqeazy::h5_file, "<< __FILE__":"<< __LINE__<<"]\t"
		  << "requested dataset " << _dname << " not found in "<< path_.string() <"\n";
	return value;
      }


      try {
	value = H5::DataSet(this->file_->openDataSet( _dname ));
      }
      catch(H5::DataSetIException & error){
	return value;
      }
      catch(H5::Exception & error){
	return value;
      }
            
      return value;
    }
    
    bool has_dataset(const std::string& _dname) const {

      //taken from http://stackoverflow.com/a/18468735
      htri_t dataset_status = H5Lexists(file_->getId(), _dname.c_str(), H5P_DEFAULT);
      return (dataset_status>0);
      
    }

    int type_size_in_byte(const std::string& _dname) const{

      int rvalue = 0;

      if(!has_dataset(_dname))
	return rvalue;
      
      H5::DataSet* ds = 0;
      try {
	ds = new H5::DataSet(file_->openDataSet( _dname ));
      }
      catch(H5::Exception & error){
	return rvalue;
      }

      rvalue = ds->getDataType().getSize();
      delete ds;
      
      return rvalue;
    }


    bool is_integer(const std::string& _dname) const {

      bool rvalue = false;
      if(!has_dataset(_dname))
	return rvalue;
 
      H5::DataSet* ds = 0;
      try {
	ds = new H5::DataSet(file_->openDataSet( _dname ));
      }
      catch(H5::Exception & error){
	return rvalue;
      }

      H5T_class_t type_class = ds->getTypeClass();
      rvalue = (type_class == H5T_INTEGER);
      
      
    
      delete ds;
      
      return rvalue;
    }

    bool is_signed(const std::string& _dname) const {

      bool rvalue = false;
      if(!has_dataset(_dname))
	return rvalue;
 
      H5::DataSet* ds = 0;
      try {
	ds = new H5::DataSet(file_->openDataSet( _dname ));
      }
      catch(H5::Exception & error){
	return rvalue;
      }

      H5T_class_t type_class = ds->getTypeClass();
      if(type_class == H5T_INTEGER){
	const H5T_sign_t fsign = ds->getIntType().getSign();
	rvalue = (fsign != 0);
      }
      
      delete ds;
      
      return rvalue;
    }

    bool is_float(const std::string& _dname) const {
      
      bool rvalue = !is_integer(_dname);

      return rvalue;
    }
    
    template <typename T>
    void shape(std::vector<T>& _shape, const std::string& _dname = "" ) const {

      _shape.clear();
      
      if(!has_dataset(_dname))
	return;
 
      H5::DataSet* ds = 0;
      try {
	ds = new H5::DataSet(file_->openDataSet( _dname ));
      }
      catch(H5::Exception & error){
	return;
      }

      
      _shape.resize(ds->getSpace().getSimpleExtentNdims());

      std::vector<hsize_t> retrieved_dims(_shape.size());
      ds->getSpace().getSimpleExtentDims( (hsize_t*)&retrieved_dims[0], NULL);
      std::copy(retrieved_dims.begin(), retrieved_dims.end(), _shape.begin());

      delete ds;
    }

    template <typename T, typename U>
    int read_nd_dataset(const std::string& _dname, std::vector<T>& _payload, std::vector<U>& _shape){
      if(!_payload.empty())
	_payload.clear();

      H5::DataSet dataset;
      if(ready())
	dataset = load_h5_dataset( _dname );

      if(!dataset.getId())
	return 1;
      
      shape(_shape,_dname);

      _payload.resize(std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<U>()));
      
      return read_nd_dataset(_dname, &_payload[0], _shape);
    }
    
    template <typename T, typename U>
    int read_nd_dataset(const std::string& _dname, T* _payload, std::vector<U>& _shape){

      int rvalue = 1;

      if(!_shape.empty())
	_shape.clear();

      H5::DataSet dataset;
      
      if(ready())
	dataset = load_h5_dataset( _dname );

      if(!dataset.getId())
	return 1;
      
      H5::DSetCreatPropList	plist(dataset.getCreatePlist ());
      H5::DataSpace dataspace;
      try{
	dataspace = H5::DataSpace(dataset.getSpace());
      }
      catch(H5::DataSpaceIException & local_error)
	{
	  error_ = local_error;
	  return 1;
	}
      
      unsigned rank = dataspace.getSimpleExtentNdims();
      std::vector<hsize_t> retrieved_dims(rank);
      dataspace.getSimpleExtentDims( (hsize_t*)&retrieved_dims[0], NULL);

      //check for type match
      if(!h5_read_type_matches<T>(dataset)){
	rvalue = 1;
	return rvalue;
      }
      
	int		numfilt = plist.getNfilters();
      H5Z_filter_t 	filter_type;
      char         	filter_name[1];
      size_t		filter_name_size = {1};
      size_t		cd_values_size = {0};
      unsigned	flags=0;
      unsigned	filter_info = 0;
      unsigned	cd_values[1];
	
      for(int i = 0;i < numfilt;i++){
	cd_values_size = 0;
	filter_info = 0;
	flags = 0;
	  
	filter_type = plist.getFilter(i,
				      flags,
				      cd_values_size,
				      cd_values,
				      filter_name_size,
				      filter_name,
				      filter_info
				      );

	if(filter_type == H5Z_FILTER_SQY)
	  break;
      }

      
      try{
	dataset.read(&_payload[0], hdf5_compiletime_dtype<T>::instance() );
      }
      catch(H5::Exception & local_error)
	{
	  error_ = local_error;
	  return 1;
	}
      
      _shape.resize(rank);
      std::copy(retrieved_dims.begin(), retrieved_dims.end(), _shape.begin());

      if(!_shape.empty())
	rvalue = 0;
      
      return rvalue;
    }

    template <typename T, typename U>
    int write_nd_dataset(const std::string& _dname, const std::vector<T>& _payload, const std::vector<U>& _shape){
      return write_nd_dataset(_dname, &_payload[0], &_shape[0], _shape.size());
    }


      template <typename U>
    int write_compressed_buffer(const std::string& _dname,
		     const char* _payload,
		     const U& _payload_size){
	
      int rvalue = 1;


      sqeazy::image_header hdr(_payload, _payload + _payload_size);
      std::vector<hsize_t> dims;      

      if(!hdr.empty()){
	dims.resize(hdr.shape()->size());
	std::copy(hdr.shape()->begin(), hdr.shape()->end(),dims.begin());
      }
      else {
	return rvalue;
      }
      
      std::vector<hsize_t> chunk_shape(dims);
      

      H5::DataSpace dataspace_(dims.size(), &dims[0]);
      H5::DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);

      std::vector<unsigned> cd_values(std::ceil(float(hdr.size())/(sizeof(unsigned))),0);
      
      if(!hdr.empty()){
	std::copy(hdr.begin(), hdr.end(),(char*)&cd_values[0]);

	plist.setFilter(H5Z_FILTER_SQY,
			H5Z_FLAG_MANDATORY,
			cd_values.size(),
			&cd_values[0]);
      }
      
      H5::PredType type_to_store = hdf5_runtime_dtype::instance(hdr.raw_type());
      
      H5::DataSet dataset_(file_->createDataSet( _dname, 
						 type_to_store,
						 dataspace_, 
						 plist) );
      unsigned long raw_size_byte = std::accumulate(dims.begin(),dims.end(),type_to_store.getSize(),std::multiplies<hsize_t>() );
      std::vector<char> temp(raw_size_byte);
      std::copy(_payload,_payload + _payload_size,temp.begin());
      dataset_.write(&temp[0],
		     type_to_store
		     );
      
      file_->flush(H5F_SCOPE_LOCAL);//this call performs i/o
      rvalue = 0;
      return rvalue;
      
    }

    /**
       \brief write nd dataset uncompressed to file
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    template <typename T, typename U>
    int write_nd_dataset(const std::string& _dname,
    			 const T* _payload,
    			 U* _shape,
    			 const unsigned& _shape_size){
	
      int rvalue = 1;
      
      std::vector<hsize_t> dims(_shape, _shape + _shape_size);
      std::vector<hsize_t> chunk_shape(dims);
      
      H5::DataSpace dataspace_(dims.size(), &dims[0]);
      H5::DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);

      std::vector<unsigned> cd_values;
      
      
      H5::DataSet dataset_(file_->createDataSet( _dname, 
						 hdf5_compiletime_dtype<T>::instance(),
						 dataspace_, 
						 plist) );

      dataset_.write(_payload,
		     hdf5_compiletime_dtype<T>::instance()
		     );
      
      file_->flush(H5F_SCOPE_LOCAL);//this call performs i/o
      rvalue = 0;
      return rvalue;
      
    }

    template <typename T, typename U, typename pipe_type>
    int write_nd_dataset(const std::string& _dname,
			 const std::vector<T>& _payload,
			 const std::vector<U>& _shape,
			 pipe_type
			 ){

      return write_nd_dataset(_dname, &_payload[0], &_shape[0], _shape.size(), pipe_type());
    }

    /**
   \brief write given data set through sqy pipeline given by pipeline type
   
   \param[in] 
   
   \return 
   \retval 
   
*/
    template <typename T, typename U, typename pipe_type>
    int write_nd_dataset(const std::string& _dname,
			 const T* _payload,
			 U* _shape,
			 const unsigned _shape_size,
			 pipe_type
			 ){

      static const std::string filter_name = pipe_type::name();
      if(filter_name.empty())
	return write_nd_dataset(_dname, _payload, _shape, _shape_size);
	
      int rvalue = 1;
      std::vector<hsize_t> dims(_shape, _shape + _shape_size);
      std::vector<hsize_t> chunk_shape(dims);
      
      H5::DataSpace dataspace_(dims.size(), &dims[0]);
      H5::DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);

      
      sqeazy::image_header hdr(T(),dims,filter_name);
      
      std::vector<unsigned> cd_values(std::ceil(float(hdr.size())/(sizeof(unsigned)/sizeof(char))),0);
      
      if(!filter_name.empty()){
	std::copy(hdr.begin(), hdr.end(),(char*)&cd_values[0]);

	plist.setFilter(H5Z_FILTER_SQY,
			H5Z_FLAG_MANDATORY,
			cd_values.size(),
			&cd_values[0]);
      }
      
      
      H5::DataSet dataset_(file_->createDataSet( _dname, 
						 hdf5_compiletime_dtype<T>::instance(),
						 dataspace_, 
						 plist) );

      dataset_.write(_payload,
		     hdf5_compiletime_dtype<T>::instance()
		     );

      
      file_->flush(H5F_SCOPE_LOCAL);//this call performs i/o
      rvalue = 0;
      return rvalue;
      
    }

/**
   \brief write given data set through sqy pipeline given by _filter_name string
   
   \param[in] 
   
   \return 
   \retval 
   
*/
    template <typename T, typename U>
    int write_nd_dataset(const std::string& _dname,
			 const std::string& _filter_name,
			 const T* _payload,
			 U* _shape,
			 const unsigned _shape_size
			 ){


      if(_filter_name.empty())
	return write_nd_dataset(_dname, _payload, _shape, _shape_size);
	
      int rvalue = 1;
      std::vector<hsize_t> dims(_shape, _shape + _shape_size);
      std::vector<hsize_t> chunk_shape(dims);
      

      H5::DataSpace dsp(dims.size(), &dims[0]);
      H5::DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);

      sqeazy::image_header hdr(T(),dims, _filter_name);
      std::string hdr_str = hdr.str();
      size_t cd_values_size = std::ceil(float(hdr_str.size())/(sizeof(int)/sizeof(char)));
      std::vector<unsigned> cd_values(cd_values_size,0);

      if(!_filter_name.empty()){
	std::copy(hdr_str.begin(), hdr_str.end(),(char*)&cd_values[0]);
	// H5Zregister(H5Z_SQY);
	plist.setFilter(H5Z_FILTER_SQY,
			H5Z_FLAG_MANDATORY,
			cd_values.size(),
			&cd_values[0]);
      }
      
      
      H5::DataSet ds(file_->createDataSet( _dname, 
					   hdf5_compiletime_dtype<T>::instance(),
					   dsp, 
					   plist) );

      ds.write(_payload,
	       hdf5_compiletime_dtype<T>::instance()
	       );

      
      file_->flush(H5F_SCOPE_LOCAL);//this call performs i/o
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
								    hdf5_compiletime_dtype<data_type>::instance(),
								    dataspace, 
								    plist) );

	// Write data to dataset.
	dataset->write(&_payload[0],
		       hdf5_compiletime_dtype<data_type>::instance()
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

    static const sqeazy::uint16_passthrough_pipe default_pipe;
    
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
      H5::DSetCreatPropList	plist(ds.getCreatePlist ());
      
      //check if type found is correct
      if(h5_read_type_matches<data_type>(ds)){
	value = 0;
      }

      // H5Zregister(H5Z_SQY);
      
      int		numfilt = plist.getNfilters();
      H5Z_filter_t 	filter_type;
      char         	filter_name[1];
      size_t		filter_name_size = {1};
      size_t		cd_values_size = {0};
      unsigned	flags=0;
      unsigned	filter_info = 0;
      unsigned	cd_values[1];
	
      for(int i = 0;i < numfilt;i++){
	cd_values_size = 0;
	filter_info = 0;
	flags = 0;
	  
	filter_type = plist.getFilter(i,
				      flags,
				      cd_values_size,
				      cd_values,
				      filter_name_size,
				      filter_name,
				      filter_info
				      );

	if(filter_type == H5Z_FILTER_SQY)
	  break;
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
      ds.read( &_payload[0], hdf5_compiletime_dtype<data_type>::instance() , memspace, dsp );
      
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
