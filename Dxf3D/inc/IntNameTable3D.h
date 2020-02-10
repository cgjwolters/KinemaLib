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

#ifndef INTNAMETABLE3D_INC
#define INTNAMETABLE3D_INC

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------

class IntEntry3D;

class IntNameTable3D
{
  IntEntry3D **list;
  unsigned int sz,cap,extCap;

  IntNameTable3D(const IntNameTable3D& cp);             // No Copying
  IntNameTable3D& operator=(const IntNameTable3D& src); // No Assignment

  void rehash(unsigned int newCap);

public:
  IntNameTable3D();
  IntNameTable3D(int initCap, int capExt);

  ~IntNameTable3D();

  unsigned int size() const { return sz; }

  int getNameId(const char *value) const;

  int addName(const char *value, int id);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
