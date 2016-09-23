#ifndef _COMPASS_DETAIL_H_
#define _COMPASS_DETAIL_H_

#include <string>
#include "compass_tags.hpp"

#ifdef __GNUC__
#include "compass_gnu_impl.hpp"
#endif

#ifdef __clang__
#include "compass_llvm_impl.hpp"
#endif
      
#ifdef __MSC_BUILD__
#include "compass_msvc_impl.hpp"
#endif

#endif /* _COMPASS_DETAIL_H_ */
