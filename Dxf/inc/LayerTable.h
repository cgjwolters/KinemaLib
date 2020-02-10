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

#ifndef LAYERTABLE_INC
#define LAYERTABLE_INC

#include "ObjNameTable.h"
#include "LineTypeTable.h"
#include "DxfNameTable.h"
#include "DxfRead.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead;
class DxfReader;

class LayerTable : ObjNameTable<DxfLayer>
{
  DxfRead& dxfRd;
  DxfNameTable&  nameTable;
  LineTypeTable& ltTable;

  LayerTable(const LayerTable& cp);            // No Copying
  LayerTable& operator=(const LayerTable& src); // No Assignment

  void readHeader(DxfReader& rdr);
  void readLayer(DxfReader& rdr);

public:
  LayerTable(DxfRead& dxf, DxfNameTable& nameTbl, LineTypeTable& ltTbl);

  using ObjNameTable<DxfLayer>::get;

  void readLayerTable(DxfReader& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
