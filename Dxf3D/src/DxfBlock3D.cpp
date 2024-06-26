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

DxfBlock3D::DxfBlock3D()
: user1(NULL), user2(NULL), userIndex(-1), name(NULL), layer(NULL),
  base(0), root(true)
{
}

//---------------------------------------------------------------------------

DxfBlock3D::DxfBlock3D(const char *blkName, DxfLayer3D *lay, const Vec3& basePt)
: user1(NULL), user2(NULL), userIndex(-1), name(dupStr(blkName)), layer(lay),
  base(basePt), root(false)
{
}

//---------------------------------------------------------------------------

DxfBlock3D::~DxfBlock3D()
{
  if (name) delete[] name;
}

} // namespace Ino

//---------------------------------------------------------------------------

