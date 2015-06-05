#ifndef _HDF5_TEST_UTILS_H
#define _HDF5_TEST_UTILS_H

#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

#include "sqeazy_hdf5_impl.hpp"

int stored_sizeof_in_h5_file(const std::string& _fname, const std::string& _dname){
  int value = -1;
    
  try{
    H5File file(_fname, H5F_ACC_RDONLY);
    DataSet dataset(file.openDataSet( _dname ));
    H5T_class_t type_class = dataset.getTypeClass();
    if( type_class == H5T_INTEGER )
      {
	IntType rtype = dataset.getIntType();
	value = rtype.getSize();
      }
    
    file.close();
  }
  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {

      std::cout << __LINE__ << ": caught FileIException ("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      std::cout <<  __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException error)
    {
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }
  
  return value;
}

void dataset_shape_in_h5_file(const std::string& _fname, const std::string& _dname, std::vector<int>& _shape){
  std::vector<hsize_t> temp;
    
  try{
    H5File file(_fname, H5F_ACC_RDONLY);
    DataSet dataset(file.openDataSet( _dname ));
    DataSpace dataspace(dataset.getSpace());
    temp.resize(dataspace.getSimpleExtentNdims());

    dataspace.getSimpleExtentDims(&temp[0]);
    _shape.resize(temp.size());
    std::copy(temp.begin(), temp.end(),_shape.begin());
      
    file.close();
  }
  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {

      std::cout << __LINE__ << ": caught FileIException ("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      std::cout <<  __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException error)
    {
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return;
    }
  
  return;
}


bool dataset_in_h5_file(const std::string& _fname, const std::string& _dname = ""){
  bool value = false;

  try{
    H5File file(_fname, H5F_ACC_RDONLY);
    DataSet dataset(file.openDataSet( _dname ));

    value = dataset.getInMemDataSize() > 0;
    
    file.close();
  }
  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {

      std::cout << __LINE__ << ": caught FileIException ("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      std::cout <<  __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException error)
    {
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }
  
  return value;
}
  
bool sqy_used_in_h5_file(const std::string& _fname, const std::string& _dname = ""){
  bool value = false;

  try{
    H5File file(_fname, H5F_ACC_RDONLY);
    DataSet dataset(file.openDataSet( _dname ));

    unsigned        numfilt;
    size_t     nelmts={1}, namelen={1};
    unsigned  flags, filter_info, cd_values[1];
    char       name[1];
    H5Z_filter_t filter_type;
	
    // Get the create property list of the dataset.
    DSetCreatPropList plist(dataset.getCreatePlist ());
	
    // Get the number of filters associated with the dataset.
    numfilt = plist.getNfilters();

    for (unsigned idx=0; idx < numfilt; idx++) {
      nelmts = 0;

      filter_type = plist.getFilter(idx, flags, nelmts, cd_values, namelen, name , filter_info);

      if(filter_type == H5Z_FILTER_SQY){
	value = true;
	break;
      }
    }
    
   
    file.close();
  }
  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {

      std::cout << __LINE__ << ": caught FileIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      std::cout <<  __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException error)
    {
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return value;
    }
  
  return value;
}

template <typename U>
int h5_compress_ushort_dataset(
			    const H5std_string& _fname,
			    const H5std_string& _dname,
			    const std::vector<unsigned short> _data,
			    const std::vector<U> _shape,
			    const bool& _use_filter = false)
{
  std::vector<hsize_t> dims(_shape.begin(), _shape.end());
  std::vector<hsize_t> chunk_shape(dims);
  //int     i,j;
  //  const unsigned long n_elements = std::accumulate(dims.begin(), dims.end(),1,std::multiplies<int>());
  

  // Try block to detect exceptions raised by any of the calls inside it
  try
    {
      // Turn off the auto-printing when failure occurs so that we can
      // handle the errors appropriately

      // Create a new file using the default property lists. 
      H5File file(_fname, H5F_ACC_TRUNC);

      // Create the data space for the dataset.
      DataSpace dataspace(_shape.size(), &dims[0]);

      // // Modify dataset creation property to enable chunking
      DSetCreatPropList  plist;
      plist.setChunk(chunk_shape.size(), &chunk_shape[0]);
      
      std::vector<unsigned> cd_values(1,42);

      if(_use_filter)
	plist.setFilter(H5Z_FILTER_SQY,
			 H5Z_FLAG_MANDATORY,
			 cd_values.size(),
			 &cd_values[0]);

      ////////////////////////////////////////////////////////////////////////////////////
      // from HDF5DynamicallyLoadedFilters.pdf:
      // --------------------------------------
      // dcpl = H5Pcreate (H5P_DATASET_CREATE);
      // status = H5Pset_filter (dcpl,
      //			   H5Z_FILTER_BZIP2, H5Z_FLAG_MANDATORY,
      // 			   (size_t)6, cd_values);
      //			   //config for dynamic bzip2 filter: cd_values[1] = {6};

      // Create the dataset.      
      DataSet*  dataset = new DataSet(file.createDataSet( _dname, 
							  PredType::NATIVE_USHORT,
							  dataspace, 
							  plist) );

      // Write data to dataset.
      dataset->write(&_data[0],
		    PredType::NATIVE_USHORT
		    );

      // Close objects and file.  Either approach will close the HDF5 item.
      //    delete dataspace;
      delete dataset;
      //    delete plist;
      file.close();


    }  // end of try block

  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught FileIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      
      return 1;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
    }

  catch(Exception & error){
    error.printError();
    std::cout << __LINE__ << ": caught Exception("<< _fname << ":" << _dname
	      <<"):\t" << error.getDetailMsg() << "\n";
    return 1;

  }
  return 0;  // successfully terminated
}

