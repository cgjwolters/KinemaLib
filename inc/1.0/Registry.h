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

#ifndef REGISTRY_INC
#define REGISTRY_INC

#include <wchar.h>

namespace AW
{

//---------------------------------------------------------------------------

extern void regInit();

//---------------------------------------------------------------------------

extern const wchar_t *regGetAppKey();

//---------------------------------------------------------------------------

extern bool regMchGetString(const wchar_t *name, wchar_t *val, size_t valSz);

//---------------------------------------------------------------------------

extern bool regMchSetString(const wchar_t *name, const wchar_t *val);

//---------------------------------------------------------------------------

extern bool regMchGetMultiString(const wchar_t *name, CString valLst[],
                                        size_t valLstCap, size_t& valLstLen);

//---------------------------------------------------------------------------

extern bool regMchSetMultiString(const wchar_t *name,
                                        CString valLst[], size_t valLstLen);

//---------------------------------------------------------------------------

extern bool regMchGetLong(const wchar_t *name, long& val);

//---------------------------------------------------------------------------

extern bool regMchGetULong(const wchar_t *name, unsigned long& val);

//---------------------------------------------------------------------------

extern bool regMchSetLong(const wchar_t *name, long val);

//---------------------------------------------------------------------------

extern bool regMchSetULong(const wchar_t *name, unsigned long val);

//---------------------------------------------------------------------------

extern bool regUsrGetString(const wchar_t *name, wchar_t *val, size_t valSz);

//---------------------------------------------------------------------------

extern bool regUsrSetString(const wchar_t *name, const wchar_t *val);

//---------------------------------------------------------------------------

extern bool regUsrGetMultiString(const wchar_t *name, CString valLst[],
                                        size_t valLstCap, size_t& valLstLen);

//---------------------------------------------------------------------------

extern bool regUsrSetMultiString(const wchar_t *name,
                                        CString valLst[], size_t valLstLen);

//---------------------------------------------------------------------------

extern bool regUsrGetLong(const wchar_t *name, long& val);

//---------------------------------------------------------------------------

extern bool regUsrGetULong(const wchar_t *name, unsigned long& val);

//---------------------------------------------------------------------------

extern bool regUsrSetLong(const wchar_t *name, long val);

//---------------------------------------------------------------------------

extern bool regUsrSetULong(const wchar_t *name, unsigned long val);

} // namespace AW

//---------------------------------------------------------------------------
#endif