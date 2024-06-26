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

namespace Ino
{

//---------------------------------------------------------------------------

DxfLayer3D::DxfLayer3D(const char *layerName, int colorNr,
                                         DxfLineType3D &ltp, short layFlags)
: name(dupStr(layerName)), colNr(colorNr), lt(ltp),
  lw(0.25), flags(layFlags), userIndex(-1)
{
}

//---------------------------------------------------------------------------

DxfLayer3D::DxfLayer3D(const DxfLayer3D& cp)
: name(dupStr(cp.name)), colNr(cp.colNr), lt(cp.lt),
  lw(cp.lw), flags(cp.flags),
  userIndex(cp.userIndex)
{
}

//---------------------------------------------------------------------------

DxfLayer3D::~DxfLayer3D()
{
  if (name) delete[] (char *)name;
}

} // namespace Ino

//---------------------------------------------------------------------------
