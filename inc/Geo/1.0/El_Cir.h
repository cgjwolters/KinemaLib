/* ---------------------------------------------------------------------- */
/* ---------------- 3D Circle Element ----------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

// $Id: El_Cir.h,v 1.2 2009/03/02 21:26:58 clemens Exp $

#ifndef EL_CIR_INC
#define EL_CIR_INC

#include "Elem.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

const int Elem_Type_Circle = 2;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

class Elem_Circle : public Elem
{
  private:
   Vec3 lp1;
   mutable Vec3 lp2;  // Only z-coordinate used
   Vec2 cntre;
   bool lccw;

   void update();

  public:
   void * operator new(size_t);
   void operator delete(void *);

   static void CleanupStore();

   void *operator new(size_t, Persistable *p) { return p; }

#ifndef __BORLANDC__
   void operator delete(void * /*p*/, Persistable*) {}
#endif

   Elem_Circle();

   Elem_Circle(const Vec3& p1, double z2, const Vec2& c, bool ccw);

   Elem_Circle(const Vec3& p1, const Vec3& p2, const Vec2& c, bool ccw);

   Elem_Circle(const Elem_Circle& cp);
   Elem_Circle(const Elem_Circle& cp, bool keep_info);

   Elem_Circle& operator=(const Elem_Circle& src);

   virtual Elem *Clone(bool keep_info = true) const
                            { return new Elem_Circle(*this, keep_info); }

   virtual int Type() const { return Elem_Type_Circle; }

   virtual bool isLine() const   { return false; }
   virtual bool isArc() const    { return false; }
   virtual bool isCircle() const { return true; }

   virtual const Vec3&  P1()  const { return lp1; }
   virtual const Vec3&  P2()  const;
   const Vec2&  C()   const { return cntre; }
   double R()   const { return cntre.distTo2(lp1); }
   bool   Ccw() const { return lccw; }

   virtual void Z1(double new_z) { lp1.z = new_z; }
   virtual void Z2(double new_z) { lp2.z = new_z; }

   virtual bool At_Par(double par, Vec3& p) const;

   virtual Vec2 Start_Tangent_XY () const;
   virtual Vec2 End_Tangent_XY   () const;
   virtual Vec2& Start_Tangent_XY(Vec2& tg) const; // For efficiency freaks
   virtual Vec2& End_Tangent_XY  (Vec2& tg) const; // For efficiency freaks
   virtual bool Tangent_At_XY    (double par, Vec2& tg) const;

   virtual Vec3 Start_Tangent () const;
   virtual Vec3 End_Tangent   () const;
   virtual Vec3& Start_Tangent(Vec3& tg) const; // For efficiency freaks
   virtual Vec3& End_Tangent  (Vec3& tg) const; // For efficiency freaks
   virtual bool Tangent_At    (double par, Vec3& tg) const;

   virtual double Start_Curve() const;
   virtual double End_Curve  () const;
   virtual bool   Curve_At   (double par, double& curve) const;

   virtual double Area_XY_P1() const;
   virtual double Moment_X() const;
   virtual double Moment_Y() const;

   virtual double Span_Angle() const;

   virtual void Reverse() { lccw = !lccw; }

   virtual bool Project_Pnt_XY(const Vec2& p, double tol, bool strict,
                               Vec3& pp,
                               double& parm, double &dist_xy) const;

   virtual bool Project_Pnt_Strict_XY(const Vec2& p,
                                      double tol,
                                      Vec3& pp,
                                      double& parm,
                                      double &dist_xy) const;
                                      
   virtual void Intersect_XY(const Elem& el, Isect_Lst& isect_lst) const;
   virtual void Intersect_XY(const Elem& el, bool tang_ok,
                                             Isect_Lst& isect_lst) const;
   virtual void Intersect_XY_Ext(const Elem& el, bool tang_ok,
                                             Isect_Lst& isect_lst) const;

   virtual bool Offset_Into(double dist, Elem_List& offel,
                                           bool keep_info = false) const;

   virtual bool Offset_Into(double dist, double minlen, Elem_List& off_lst,
                                             Vec3& strtpt, Vec3& endpt,
                                                   double& mislen) const;

   virtual bool Split(double par, Elem_List& spl_lst) const;

   virtual void Stretch_Begin_XY(const Vec2& isp, bool check=true);
   virtual void Stretch_End_XY(const Vec2& isp, bool check=true);

   bool Limit_Span_180(Elem_Cursor& newpair) const;

   virtual void Transform(const Trf2& trf);

   virtual bool Fillet(const Elem& el2, double filletRad,
                                             Elem_Arc& fillet) const;

   // Persistable Section:

   Elem_Circle(PersistentReader& pi);
   virtual void definePersistentFields(PersistentWriter& po) const;
   virtual void writePersistentObject(PersistentWriter& po) const;
};

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
