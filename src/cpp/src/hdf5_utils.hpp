#ifndef _HDF5_UTILS_H_
#define _HDF5_UTILS_H_

#include "H5Cpp.h"


namespace sqeazy {
  
  template <typename T>
  struct hdf5_dtype{

    
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


}


#endif /* _HDF5_UTILS_H_ */
