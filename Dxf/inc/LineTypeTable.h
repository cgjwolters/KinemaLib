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

#ifndef DXFLINETYPETABLE_INC
#define DXFLINETYPETABLE_INC

#include "ObjNameTable.h"
#include "DxfRead.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfRead;
class DxfReader;
class DxfLineType;
class DxfNameTable;

class LineTypeTable : ObjNameTable<DxfLineType>
{
  DxfRead& dxfRd;
  DxfNameTable& nameTable;

  LineTypeTable(const LineTypeTable& cp);             // No Copying
  LineTypeTable& operator=(const LineTypeTable& src); // No Assignment

  void readHeader(DxfReader& rdr);
  void readLineType(DxfReader& rdr);

public:
  LineTypeTable(DxfRead& dxf, DxfNameTable& nameTbl);

  using ObjNameTable<DxfLineType>::get;

  void readLtTable(DxfReader& rdr);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

