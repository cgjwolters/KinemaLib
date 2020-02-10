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

#include "HeaderTable.h"

#include "Basics.h"
#include "DxfRead.h"
#include "DxfReader.h"
#include "DxfNameTable.h"

namespace Ino
{

//---------------------------------------------------------------------------

void HeaderTable::defaultTables()
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

HeaderTable::HeaderTable(DxfRead& dxf, DxfNameTable& nameTbl)
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

HeaderTable::~HeaderTable()
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

void HeaderTable::readTable(DxfReader& rdr)
{
  while (rdr.next()) {
    if (rdr.code == 0) {
      int id = nameTable.getNameId(rdr.value);

      if (id == DxfNameTable::ID_ENDSEC) return;
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
