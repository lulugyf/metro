#ifndef _HELLO_H
  #define _HELLO_H

#if defined( __cplusplus ) || defined( c_plusplus )
extern "C" {
#endif


#ifdef BUILD_DLL
 #define DLL_API __declspec( dllexport )
#else
 #define DLL_API __declspec( dllimport )
#endif

DLL_API void MyDllSay( void );

#if defined( __cplusplus ) || defined( c_plusplus )
}
#endif

#endif
