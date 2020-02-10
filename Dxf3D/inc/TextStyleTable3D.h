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

#ifndef TEXTSTYLETABLE3D_INC
#define TEXTSTYLETABLE3D_INC

#include "ObjNameTable3D.h"
#include "DxfRead3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfNameTable3D;

//---------------------------------------------------------------------------

class TextStyleTable3D : ObjNameTable3D<DxfTxtStyle3D>
{
  DxfRead3D& dxfRd;
  DxfNameTable3D& nameTable;

  void readHeader(DxfReader3D& rdr);
  void readTextStyle(DxfReader3D& rdr);

  TextStyleTable3D(const TextStyleTable3D& cp);            // No Copying
  TextStyleTable3D& operator=(const TextStyleTable3D& src); //No Assignment

public:
  TextStyleTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl);

  using ObjNameTable3D<DxfTxtStyle3D>::get;

  DxfTxtStyle3D *getStdStyle() const;

  void readStyleTable(DxfReader3D& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
