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

#ifndef LAYERTABLE3D_INC
#define LAYERTABLE3D_INC

#include "ObjNameTable3D.h"
#include "LineTypeTable3D.h"
#include "DxfNameTable3D.h"
#include "DxfRead3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead3D;
class DxfReader3D;

class LayerTable3D : ObjNameTable3D<DxfLayer3D>
{
  DxfRead3D& dxfRd;
  DxfNameTable3D&  nameTable;
  LineTypeTable3D& ltTable;

  LayerTable3D(const LayerTable3D& cp);            // No Copying
  LayerTable3D& operator=(const LayerTable3D& src); // No Assignment

  void readHeader(DxfReader3D& rdr);
  void readLayer(DxfReader3D& rdr);

public:
  LayerTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl, LineTypeTable3D& ltTbl);

  using ObjNameTable3D<DxfLayer3D>::get;

  void readLayerTable(DxfReader3D& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