int h5_decompress_ushort_dataset(
				 const H5std_string& _fname,
				 const H5std_string& _dname,
				 std::vector<unsigned short>& _content
				 )
{
  std::vector<hsize_t> dims;

  //   int     i,j;
  //   const unsigned long n_elements = std::accumulate(dims.begin(), dims.end(),1,std::multiplies<int>());
  //   std::vector<int> buf(n_elements);

  try{
    // -----------------------------------------------
    // Re-open the file and dataset, retrieve filter 
    // information for dataset and read the data back.
    // -----------------------------------------------

    // Open the file and the dataset in the file.
    H5File			file(_fname, H5F_ACC_RDONLY);
    DataSet			dataset(file.openDataSet( _dname ));
    DSetCreatPropList	plist(dataset.getCreatePlist ());

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

    DataSpace dataspace = dataset.getSpace();
    unsigned rank = dataspace.getSimpleExtentNdims();
    if(rank!=dims.size()){
      dims.resize(rank);
      dataspace.getSimpleExtentDims( &dims[0], NULL);
    }
      
    
    unsigned size_u_short = std::accumulate(dims.begin(), dims.end(),1.,std::multiplies<int>());
    if(_content.size()!=size_u_short)
      _content.resize(size_u_short);

    dataset.read(&_content[0], PredType::NATIVE_USHORT);

    // delete plist; 
    // delete dataset;
    file.close();	// can be skipped
	
  }
 
  // catch failure caused by the H5File operations
  catch(FileIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught FileIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
    }

  // catch failure caused by the DataSet operations
  catch(DataSetIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught DataSetIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
    }

  // catch failure caused by the DataSpace operations
  catch(DataSpaceIException & error)
    {
      error.printError();
      std::cout << __LINE__ << ": caught DataSpaceIException("<< _fname << ":" << _dname
		<<"):\t" << error.getDetailMsg() << "\n";
      return 1;
    }

  catch(Exception & error){
    error.printError();
    std::cout << __LINE__ << ": caught Exception("<< _fname << ":" << _dname
	      <<"):\t" << error.getDetailMsg() << "\n";
    return 1;

  }
  
  return 0;

}

#endif /* _HDF5_TEST_UTILS_H */
