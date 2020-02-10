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
#include "Basics.h"
#include "Vec.h"

namespace Ino
{

//---------------------------------------------------------------------------

DxfPattern3D::DxfPattern3D()
: angle(0.0), basePt(), offset(), dashes(NULL), dashSz(0)
{
}

//---------------------------------------------------------------------------

DxfPattern3D::~DxfPattern3D()
{
  if (dashes) delete[] dashes;
}

//---------------------------------------------------------------------------

double DxfPattern3D::getDash(unsigned int idx) const
{
  if (idx >= dashSz) return 0.0;

  return dashes[idx];
}

//---------------------------------------------------------------------------

bool DxfPattern3D::read(DxfReader3D& rdr)
{
  for (;;) {
    switch (rdr.code) {
    case 53: angle = rdr.toDouble()/180.0*Vec2::Pi;
      break;

      case 43: basePt.x = rdr.toDouble();
      break;

      case 44: basePt.y = rdr.toDouble();
      break;

      case 45: offset.x = rdr.toDouble();
      break;

      case 46: offset.y = rdr.toDouble();
      break;

      case 79: {
        int dashCnt = rdr.toInt();
        if (dashCnt > 0) dashes = new double[dashCnt];

        if (!rdr.next()) return false;

        for (int i=0; i<dashCnt; i++) {
          if (rdr.code != 49) return false;

          dashes[i] = rdr.toDouble();

          if (!rdr.next()) return false;
        }

        return true;
      }

      default: return false;
    }

    if (!rdr.next()) break;
  }

  return false;
}

} // namespace Ino

//---------------------------------------------------------------------------
