#ifndef _SQEAZY_H5_FILTER_H_
#define _SQEAZY_H5_FILTER_H_

#include "hdf5.h"
#include "H5PLextern.h"
#include "sqeazy_definitions.hpp"

/* use an integer greater than 256 to be id of the registered filter. */
//zip code of MPI CBG
static const H5Z_filter_t H5Z_FILTER_SQY = 01307;



/* declare a hdf5 filter function */
SQY_FUNCTION_PREFIX size_t H5Z_filter_sqy(unsigned flags,
					  size_t cd_nelmts,
					  const unsigned cd_values[],
					  size_t nbytes,
					  size_t *buf_size,
					  void**buf);

// declare hdf5 plugin info functions
SQY_FUNCTION_PREFIX H5PL_type_t   H5PLget_plugin_type();
SQY_FUNCTION_PREFIX const void*   H5PLget_plugin_info();

static const H5Z_class2_t H5Z_SQY[1] = {{
  H5Z_CLASS_T_VERS, /* H5Z_class_t version */
  H5Z_FILTER_SQY, /* Filter id number */
  1, /* encoder_present flag (set to true) */
  1, /* decoder_present flag (set to true) */
  "HDF5 sqy filter; see https://bitbucket.org/sqeazy/sqeazy",  /* Filter info */
  NULL, /* The "can apply" callback (TODO: was is that?) */
  NULL, /* The "set local" callback (TODO: was is that?) */
  (H5Z_func_t) H5Z_filter_sqy,  /* The filter function */
}};


#endif /* _SQEAZY_H5_FILTER_H_ */
