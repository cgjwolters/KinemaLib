// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// ------------ Vector and matrix manipulations 2D/3D ----------------------
// -------------------------------------------------------------------------
// ---------------- Inofor Hoek Automatisering BV---------------------------
// -------------------------------------------------------------------------
// ------------------- Copyright C. Wolters aug 1996 -----------------------
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

#include "PTrf.h"

namespace Ino
{

// -------------------------------------------------------------------------
// ------- 2D Transformation -----------------------------------------------
// -------------------------------------------------------------------------

PTrf2::PTrf2()
: Persistable(), Trf2()
{
}

// -------------------------------------------------------------------------

PTrf2::PTrf2(const Trf2& trf)
: Persistable(), Trf2(trf)
{
}

// -------------------------------------------------------------------------

PTrf2::PTrf2(double m00, double m01, double m02,
             double m10, double m11, double m12)
: Persistable(), Trf2(m00,m01,m02, m10,m11, m12)
{
}

// -------------------------------------------------------------------------

PTrf2::PTrf2(const Vec2& org, const Vec2& dir)
: Persistable(), Trf2(org,dir)
{
}

// -------------------------------------------------------------------------
// Persistence:

static const char *fld[3][4] = { { "M00", "M01", "M02", "M03" },
                                 { "M10", "M11", "M12", "M13" },
                                 { "M20", "M21", "M22", "M23" } };
static const char fldDer[] = "Der";

// -------------------------------------------------------------------------

void PTrf2::definePersistentFields(PersistentWriter& po) const
{
  for (int i=0; i<2; ++i) {
    for (int j=0; j<3; ++j) po.addField(fld[i][j],typeid(double));
  }

  po.addField(fldDer,typeid(bool));
}

// -------------------------------------------------------------------------

PTrf2::PTrf2(PersistentReader& pi)
: Persistable(),
  Trf2(pi.readDouble(fld[0][0],1.0),pi.readDouble(fld[0][1],0.0),
                                             pi.readDouble(fld[0][2],0.0),
       pi.readDouble(fld[1][0],0.0),pi.readDouble(fld[1][1],1.0),
                                             pi.readDouble(fld[1][2],0.0))
{
  isDerivative = pi.readBool(fldDer,false);
}

// -------------------------------------------------------------------------

void PTrf2::writePersistentObject(PersistentWriter& po) const
{
  for (int i=0; i<2; ++i) {
    for (int j=0; j<3; ++j) po.writeDouble(fld[i][j],Trf2::operator()(i,j));
  }

  po.writeBool(fldDer,isDerivative);
}

// -------------------------------------------------------------------------
// ------- 3D Transformation -----------------------------------------------
// -------------------------------------------------------------------------

PTrf3::PTrf3()
: Persistable(), Trf3()
{
}

// -------------------------------------------------------------------------

PTrf3::PTrf3(const Trf3& trf)
: Persistable(), Trf3(trf)
{
}

// -------------------------------------------------------------------------

PTrf3::PTrf3(double m00, double m01, double m02, double m03,
             double m10, double m11, double m12, double m13,
             double m20, double m21, double m22, double m23)
: Persistable(),
  Trf3(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23)
{
}

// -------------------------------------------------------------------------

PTrf3::PTrf3(const Vec3& org, const Vec3& zDir, const Vec3& xDir)
: Persistable(), Trf3(org,zDir,xDir)
{
}

// -------------------------------------------------------------------------
// Persistence:

// -------------------------------------------------------------------------

void PTrf3::definePersistentFields(PersistentWriter& po) const
{
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) po.addField(fld[i][j],typeid(double));
  }

  po.addField(fldDer,typeid(bool));
}

// -------------------------------------------------------------------------

PTrf3::PTrf3(PersistentReader& pi)
: Persistable(),
  Trf3(pi.readDouble(fld[0][0],1.0),pi.readDouble(fld[0][1],0.0),
                 pi.readDouble(fld[0][2],0.0),pi.readDouble(fld[0][3],0.0),
       pi.readDouble(fld[1][0],0.0),pi.readDouble(fld[1][1],1.0),
                 pi.readDouble(fld[1][2],0.0),pi.readDouble(fld[1][3],0.0),
       pi.readDouble(fld[2][0],0.0),pi.readDouble(fld[2][1],0.0),
                 pi.readDouble(fld[2][2],1.0),pi.readDouble(fld[2][3],0.0))
{
  isDerivative = pi.readBool(fldDer,false);
}

// -------------------------------------------------------------------------

void PTrf3::writePersistentObject(PersistentWriter& po) const
{
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j)
      po.writeDouble(fld[i][j],Trf3::operator()(i,j));
  }

  po.writeBool(fldDer,isDerivative);
}

} // namespace Ino

// -------------------------------------------------------------------------
