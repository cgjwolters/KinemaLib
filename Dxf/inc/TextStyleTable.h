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

#ifndef TEXTSTYLETABLE_INC
#define TEXTSTYLETABLE_INC

#include "ObjNameTable.h"
#include "DxfRead.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfNameTable;

//---------------------------------------------------------------------------

class TextStyleTable : ObjNameTable<DxfTxtStyle>
{
  DxfRead& dxfRd;
  DxfNameTable& nameTable;

  void readHeader(DxfReader& rdr);
  void readTextStyle(DxfReader& rdr);

  TextStyleTable(const TextStyleTable& cp);            // No Copying
  TextStyleTable& operator=(const TextStyleTable& src); //No Assignment

public:
  TextStyleTable(DxfRead& dxf, DxfNameTable& nameTbl);

  using ObjNameTable<DxfTxtStyle>::get;

  DxfTxtStyle *getStdStyle() const;

  void readStyleTable(DxfReader& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
