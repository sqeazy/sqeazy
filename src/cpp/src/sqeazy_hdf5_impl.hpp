#ifndef _SQEAZY_HDF5_IMPL_H_
#define _SQEAZY_HDF5_IMPL_H_

#include "H5Cpp.h"

//#include "sqeazy.h"
#include "sqeazy_h5_filter.hpp"
#include "sqeazy_header.hpp"
#include "sqeazy_predef_pipelines.hpp"


namespace sqeazy {

  struct loaded_hdf5_plugin {

    loaded_hdf5_plugin(){
      if(!H5Zfilter_avail(H5Z_FILTER_SQY))
	H5Zregister(H5Z_SQY);

    }

    ~loaded_hdf5_plugin(){
      // if(H5Zfilter_avail(H5Z_FILTER_BZIP2))
      // 	H5Zunregister(H5Z_FILTER_SQY);
    }
    
  };

}


#endif /* _SQEAZY_HDF5_IMPL_H_ */
