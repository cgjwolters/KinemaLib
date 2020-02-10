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

#include "Exceptions.h"
#include "DxfRead3D.h"
#include "DxfNameTable3D.h"
#include "DxfReader3D.h"
#include "LineTypeTable3D.h"

#include <cmath>
#include <cstring>

using namespace std;

namespace Ino
{

//---------------------------------------------------------------------------

void LineTypeTable3D::readHeader(DxfReader3D& rdr)
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

void LineTypeTable3D::readLineType(DxfReader3D& rdr)
{
  char name[256] = "";
  double ltArr[256];
  int ltSz = 0, ltCap = 0;

  while (rdr.next()) {
    switch (rdr.code) {
      case   2:      // Name
        strcpy(name,rdr.value);
      break;

      case 102: skipGroup(rdr);
      break;

      case  73: ltCap = rdr.toInt();
                if (ltCap > 256) ltCap = 256;
      break;

      case  49: if (ltSz < ltCap) ltArr[ltSz++] = fabs(rdr.toDouble());
      break;

      case   0: {   // Start of next entry
        DxfLineType3D *lt = new DxfLineType3D(name,ltArr,ltSz);
        add(name,lt);

        return;
      }
    }
  }
}

//---------------------------------------------------------------------------

LineTypeTable3D::LineTypeTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl)
: ObjNameTable3D<DxfLineType3D>(20,20), dxfRd(dxf), nameTable(nameTbl)
{
}

//---------------------------------------------------------------------------

void LineTypeTable3D::readLtTable(DxfReader3D& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0
    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable3D::ID_ENDTAB) return;

    if (id != nameTable.ID_LTYPE) throw FileFormatException("Expecting LTTYPE Table");

    readLineType(rdr);
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
