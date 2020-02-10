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

#ifndef INTNAMETABLE_INC
#define INTNAMETABLE_INC

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------

class IntEntry;

class IntNameTable
{
  IntEntry **list;
  unsigned int sz,cap,extCap;

  IntNameTable(const IntNameTable& cp);             // No Copying
  IntNameTable& operator=(const IntNameTable& src); // No Assignment

  void rehash(unsigned int newCap);

public:
  IntNameTable();
  IntNameTable(int initCap, int capExt);

  ~IntNameTable();

  unsigned int size() const { return sz; }

  int getNameId(const char *value) const;

  int addName(const char *value, int id);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
