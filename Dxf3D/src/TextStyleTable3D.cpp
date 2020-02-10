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

#include "DxfRead3D.h"
#include "DxfReader3D.h"
#include "DxfNameTable3D.h"
#include "TextStyleTable3D.h"

#include <cmath>
#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

static DxfTxtStyle3D defStyle(1.0,0.0);

static const char stdName[] = "STANDARD";

//---------------------------------------------------------------------------

void TextStyleTable3D::readHeader(DxfReader3D& rdr)
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

void TextStyleTable3D::readTextStyle(DxfReader3D& rdr)
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
        DxfTxtStyle3D *st = new DxfTxtStyle3D(widFact,slant);

        add(name,st);

        return;
      }
    }
  }
}

//---------------------------------------------------------------------------

TextStyleTable3D::TextStyleTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl)
: ObjNameTable3D<DxfTxtStyle3D>(20,20), dxfRd(dxf), nameTable(nameTbl)
{
}

//---------------------------------------------------------------------------

DxfTxtStyle3D *TextStyleTable3D::getStdStyle() const
{
  DxfTxtStyle3D *st = (DxfTxtStyle3D *)get(stdName);

  if (!st) return &defStyle;
  else return st;
}

//---------------------------------------------------------------------------

void TextStyleTable3D::readStyleTable(DxfReader3D& rdr)
{
  readHeader(rdr);
  if (rdr.eof) return;

  for (;;) {
    // Here rdr.code == 0
    int id = nameTable.getNameId(rdr.value);

    if (id == DxfNameTable3D::ID_ENDTAB) return;

    if (id != DxfNameTable3D::ID_STYLE) return; // Something wrong

    readTextStyle(rdr);
  }
}


//---------------------------------------------------------------------------

DxfTxtStyle3D::DxfTxtStyle3D(double wid, double sl)
: widFac(wid), slant(sl)
{
}

} // namespace Ino

//---------------------------------------------------------------------------
