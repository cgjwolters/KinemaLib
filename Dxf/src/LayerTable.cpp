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

#include "LayerTable.h"

#include "Exceptions.h"
#include "DxfRead.h"
#include "DxfReader.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------

void LayerTable::readHeader(DxfReader& rdr)
{
  while (rdr.next()) { // Read header
    switch (rdr.code) {
      case 102: skipGroup(rdr);
      break;

      case 0: return;
    }
  }
}

//---------------------------------------------------------------------------

void LayerTable::readLayer(DxfReader& rdr)
{
  char layerName[256] = "";
  int colNr = 256;
  DxfLineType *lt = &DxfLineType::solid;

  while (rdr.next()) {
    switch (rdr.code) {
      case   2:      // Name
        strcpy(layerName,rdr.value);
      break;

      case   6: {     // LineType Name
        lt = ltTable.get(rdr.value);
      }
      break;

      case 102: skipGroup(rdr);
      break;

      case  62: colNr = abs(rdr.toInt());
      break;

      case 370:      // Line Weight
      break;

      case 0:  // Start of next entry
        if (strlen(layerName) < 1) return; // Something wrong

        if (!lt) lt = &DxfLineType::solid;
        DxfLayer *lay = new DxfLayer(layerName,colNr,*lt);

        add(layerName,lay);
      return;
    }
  }
}

//---------------------------------------------------------------------------

LayerTable::LayerTable(DxfRead& dxf, DxfNameTable& nameTbl, LineTypeTable& ltTbl)
: ObjNameTable<DxfLayer>(200,200), dxfRd(dxf), nameTable(nameTbl), ltTable(ltTbl)
{
}

//---------------------------------------------------------------------------

void LayerTable::readLayerTable(DxfReader& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0

    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable::ID_ENDTAB) return;

    if (id != DxfNameTable::ID_LAYER) throw FileFormatException("Expecting LAYER Table");

    readLayer(rdr);
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
