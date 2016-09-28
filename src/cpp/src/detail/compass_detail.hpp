#ifndef _COMPASS_DETAIL_H_
#define _COMPASS_DETAIL_H_

#include <string>
#include "compass_tags.hpp"


#ifdef __clang__
#include "ct/compass_llvm_impl.hpp"
#else
#ifdef __GNUC__
#include "ct/compass_gnu_impl.hpp"
#endif
#endif
      
#ifdef _MSC_BUILD
#include "ct/compass_msvc_impl.hpp"
#endif

#include "rt/compass_x86_impl.hpp"

#endif /* _COMPASS_DETAIL_H_ */
