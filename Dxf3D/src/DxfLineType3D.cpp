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

#include "Basics.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

DxfLineType3D DxfLineType3D::solid("solid",NULL,0);

//---------------------------------------------------------------------------

DxfLineType3D::DxfLineType3D(const char *ltName, double *dashes, unsigned int dashLen)
: dashList(NULL), dashSz(dashLen), name(dupStr(ltName))
{
  if (!dashes) dashSz = 0;
  else if (dashSz) {
    dashList = new double[dashSz];
    memmove(dashList,dashes,sizeof(double)*dashLen);
  }
}

//---------------------------------------------------------------------------

DxfLineType3D::DxfLineType3D(const DxfLineType3D& cp)
: dashList(NULL), dashSz(cp.dashSz), name(dupStr(cp.name))
{
  if (dashSz) {
    dashList = new double[dashSz];
    memmove(dashList,cp.dashList,sizeof(double)*dashSz);
  }
}

//---------------------------------------------------------------------------

DxfLineType3D::~DxfLineType3D()
{
  if (name) delete[] name;
  if (dashList) delete[] dashList;
}

//---------------------------------------------------------------------------

double DxfLineType3D::operator[](unsigned int idx) const
{
  if (idx >= dashSz) return 0.0;
  else return dashList[idx];
}

} // namespace Ino

//---------------------------------------------------------------------------
