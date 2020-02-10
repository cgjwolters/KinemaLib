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

#include "DxfRead.h"
#include "DxfReader.h"
#include "DxfNameTable.h"
#include "TextStyleTable.h"

#include <cmath>
#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

static DxfTxtStyle defStyle(1.0,0.0);

static const char stdName[] = "STANDARD";

//---------------------------------------------------------------------------

void TextStyleTable::readHeader(DxfReader& rdr)
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

void TextStyleTable::readTextStyle(DxfReader& rdr)
{
  char name[256] = "";
  double widFact = 1.0;
  double slant   = 0.0;

  while (rdr.next()) {
    switch (rdr.code) {
      case   2:      // Name
        strcpy(name,rdr.value);
      break;

      case 102: skipGroup(rdr);
      break;

      case  41: widFact = fabs(rdr.toDouble());
      break;

      case  50: {
        double oblAng = rdr.toDouble();
        if (oblAng < -45) oblAng = -45;
        else if (oblAng > 45) oblAng = 45;

        slant = oblAng/45.0;
      }
      break;

      case   0: {   // Start of next entry
        DxfTxtStyle *st = new DxfTxtStyle(widFact,slant);

        add(name,st);

        return;
      }
    }
  }
}

//---------------------------------------------------------------------------

TextStyleTable::TextStyleTable(DxfRead& dxf, DxfNameTable& nameTbl)
: ObjNameTable<DxfTxtStyle>(20,20), dxfRd(dxf), nameTable(nameTbl)
{
}

//---------------------------------------------------------------------------

DxfTxtStyle *TextStyleTable::getStdStyle() const
{
  DxfTxtStyle *st = (DxfTxtStyle *)get(stdName);

  if (!st) return &defStyle;
  else return st;
}

//---------------------------------------------------------------------------

void TextStyleTable::readStyleTable(DxfReader& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0
    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable::ID_ENDTAB) return;

    if (id != DxfNameTable::ID_STYLE) return; // Something wrong

    readTextStyle(rdr);
  }
}


//---------------------------------------------------------------------------

DxfTxtStyle::DxfTxtStyle(double wid, double sl)
: widFac(wid), slant(sl)
{
}

} // namespace Ino

//---------------------------------------------------------------------------
