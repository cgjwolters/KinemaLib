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

#include "HeaderTable3D.h"

#include "Basics.h"
#include "DxfRead3D.h"
#include "DxfReader3D.h"
#include "DxfNameTable3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

void HeaderTable3D::defaultTables()
{
  intTbl[ID_UNITS] = 4; // Millimeters

  vecXTbl[ID_UCSXDIR] = 1.0;
  vecYTbl[ID_UCSXDIR] = 0.0;
  vecZTbl[ID_UCSXDIR] = 0.0;

  vecXTbl[ID_UCSYDIR] = 0.0;
  vecYTbl[ID_UCSYDIR] = 1.0;
  vecZTbl[ID_UCSYDIR] = 0.0;

  strTbl[ID_ACADVER] = dupStr("Unknown");
}

//---------------------------------------------------------------------------

HeaderTable3D::HeaderTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl)
: dxfRd(dxf), nameTable(nameTbl),
  intNames(20,20), dblNames(20,20), vecNames(20,20), strNames(20,20),

  ID_UNITS(intNames.addName("$LUNITS",0)),
  ID_UCSXDIR(vecNames.addName("$UCSXDIR",0)),
  ID_UCSYDIR(vecNames.addName("$UCSYDIR",1)),
  ID_ACADVER(strNames.addName("$ACADVER",0)),

  intTbl( new int[intNames.size()]),
  dblTbl( new double[dblNames.size()]),
  vecXTbl(new double[vecNames.size()]),
  vecYTbl(new double[vecNames.size()]),
  vecZTbl(new double[vecNames.size()]),
  strTbl( new char*[strNames.size()])
{
  defaultTables();
}

//---------------------------------------------------------------------------

HeaderTable3D::~HeaderTable3D()
{
  if (intTbl)  delete[] intTbl;
  if (dblTbl)  delete[] dblTbl;
  if (vecXTbl) delete[] vecXTbl;
  if (vecYTbl) delete[] vecYTbl;
  if (vecZTbl) delete[] vecZTbl;

  if (strTbl) {
    int ssz = strNames.size();

    for (int i=0; i<ssz; ++i) {
      if (strTbl[i]) delete[] strTbl[i];
    }

    delete[] strTbl;
  }
}

//---------------------------------------------------------------------------

void HeaderTable3D::readTable(DxfReader3D& rdr)
{
  while (rdr.next()) {
    if (rdr.code == 0) {
      int id = nameTable.getNameId(rdr.value);

      if (id == DxfNameTable3D::ID_ENDSEC) return;
    }

    if (rdr.code != 9) continue;

    int id = intNames.getNameId(rdr.value);
    if (id >= 0) {
      if (!rdr.next()) return;

      intTbl[id] = rdr.toInt();

      continue;
    }

    id = dblNames.getNameId(rdr.value);
    if (id >= 0) {
      if (!rdr.next()) return;

      dblTbl[id] = rdr.toDouble();

      continue;
    }

    id = vecNames.getNameId(rdr.value);
    if (id >= 0) {
      if (!rdr.next()) return;

      switch (rdr.code) {
        case 10: vecXTbl[id] = rdr.toDouble();
        break;

        case 20: vecYTbl[id] = rdr.toDouble();
        break;

        case 30: vecZTbl[id] = rdr.toDouble();
        break;
      }
      continue;
    }

    id = strNames.getNameId(rdr.value);
    if (id >= 0) {
      if (!rdr.next()) return;

      if (strTbl[id]) delete[] strTbl[id];
      strTbl[id] = dupStr(rdr.value);

      continue;
    }
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
