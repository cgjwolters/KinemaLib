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

#include "Basics.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

DxfLineType DxfLineType::solid("solid",NULL,0);

//---------------------------------------------------------------------------

DxfLineType::DxfLineType(const char *ltName, double *dashes, unsigned int dashLen)
: dashList(NULL), dashSz(dashLen), name(dupStr(ltName))
{
  if (!dashes) dashSz = 0;
  else if (dashSz) {
    dashList = new double[dashSz];
    memmove(dashList,dashes,sizeof(double)*dashLen);
  }
}

//---------------------------------------------------------------------------

DxfLineType::DxfLineType(const DxfLineType& cp)
: dashList(NULL), dashSz(cp.dashSz), name(dupStr(cp.name))
{
  if (dashSz) {
    dashList = new double[dashSz];
    memmove(dashList,cp.dashList,sizeof(double)*dashSz);
  }
}

//---------------------------------------------------------------------------

DxfLineType::~DxfLineType()
{
  if (name) delete[] name;
  if (dashList) delete[] dashList;
}

//---------------------------------------------------------------------------

double DxfLineType::operator[](unsigned int idx) const
{
  if (idx >= dashSz) return 0.0;
  else return dashList[idx];
}

} // namespace Ino

//---------------------------------------------------------------------------
