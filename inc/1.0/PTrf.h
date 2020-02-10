// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// ------------ Vector and matrix manipulations 2D/3D ----------------------
// -------------------------------------------------------------------------
// ---------------- Inofor Hoek Automatisering BV---------------------------
// -------------------------------------------------------------------------
// ------------------- Copyright C. Wolters aug 1996 -----------------------
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

#ifndef INOPAWTRF_INC
#define INOPAWTRF_INC

#include "Trf.h"
#include "PersistentIO.h"

namespace Ino
{

// -------------------------------------------------------------------------
// ------- 2D Transformation -----------------------------------------------
// -------------------------------------------------------------------------

class PTrf2 : public Persistable, public Trf2
{
public:
  PTrf2();                                 // Initialise to unitmat
  PTrf2(const Trf2& trf);

  PTrf2(double m00, double m01, double m02,
        double m10, double m11, double m12);
  PTrf2(const Vec2& org, const Vec2& dir); // Initialise to org + dir

  // Persistence:

  PTrf2(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;
};

// -------------------------------------------------------------------------
// ------- 3D Transformation -----------------------------------------------
// -------------------------------------------------------------------------

class PTrf3 : public Persistable, public Trf3
{
public:
  PTrf3();                                    // Initialize to unitmat
  PTrf3(const Trf3& trf);

  PTrf3(double m00, double m01, double m02, double m03,
        double m10, double m11, double m12, double m13,
        double m20, double m21, double m22, double m23);

  PTrf3(const Vec3& org, const Vec3& zDir, const Vec3& xDir);

  // Persistence:

  PTrf3(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;
};

} // namespace Ino

// -------------------------------------------------------------------------
#endif
