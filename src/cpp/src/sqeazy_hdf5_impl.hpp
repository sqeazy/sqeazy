#ifndef _SQEAZY_HDF5_IMPL_H_
#define _SQEAZY_HDF5_IMPL_H_

#include <iostream>

#include "H5Cpp.h"

#include "sqeazy_h5_filter.hpp"
#include "sqeazy_header.hpp"


namespace sqeazy {

  struct loaded_hdf5_plugin {
	  
	  //TODO: create atomic bool/int to save that sqy was already registered with hdf5

	  loaded_hdf5_plugin() {

		  int reg_rval = 0;
		  if (!H5Zfilter_avail(H5Z_FILTER_SQY))
			  reg_rval = H5Zregister(H5Z_SQY);

		  if (reg_rval < 0)
			  std::cerr << __FILE__ << ":"<< __LINE__ 
			  << "\t unable to register sqy as hdf5 filter!!\n";
		  else
			  std::cout << "Done registering sqy as hdf5 filter.\n";

	  }

    ~loaded_hdf5_plugin(){
      // if(H5Zfilter_avail(H5Z_FILTER_BZIP2))
      // 	H5Zunregister(H5Z_FILTER_SQY);
    }
    
  };

}


#endif /* _SQEAZY_HDF5_IMPL_H_ */
