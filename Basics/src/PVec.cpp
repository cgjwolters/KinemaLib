// --------------------------------------------------------------------------
// ---------------- Persistent Vectors --------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Sept 2008 -------------
// --------------------------------------------------------------------------

#include "PVec.h"

namespace Ino {

// --------------------------------------------------------------------------
// ------- 2D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

PVec2::PVec2(double cx, double cy)
: Persistable(), Vec2(cx,cy)
{
}

// --------------------------------------------------------------------------

PVec2::PVec2(const Vec2& v)
: Persistable(), Vec2(v)
{
}

// --------------------------------------------------------------------------
// Persistence Section

static const char fldX[]   = "X";
static const char fldY[]   = "Y";
static const char fldZ[]   = "Z";
static const char fldDer[] = "IsDer";

// --------------------------------------------------------------------------

void PVec2::definePersistentFields(PersistentWriter& po) const
{
  po.addField(fldX,typeid(double));
  po.addField(fldY,typeid(double));
  po.addField(fldDer,typeid(bool));
}

// --------------------------------------------------------------------------

PVec2::PVec2(PersistentReader& pi)
: Persistable(),
  Vec2(pi.readDouble(fldX,0.0),pi.readDouble(fldY,0.0))
{
  isDerivative = pi.readBool(fldDer,false);
}

// --------------------------------------------------------------------------

void PVec2::writePersistentObject(PersistentWriter& po) const
{
  po.writeDouble(fldX,x);
  po.writeDouble(fldY,y);
  po.writeBool(fldDer,isDerivative);
}

// --------------------------------------------------------------------------
// ------- 3D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

PVec3::PVec3(double cx, double cy, double cz)
: Persistable(), Vec3(cx,cy,cz)
{
}

// --------------------------------------------------------------------------

PVec3::PVec3(const Vec3& v)
: Persistable(), Vec3(v)
{
}

// --------------------------------------------------------------------------

PVec3::PVec3(const Vec2& v)
: Persistable(), Vec3(v)
{
}

// --------------------------------------------------------------------------

PVec3::PVec3(const Vec2& v, double cz)
: Persistable(), Vec3(v,cz)
{
}

// --------------------------------------------------------------------------
// Persistence Section

// --------------------------------------------------------------------------

void PVec3::definePersistentFields(PersistentWriter& po) const
{
  po.addField(fldX,typeid(double));
  po.addField(fldY,typeid(double));
  po.addField(fldZ,typeid(double));
  po.addField(fldDer,typeid(bool));
}

// --------------------------------------------------------------------------

PVec3::PVec3(PersistentReader& pi)
: Persistable(),
  Vec3(pi.readDouble(fldX,0.0),pi.readDouble(fldY,0.0),pi.readDouble(fldZ,0.0))
{
  isDerivative = pi.readBool(fldDer,false);
}

// --------------------------------------------------------------------------

void PVec3::writePersistentObject(PersistentWriter& po) const
{
  po.writeDouble(fldX,x);
  po.writeDouble(fldY,y);
  po.writeDouble(fldZ,z);
  po.writeBool(fldDer,isDerivative);
}

} // namespace Ino
