//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Registry functions ------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright 2002 AntWorks Engineering Software BV -------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters March 2002 ---------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include <StdAfx.h>
#include "Registry.h"

namespace AW
{

//---------------------------------------------------------------------------
/** \addtogroup mswindows MS Windows Specific
 @{
*/

static wchar_t appKey[256] = L"";

//---------------------------------------------------------------------------
/** \file Registry.h
    This file contains a set of convenience functions that allow Windows
    registry values to be set and retrieved in an easy manner.

    \attention Make sure to call function regInit() once before
    a call to any of the other functions.

    \bug Function regInit assume an MFC environment, these functions are
    therefore not usable under Qt (use class \c QSettings instead).

    \author C. Wolters
    \date Mar 2003
*/

//---------------------------------------------------------------------------
/** Initializes the registry key path under which items will be read and
    written.

    Call this function before any other in this modules.\n
    \n
    The path is compiled as:\n
    <tt>"Software\" + app->m_pszRegistryKey + "\" + app->m_pszAppName</tt>.
*/

void regInit()
{
  CWinApp *app = AfxGetApp();

  wcscpy(appKey,L"Software\\");
  wcscat(appKey,app->m_pszRegistryKey);
  wcscat(appKey,L"\\");
  wcscat(appKey,app->m_pszAppName);
}

//---------------------------------------------------------------------------
/** Retrieves the currently set (relative) registry key under which
    all values are stored and retrieved.
    
    \return The applications registry key.
    \see regInit()
*/

const wchar_t *regGetAppKey()
{
  return appKey;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static bool getString(HKEY hive, const wchar_t *name, wchar_t *val, size_t valSz)
{
  if (!val) return false;

  HKEY key;
  DWORD st = RegOpenKeyEx(hive,appKey,0,KEY_QUERY_VALUE,&key);

  if (st != ERROR_SUCCESS) return false;

  DWORD sz = valSz*sizeof(wchar_t);
  st = RegQueryValueEx(key,name,NULL,NULL,(LPBYTE)val,&sz);

  RegCloseKey(key);

  return st == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static bool setString(HKEY hive, const wchar_t *name, const wchar_t *val)
{
  if (!val) return false;

  HKEY key;
  DWORD st = RegCreateKeyEx(hive,appKey,0,NULL,0,
                                               KEY_WRITE,NULL,&key,NULL);

  if (st != ERROR_SUCCESS) return false;

  st = RegSetValueEx(key,name,0,REG_SZ,(CONST BYTE *)val,
                                           (wcslen(val)+1)*sizeof(wchar_t));

  RegCloseKey(key);

  return st == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static bool setMultiString(HKEY hive, const wchar_t *name,
                           const wchar_t *val, int valLen)
{
  if (!val) return false;

  HKEY key;
  DWORD st = RegCreateKeyEx(hive,appKey,0,NULL,0,
                                               KEY_WRITE,NULL,&key,NULL);

  if (st != ERROR_SUCCESS) return false;

  st = RegSetValueEx(key,name,0,REG_MULTI_SZ,(CONST BYTE *)val,valLen*sizeof(wchar_t));

  RegCloseKey(key);

  return st == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static bool getLong(HKEY hive, const wchar_t *name, long& val)
{
  HKEY key;
  DWORD st = RegOpenKeyEx(hive,appKey,0,KEY_QUERY_VALUE,&key);

  if (st != ERROR_SUCCESS) return false;

  DWORD sz = sizeof(val);
  st = RegQueryValueEx(key,name,NULL,NULL,(LPBYTE)&val,&sz);

  RegCloseKey(key);

  return st == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static bool setLong(HKEY hive, const wchar_t *name, long val)
{
  HKEY key;
  DWORD st = RegCreateKeyEx(hive,appKey,0,NULL,0,
                                               KEY_WRITE,NULL,&key,NULL);

  if (st != ERROR_SUCCESS) return false;

  st = RegSetValueEx(key,name,0,REG_DWORD,(CONST BYTE *)&val,sizeof(val));

  RegCloseKey(key);

  return st == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
/** Gets a machine string.
    Gets a string from the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_SZ).
    \param val The value is returned in this parameter, must not be \c NULL.
    \param valSz The size of parameter \a val, this size will be observed
    when writing the value to parameter \a val.
    \return \c true if the string exists and could be fit into parameter
    \a val (observing its size), \c false otherwise.
    \throws NullPointerException if <tt>val == NULL</tt>.
*/

extern bool regMchGetString(const wchar_t *name, wchar_t *val, size_t valSz)
{
  return getString(HKEY_LOCAL_MACHINE,name,val,valSz);
}

//---------------------------------------------------------------------------
/** Writes a machine string.

    Writes a string in the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_SZ).
    \param val The value to write, must not be \c NULL.
    \return \c true if the value could be written, \c false otherwise.
    \throws NullPointerException if <tt>val == NULL</tt>.
*/

extern bool regMchSetString(const wchar_t *name, const wchar_t *val)
{
  return setString(HKEY_LOCAL_MACHINE,name,val);
}

//---------------------------------------------------------------------------
/** Gets a machine multi string.
    Gets a string array from the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string array (type \c REG_MULTI_SZ).
    \param valLst The values are returned in this parameter.
    \param valLstCap The size of parameter \a val, i.e. the number of
    \c CStrings it can hold, this size will be observed when writing the
    values to parameter \a val.
    \param valLstLen Contains the actual number of strings written to
    \a valLst on return (if successful).
    \return \c true if the strings exist and could be fit into parameter
    \a val (observing its size), \c false otherwise.
    \throws NullPointerException if <tt>valLst == NULL</tt>.
    \bug The size check on parameter \a vallLst is not entirely water proof.
*/

extern bool regMchGetMultiString(const wchar_t *name, CString valLst[],
                                        size_t valLstCap, size_t& valLstLen)
{
  if (!valLst) return false;

  valLstLen = 0;

  wchar_t buf[8192] = L"";

  if (!getString(HKEY_LOCAL_MACHINE,name,buf,8191)) return false;

  wchar_t *p = buf;

  for (size_t i=0; i<valLstCap; i++) {
    valLst[i] = p;
    int len = valLst[i].GetLength();
    if (len < 1) return true;
    p += len+1;
    valLstLen++;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Writes a machine multi string.
    Writes a multi string in the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_MULTI_SZ).
    \param valLst The list of strings to write, must not be \c NULL.
    \param valLstLen The number of \c CStrings to write.
    \return \c true if the values could be written, \c false otherwise.
    \throws NullPointerException if <tt>valLst == NULL</tt>.
*/

extern bool regMchSetMultiString(const wchar_t *name,
                                        CString valLst[], size_t valLstLen)
{
  if (!valLst) return false;

  wchar_t buf[8192] = L"";
  wchar_t *p = buf;

  for (size_t i=0; i < valLstLen; i++) {
    wcscpy(p,valLst[i]);
    p += valLst[i].GetLength()+1;
  }

  *p++ = '\0';

  return setMultiString(HKEY_LOCAL_MACHINE,name,buf,p-buf);
}

//---------------------------------------------------------------------------
/** Gets a 32-bit signed machine integer.

    Gets a long from the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the long value (type \c REG_DWORD).
    \param val The value is returned in this parameter.
    \return \c true if the value exists, \c false otherwise.
*/

extern bool regMchGetLong(const wchar_t *name, long& val)
{
  return getLong(HKEY_LOCAL_MACHINE,name,val);
}

//---------------------------------------------------------------------------
/** Gets a 32-bit unsigned machine integer.
    Gets an unsigned long from the \c HKEY_LOCAL_MACHINE hive and then under
    the \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value is returned in this parameter.
    \return \c true if the value exists, \c false otherwise.
*/

extern bool regMchGetULong(const wchar_t *name, unsigned long& val)
{
  long lVal = 0;
  if (!getLong(HKEY_LOCAL_MACHINE,name,lVal)) return false;

  val = (unsigned long)lVal;

  return true;
}

//---------------------------------------------------------------------------
/** Writes a 32-bit signed machine integer.
    Writes a long value in the \c HKEY_LOCAL_MACHINE hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value to write.
    \return \c true if the value could be written, \c false otherwise.
*/

extern bool regMchSetLong(const wchar_t *name, long val)
{
  return setLong(HKEY_LOCAL_MACHINE,name,val);
}

//---------------------------------------------------------------------------
/** Writes a 32-bit unsigned machine integer.
    Writes an unsigned long value in the \c HKEY_LOCAL_MACHINE hive and
    then under the  \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value to write.
    \return \c true if the value could be written, \c false otherwise.
*/

extern bool regMchSetULong(const wchar_t *name, unsigned long val)
{
  long ival = (long)val;
  return setLong(HKEY_LOCAL_MACHINE,name,ival);
}

//---------------------------------------------------------------------------
/** Gets a user string.
    Gets a string from the \c HKEY_CURRENT_USER hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_SZ).
    \param val The value is returned in this parameter, must not be \c NULL.
    \param valSz The size of parameter \a val, this size will be observed
    when writing the value to parameter \a val.
    \return \c true if the string exists and could be fit into parameter
    \a val (observing its size), \c false otherwise.
    \throws NullPointerException if <tt>val == NULL</tt>.
*/

extern bool regUsrGetString(const wchar_t *name, wchar_t *val, size_t valSz)
{
  return getString(HKEY_CURRENT_USER,name,val,valSz);
}

//---------------------------------------------------------------------------
/** Writes a user string.
    Writes a string in the \c HKEY_CURRENT_USER hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_SZ).
    \param val The value to write, must not be \c NULL.
    \return \c true if the value could be written, \c false otherwise.
    \throws NullPointerException if <tt>val == NULL</tt>.
*/

extern bool regUsrSetString(const wchar_t *name, const wchar_t *val)
{
  return setString(HKEY_CURRENT_USER,name,val);
}

//---------------------------------------------------------------------------
/** Gets a user multi string.
    Gets a string array from the \c HKEY_CURRENT_USER hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string array (type \c REG_MULTI_SZ).
    \param valLst The values are returned in this parameter.
    \param valLstCap The size of parameter \a val, i.e. the number of
    \c CStrings it can hold, this size will be observed when writing the
    values to parameter \a val.
    \param valLstLen Contains the actual number of strings written to
    \a valLst on return (if successful).
    \return \c true if the strings exist and could be fit into parameter
    \a val (observing its size), \c false otherwise.
    \throws NullPointerException if <tt>valLst == NULL</tt>.
    \bug The size check on parameter \a vallLst is not entirely water proof.
*/

extern bool regUsrGetMultiString(const wchar_t *name, CString valLst[],
                                        size_t valLstCap, size_t& valLstLen)
{
  if (!valLst) return false;

  valLstLen = 0;

  wchar_t buf[8192] = L"";

  if (!getString(HKEY_CURRENT_USER,name,buf,8191)) return false;

  wchar_t *p = buf;

  for (size_t i=0; i<valLstCap; i++) {
    valLst[i] = p;
    int len = valLst[i].GetLength();
    if (len < 1) return true;
    p += len+1;
    valLstLen++;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Writes a user multi string.
    Writes a multi string in the \c HKEY_CURRENT hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the string value (type \c REG_MULTI_SZ).
    \param valLst The list of strings to write, must not be \c NULL.
    \param valLstLen The number of \c CStrings to write.
    \return \c true if the values could be written, \c false otherwise.
    \throws NullPointerException if <tt>valLst == NULL</tt>.
*/

extern bool regUsrSetMultiString(const wchar_t *name,
                                        CString valLst[], size_t valLstLen)
{
  if (!valLst) return false;

  wchar_t buf[8192] = L"";
  wchar_t *p = buf;

  for (size_t i=0; i < valLstLen; i++) {
    wcscpy(p,valLst[i]);
    p += valLst[i].GetLength()+1;
  }

  *p++ = L'\0';

  return setMultiString(HKEY_CURRENT_USER,name,buf,p-buf);
}

//---------------------------------------------------------------------------
/** Gets a 32-bit signed user integer.
    Gets a long from the \c HKEY_CURRENT_USER hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the long value (type \c REG_DWORD).
    \param val The value is returned in this parameter.
    \return \c true if the value exists, \c false otherwise.
*/

extern bool regUsrGetLong(const wchar_t *name, long& val)
{
  return getLong(HKEY_CURRENT_USER,name,val);
}

//---------------------------------------------------------------------------
/** Writes a 32-bit signed user integer.
    Writes a long value in the \c HKEY_CURRENT_USER hive and then under the 
    \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value to write.
    \return \c true if the value could be written, \c false otherwise.
*/

extern bool regUsrSetLong(const wchar_t *name, long val)
{
  return setLong(HKEY_CURRENT_USER,name,val);
}

//---------------------------------------------------------------------------
/** Gets a 32-bit unsigned user integer.
    Gets an unsigned long from the \c HKEY_CURRENT_USER hive and then under
    the \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value is returned in this parameter.
    \return \c true if the value exists, \c false otherwise.
*/

extern bool regUsrGetULong(const wchar_t *name, unsigned long& val)
{
  long lVal = 0;
  if (!getLong(HKEY_CURRENT_USER,name,lVal)) return false;

  val = (unsigned long)lVal;

  return true;
}

//---------------------------------------------------------------------------
/** Writes a 32-bit unsigned user integer.
    Writes an unsigned long value in the \c HKEY_CURRENT_USER hive and
    then under the  \link regGetAppKey() application key \endlink.
    \param name The name of the value (type \c REG_DWORD).
    \param val The value to write.
    \return \c true if the value could be written, \c false otherwise.
*/

extern bool regUsrSetULong(const wchar_t *name, unsigned long val)
{
  long ival = (long)val;
  return setLong(HKEY_CURRENT_USER,name,ival);
}

//---------------------------------------------------------------------------
/**
 @}
*/

} // namespace AW

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
