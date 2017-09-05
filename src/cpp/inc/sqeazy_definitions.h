#ifndef _SQEAZY_DEFINITIONS_H_
#define _SQEAZY_DEFINITIONS_H_

#ifdef _WIN32
#define SQY_FUNCTION_PREFIX extern "C" __declspec(dllexport)
#else
#define SQY_FUNCTION_PREFIX extern "C"
#endif



#endif /* _SQEAZY_DEFINITIONS_H_ */
