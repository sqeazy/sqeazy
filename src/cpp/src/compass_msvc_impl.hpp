#ifndef _COMPASS_MSVC_IMPL_H_
#define _COMPASS_MSVC_IMPL_H_

#include "cpuid.h"
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
  
  namespace runtime {

    namespace detail {
      
      static bool works(compiletime::gnu_tag) {

	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(0,&regs[0],&regs[1],&regs[2],&regs[3]);
      
	if(cpuid_rvalue > 0)
	  return true;
	else
	  return false;
      
      }
    
      static bool works(compiletime::llvm_tag) {

	return false;//not implemented yet
      
      }

      static bool works(compiletime::msvc_tag) {

	return false;//not implemented yet
      
      }


      static std::string vendor(compiletime::gnu_tag) {

	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(0,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );

	std::string vendor_name;
	vendor_name.resize(3*4);
	
	std::copy(reinterpret_cast<char*>(&regs[1]),reinterpret_cast<char*>(&regs[1])+4,
		  vendor_name.begin());
	std::copy(reinterpret_cast<char*>(&regs[3]),reinterpret_cast<char*>(&regs[3])+4,
		  vendor_name.begin()+4);
	std::copy(reinterpret_cast<char*>(&regs[2]),reinterpret_cast<char*>(&regs[2])+4,
		  vendor_name.begin()+8);
	
	if(cpuid_rvalue > 0)
	  return vendor_name;
	else
	  return "";
      
      }
    
      static std::string vendor(compiletime::llvm_tag) {

	return "";//not implemented yet
      
      }

      static std::string vendor(compiletime::msvc_tag) {

	return "";//not implemented yet
      
      }

      
      static bool has(feature::sse , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(1,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t bit25 = (1<<25);
	bool value = (regs[3] & bit25) > 0;
	return value;
      }

      static bool has(feature::sse , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse , ct::msvc_tag){
	return false;
      }

      static bool has(feature::sse2 , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(1,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t bit26 = (1<<26);
	bool value = (regs[3] & bit26) > 0;
	return value;
      }

      static bool has(feature::sse2 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse2 , ct::msvc_tag){
	return false;
      }

      static bool has(feature::sse3 , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(1,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t check_bits = (1<<9) | 1 | (1<<3);
	bool value = (regs[2] & check_bits) > 0;
	return value;
      }

      static bool has(feature::sse3 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse3 , ct::msvc_tag){
	return false;
      }

      static bool has(feature::sse4 , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(1,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t check_bits = (1<<19) | (1<<20);
	bool value = (regs[2] & check_bits) > 0;

	//TODO: check sse4a if AMD is detected
	
	return value;
      }

      static bool has(feature::sse4 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse4 , ct::msvc_tag){
	return false;
      }

      static bool has(feature::avx , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(1,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t check_bits = (1<<28);
	bool value = (regs[2] & check_bits) > 0;


	return value;
      }

      static bool has(feature::avx , ct::llvm_tag){
	return false;
      }

      static bool has(feature::avx , ct::msvc_tag){
	return false;
      }

      
      static bool has(feature::avx2 , ct::gnu_tag){
	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(7,
				       &regs[0],//eax
				       &regs[1],//ebx
				       &regs[2],//ecx
				       &regs[3] //edx
				       );
	if(cpuid_rvalue < 1){
	  std::cerr << "unsupported cpuid level detected\n";
	}
	
	std::uint32_t check_bits = (1<<5);
	bool value = (regs[1] & check_bits) > 0;
	return value;
      }

      static bool has(feature::avx2 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::avx2 , ct::msvc_tag){
	return false;
      }

      
    };
  };
  
  namespace rt = runtime;
  
};
#endif /* _COMPASS_MSVC_IMPL_H_ */
