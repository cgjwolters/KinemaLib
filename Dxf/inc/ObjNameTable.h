//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf Format Reader -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters June 2005----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef OBJNAMETABLE_INC
#define OBJNAMETABLE_INC

#include "Basics.h"
#include "ObjNameTableBase.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfReader;

template <class T> class ObjNameTable : protected ObjNameTableBase
{
  ObjNameTable(const ObjNameTable& cp);             // No Copying
  ObjNameTable& operator=(const ObjNameTable& src); // No Assignment

protected:
  virtual ObjEntryBase *newObjEntry(const char *val, int hashCode);

  using ObjNameTableBase::skipGroup;

public:
  ObjNameTable() : ObjNameTableBase() {}
  ObjNameTable(unsigned int initCap,unsigned int capExt) : ObjNameTableBase(initCap,capExt) {}
  virtual ~ObjNameTable() {}

  using ObjNameTableBase::size;
  using ObjNameTableBase::clear;

  T *get(const char *val) const;

  T *add(const char *val, T *v);
};

} // namespace Ino

#include "ObjNameTableImp.h"

//---------------------------------------------------------------------------
#endif
