#ifndef _COMPASS_TAGS_H_
#define _COMPASS_TAGS_H_

#include <string>

namespace compass {

  namespace feature {

    struct sse {};
    struct sse2 {};
    struct sse3 {};
    struct sse4 {};

    struct avx {};
    struct avx2 {};

  };

  
  namespace compiletime {

    struct gnu_tag  {};
    struct llvm_tag {};
    struct msvc_tag {};

    struct platform {

#ifdef __GNUC__
      typedef gnu_tag type;
#endif

#ifdef __clang__
      typedef llvm_tag type;
#endif
      
#ifdef __MSC_BUILD__
      typedef msvc_tag type;
#endif
      
    };
  };

  namespace ct = compiletime;
  
};
#endif /* _COMPASS_TAGS_H_ */
