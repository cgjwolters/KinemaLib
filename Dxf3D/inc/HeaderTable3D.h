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

#ifndef HEADERTABLE3D_INC
#define HEADERTABLE3D_INC

#include "IntNameTable3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead3D;
class DxfNameTable3D;
class DxfReader3D;

class HeaderTable3D
{
  DxfRead3D& dxfRd;
  DxfNameTable3D& nameTable;

  IntNameTable3D intNames;
  IntNameTable3D dblNames;
  IntNameTable3D vecNames;
  IntNameTable3D strNames;

  const int ID_UNITS;
  const int ID_UCSXDIR;
  const int ID_UCSYDIR;
  const int ID_ACADVER;

  int    *intTbl;
  double *dblTbl;
  double *vecXTbl;
  double *vecYTbl;
  double *vecZTbl;
  char  **strTbl;

  HeaderTable3D(const HeaderTable3D& cp);             // No Copying
  HeaderTable3D& operator=(const HeaderTable3D& src); // No Assignment

  void defaultTables();

public:
  HeaderTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl);
  ~HeaderTable3D();

  void readTable(DxfReader3D& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
