//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf 3D Format Reader --------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2008 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Sept 2008----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef OBJNAMETABLE3D_INC
#define OBJNAMETABLE3D_INC

#include "Basics.h"
#include "ObjNameTableBase3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfReader3D;

template <class T> class ObjNameTable3D : protected ObjNameTableBase3D
{
  ObjNameTable3D(const ObjNameTable3D& cp);             // No Copying
  ObjNameTable3D& operator=(const ObjNameTable3D& src); // No Assignment

protected:
  virtual ObjEntryBase3D *newObjEntry(const char *val, int hashCode);

  using ObjNameTableBase3D::skipGroup;

public:
  ObjNameTable3D() : ObjNameTableBase3D() {}
  ObjNameTable3D(unsigned int initCap,unsigned int capExt) : ObjNameTableBase3D(initCap,capExt) {}
  virtual ~ObjNameTable3D() {}

  using ObjNameTableBase3D::size;
  using ObjNameTableBase3D::clear;

  T *get(const char *val) const;

  T *add(const char *val, T *v);
};

} // namespace Ino

#include "ObjNameTableImp3D.h"

//---------------------------------------------------------------------------
#endif
