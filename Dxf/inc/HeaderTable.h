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

#ifndef HEADERTABLE_INC
#define HEADERTABLE_INC

#include "IntNameTable.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead;
class DxfNameTable;
class DxfReader;

class HeaderTable
{
  DxfRead& dxfRd;
  DxfNameTable& nameTable;

  IntNameTable intNames;
  IntNameTable dblNames;
  IntNameTable vecNames;
  IntNameTable strNames;

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

  HeaderTable(const HeaderTable& cp);             // No Copying
  HeaderTable& operator=(const HeaderTable& src); // No Assignment

  void defaultTables();

public:
  HeaderTable(DxfRead& dxf, DxfNameTable& nameTbl);
  ~HeaderTable();

  void readTable(DxfReader& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
