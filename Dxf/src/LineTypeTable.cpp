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

#include "Exceptions.h"
#include "DxfRead.h"
#include "DxfNameTable.h"
#include "DxfReader.h"
#include "LineTypeTable.h"

#include <cmath>
#include <cstring>

using namespace std;

namespace Ino
{

//---------------------------------------------------------------------------

void LineTypeTable::readHeader(DxfReader& rdr)
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

void LineTypeTable::readLineType(DxfReader& rdr)
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
        DxfLineType *lt = new DxfLineType(name,ltArr,ltSz);
        add(name,lt);

        return;
      }
    }
  }
}

//---------------------------------------------------------------------------

LineTypeTable::LineTypeTable(DxfRead& dxf, DxfNameTable& nameTbl)
: ObjNameTable<DxfLineType>(20,20), dxfRd(dxf), nameTable(nameTbl)
{
}

//---------------------------------------------------------------------------

void LineTypeTable::readLtTable(DxfReader& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0
    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable::ID_ENDTAB) return;

    if (id != nameTable.ID_LTYPE) throw FileFormatException("Expecting LTTYPE Table");

    readLineType(rdr);
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
