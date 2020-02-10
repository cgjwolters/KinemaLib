/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1994 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#ifndef INORECT_INC
#define INORECT_INC

#include "Vec.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- 3D Axis Aligned Rectangle ------------------------------------ */
/* ---------------------------------------------------------------------- */

class Rect_Ax
{
   Ino::Vec3 vll, vur;
   bool valid;

   void order();

  public:
   Rect_Ax();
   Rect_Ax(const Ino::Vec3& ll, const Ino::Vec3& ur) : vll(ll), vur(ur) { order(); };
   Rect_Ax(double lx, double ly, double lz, double hx, double hy, double hhz);

   Rect_Ax& operator=(const Rect_Ax& src);
   Rect_Ax& operator+= (const Rect_Ax& r);     // Join a rect 
   Rect_Ax& operator+= (const Ino::Vec3& p);

   bool isValid() const { return valid; }
   void clear();

   bool Intersects_XY(const Rect_Ax& r, double extra_edge) const;
   bool Intersects(const Rect_Ax& r, double extra_edge) const;

   void Rect_Update(const Rect_Ax& r);
   void Rect_Update(const Ino::Vec3& p1, const Ino::Vec3& p2);
   void Around_Arc(const Ino::Vec3& s, const Ino::Vec3& e, const Ino::Vec2& c, bool acw);

   bool Point_Inside_XY(const Ino::Vec2& p, double extra_edge) const;
   bool Point_Inside(const Ino::Vec3& p, double extra_edge) const;

   const Ino::Vec3& Ll() const { return vll; };
   const Ino::Vec3& Ur() const { return vur; };

   Ino::Vec3 Ul() const;
   Ino::Vec3 Lr() const;

   double Height() const;
   double Width()  const;
   
   bool MidPoint(Ino::Vec3& midPt) const;

   double Dist_To_XY(const Ino::Vec2& p) const; // if p inside --> return 0.0

   double Area_XY() const;
};

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
