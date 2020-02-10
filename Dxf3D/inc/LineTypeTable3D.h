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

#ifndef DXFLINETYPETABLE3D_INC
#define DXFLINETYPETABLE3D_INC

#include "ObjNameTable3D.h"
#include "DxfRead3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead3D;
class DxfReader3D;
class DxfLineType3D;
class DxfNameTable3D;

class LineTypeTable3D : ObjNameTable3D<DxfLineType3D>
{
  DxfRead3D& dxfRd;
  DxfNameTable3D& nameTable;

  LineTypeTable3D(const LineTypeTable3D& cp);             // No Copying
  LineTypeTable3D& operator=(const LineTypeTable3D& src); // No Assignment

  void readHeader(DxfReader3D& rdr);
  void readLineType(DxfReader3D& rdr);

public:
  LineTypeTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl);

  using ObjNameTable3D<DxfLineType3D>::get;

  void readLtTable(DxfReader3D& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

