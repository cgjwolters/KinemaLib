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

#ifndef OBJNAMETABLEBASE_INC
#define OBJNAMETABLEBASE_INC

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------

class ObjEntryBase;
class DxfReader;

class ObjNameTableBase
{
  ObjEntryBase **list; // HashMap
  unsigned int cap,sz;

  ObjNameTableBase(const ObjNameTableBase& cp);             // No Copying
  ObjNameTableBase& operator=(const ObjNameTableBase& src); // No Assignment

  void rehash(unsigned int newCap);

protected:
  const unsigned int extCap;

  virtual ObjEntryBase *newObjEntry(const char *val, int hashCode) = 0;

  void skipGroup(DxfReader& rdr);

public:
  ObjNameTableBase();
  ObjNameTableBase(unsigned int initCap, unsigned int capExt);
  virtual ~ObjNameTableBase();

  int size() const { return sz; }

  void clear();

  ObjEntryBase *getEnt(const char *val) const;

  ObjEntryBase *addEnt(const char *val);
};

//---------------------------------------------------------------------------

class ObjEntryBase
{
  ObjEntryBase(const ObjEntryBase& cp);             // No Copying
  ObjEntryBase& operator=(const ObjEntryBase& src); // No Assignment

public:
  ObjEntryBase(const char *chValue, int hshCode);
  virtual ~ObjEntryBase();

  const int hashCode;
  const char *const value;

  ObjEntryBase *next;

  int compare(const char *chVal) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
