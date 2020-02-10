// --------------------------------------------------------------------------
// ---------------- Persistent Vectors --------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Sept 2008 -------------
// --------------------------------------------------------------------------

#ifndef INOPAWVEC_INC
#define INOPAWVEC_INC

#include "Vec.h"
#include "PersistentIO.h"

namespace Ino 
{

// --------------------------------------------------------------------------
// ------- 2D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

class PVec2 : public Persistable, public Vec2
{
public:
  PVec2(double cx=0, double cy=0.0);
  PVec2(const Vec2& v);

  // Persistence:

  PVec2(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;
};

// --------------------------------------------------------------------------
// ------- 3D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

class PVec3 : public Persistable, public Vec3
{
  public:
   PVec3(double cx=0.0, double cy=0.0, double cz=0.0);
   PVec3(const Vec3& v);
   PVec3(const Vec2& v);
   PVec3(const Vec2& v, double cz);

  // Persistence:

  PVec3(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;
};

} // namespace Ino

// --------------------------------------------------------------------------
#endif
