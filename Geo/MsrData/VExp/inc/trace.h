#ifndef H_TRACE
#define H_TRACE

#ifndef NDEBUG
extern void cdecl trace(char * pszFormat, ...);
void cdecl print(char * pszFormat, ...);
#endif

#endif
#ifdef WIN32
#pragma comment(compiler)
#endif

