#define STRICT
#include <windows.h>
#include <stdarg.h> 
#include <stdio.h> 

#ifndef NDEBUG
void cdecl trace(char * pszFormat, ...);

#define nLocalBuf 512
void cdecl trace(char * pszFormat, ...)
{
	int nBuf ;
	static char szBuffer[nLocalBuf] ;
	va_list args ;
	va_start(args, pszFormat) ;
	nBuf = vsprintf(szBuffer, pszFormat, args) ;    
    printf(szBuffer) ;
	OutputDebugString(szBuffer) ;

#pragma message("DEBUG VERSION: void cdecl trace(char * pszFormat, ...); ")	
}

#define nLocalBuf 512
void cdecl print(char * pszFormat, ...)
{
	int nBuf ;
	static char szBuffer[nLocalBuf] ;
	va_list args ;
	va_start(args, pszFormat) ;
	nBuf = vsprintf(szBuffer, pszFormat, args) ;
	printf(szBuffer) ;
	//OutputDebugString(szBuffer) ;
#pragma message("DEBUG VERSION: void cdecl print(char * pszFormat, ...); ")	
}


#endif 