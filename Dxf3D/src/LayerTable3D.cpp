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

#include "LayerTable3D.h"

#include "Exceptions.h"
#include "DxfRead3D.h"
#include "DxfReader3D.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------

void LayerTable3D::readHeader(DxfReader3D& rdr)
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

void LayerTable3D::readLayer(DxfReader3D& rdr)
{
  char layerName[256] = "";
  int colNr = 256;
  short flags = 0;

  DxfLineType3D *lt = &DxfLineType3D::solid;

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

      case 70:
        flags = (short)rdr.toInt();
      break;

      case 370:      // Line Weight
      break;

      case 0:  // Start of next entry
        if (strlen(layerName) < 1) return; // Something wrong

        if (!lt) lt = &DxfLineType3D::solid;
        DxfLayer3D *lay = new DxfLayer3D(layerName,colNr,*lt,flags);

        add(layerName,lay);
        dxfRd.builder.addLayer(*lay);

      return;
    }
  }
}

//---------------------------------------------------------------------------

LayerTable3D::LayerTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl, LineTypeTable3D& ltTbl)
: ObjNameTable3D<DxfLayer3D>(200,200), dxfRd(dxf), nameTable(nameTbl), ltTable(ltTbl)
{
}

//---------------------------------------------------------------------------

void LayerTable3D::readLayerTable(DxfReader3D& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0

    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable3D::ID_ENDTAB) return;

    if (id != DxfNameTable3D::ID_LAYER) throw FileFormatException("Expecting LAYER Table");

    readLayer(rdr);
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
