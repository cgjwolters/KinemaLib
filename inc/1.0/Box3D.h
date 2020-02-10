// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------- 3D axis aligned box --------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Prodim International BV 2008 --------------
// --------------------------------------------------------------------------
// -------------------- C. Wolters Sept 2008 --------------------------------
// --------------------------------------------------------------------------

#ifndef INOBOX3D_INC
#define INOBOX3D_INC

#include "Vec.h"

namespace Ino
{

// --------------------------------------------------------------------------

class Box3D
{
   Vec3 vll, vur;
   bool valid;

   void order();

  public:
   Box3D();
   Box3D(const Vec3& ll, const Vec3& ur) : vll(ll), vur(ur) { order(); };
   Box3D(double lx, double ly, double lz, double hx, double hy, double hhz);

   Box3D& operator=(const Box3D& src);
   Box3D& operator+= (const Box3D& r);     // Join a rect 
   Box3D& operator+= (const Vec3& p);

   bool isValid() const { return valid; }
   void clear();

   bool intersectsXY(const Box3D& r, double extra_edge) const;
   bool intersects(const Box3D& r, double extra_edge) const;

   void rectUpdate(const Box3D& r);
   void rectUpdate(const Vec3& p1, const Vec3& p2);
   void aroundArc(const Vec3& s, const Vec3& e, const Vec2& c, bool acw);

   bool pointInsideXY(const Vec2& p, double extra_edge) const;
   bool pointInside(const Vec3& p, double extra_edge) const;

   const Vec3& ll() const { return vll; };
   const Vec3& ur() const { return vur; };

   Vec3 ul() const;
   Vec3 lr() const;

   double height() const;
   double width()  const;
   
   bool midPoint(Vec3& midPt) const;

   double distToXY(const Vec2& p) const; // if p inside --> return 0.0

   double areaXY() const;
};

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
