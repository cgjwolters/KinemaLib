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

#ifndef OBJNAMETABLEBASE3D_INC
#define OBJNAMETABLEBASE3D_INC

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------

class ObjEntryBase3D;
class DxfReader3D;

class ObjNameTableBase3D
{
  ObjEntryBase3D **list; // HashMap
  unsigned int cap,sz;

  ObjNameTableBase3D(const ObjNameTableBase3D& cp);             // No Copying
  ObjNameTableBase3D& operator=(const ObjNameTableBase3D& src); // No Assignment

  void rehash(unsigned int newCap);

protected:
  const unsigned int extCap;

  virtual ObjEntryBase3D *newObjEntry(const char *val, int hashCode) = 0;

  void skipGroup(DxfReader3D& rdr);

public:
  ObjNameTableBase3D();
  ObjNameTableBase3D(unsigned int initCap, unsigned int capExt);
  virtual ~ObjNameTableBase3D();

  int size() const { return sz; }

  void clear();

  ObjEntryBase3D *getEnt(const char *val) const;

  ObjEntryBase3D *addEnt(const char *val);
};

//---------------------------------------------------------------------------

class ObjEntryBase3D
{
  ObjEntryBase3D(const ObjEntryBase3D& cp);             // No Copying
  ObjEntryBase3D& operator=(const ObjEntryBase3D& src); // No Assignment

public:
  ObjEntryBase3D(const char *chValue, int hshCode);
  virtual ~ObjEntryBase3D();

  const int hashCode;
  const char *const value;

  ObjEntryBase3D *next;

  int compare(const char *chVal) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
