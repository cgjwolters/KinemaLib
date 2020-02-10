/* ---------------------------------------------------------------------- */
/* ---------------- Abstract Basic 3D Element --------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

// $Id: Elem.h,v 1.2 2009/03/06 10:40:30 clemens Exp $

#ifndef ELEM_INC
#define ELEM_INC

#include "Vec.h"
#include "Rect.h"
#include "Isect.h"

#include "it_dlist.h"

#include "PersistentIO.h"

namespace Ino
{
  class Trf2;
}

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Next function to be implemented by client! ------------------- */
/* ---------------------------------------------------------------------- */

class Elem_Info;

extern Elem_Info *Clone_Elem_Info (const Elem_Info *from);
extern void       Delete_Elem_Info(Elem_Info *info);

class Elem;
class Elem_Arc;

/* ---------------------------------------------------------------------- */
/* ------- Element Reference Class -------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ------- (To construct list of elements etc) -------------------------- */
/* ---------------------------------------------------------------------- */

class Elem_Ref
{
   Elem *elp;

  public:
   Elem_Ref() : elp(NULL) {}
   Elem_Ref(const Elem& el);
   Elem_Ref(const Elem_Ref& cp);
   ~Elem_Ref();

   Elem_Ref& operator=(const Elem_Ref& src);
   Elem_Ref& operator=(const Elem& src);

   const Elem& El() const { return *elp; }
   Elem& El()             { return *elp; }

   void setElem(Elem *el);

   operator bool() const { return elp != NULL; } // Is valid??
};

/* ---------------------------------------------------------------------- */
/* ------- Basic Element List Typedefs ---------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Elem_Ref> > Elem_Alloc;

typedef IT_D_List<Elem_Ref,Elem_Alloc> Elem_List;
typedef Elem_List::Cursor   Elem_Cursor;
typedef Elem_List::C_Cursor Elem_C_Cursor;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Calc_Par_Len(const Elem_List& ellst);

/* ---------------------------------------------------------------------- */
/* ----------- Basic abstract (contour) element ------------------------- */
/* ---------------------------------------------------------------------- */

class Elem : public Persistable, protected Rect_Ax
{
  private:
   Elem_Info *inf;    // Optional additional element info

   Elem& operator=(const Elem& src); // No Assignment

  protected:
   static long defaultColor;

   double plen;       // Parametric length of this element
   double len;        // 3D length of this element
   double len_xy;     // 2D length of this element
   double bpar;       // Begin 3D parametric value

   int el_id;         // id number for use in milling
   int cnt_id;        // id number for use in milling
   int p_cnt_id;      // id number of parent contour
   int cam_inf;       // extra info for cam use

   bool insArc;

   long color;

   void clone_from(const Elem& from, bool keep_info = true);
   Elem(const Elem& cp) : Persistable(cp), Rect_Ax(cp), 
                          inf(NULL) { clone_from(cp, true); }
   Elem(const Elem& cp, bool keep_info) : Persistable(cp), Rect_Ax(cp),
                          inf(NULL)  { clone_from(cp, keep_info); }

  public:
   Elem() : Persistable(), Rect_Ax(), inf(NULL), plen(0.0),
            len(0.0), len_xy(0.0),
            bpar(0.0), el_id(0), cnt_id(0), p_cnt_id(0),
            cam_inf(0), insArc(false),color(defaultColor) {}
   virtual ~Elem();

   virtual Elem *Clone(bool keep_info = true) const = 0;

   virtual int Type() const = 0;

   virtual bool isLine() const   = 0;
   virtual bool isArc() const    = 0;
   virtual bool isCircle() const = 0;

   static long getDefaultColor();
   static void setDefaultColor(long newColor);

   long getColor() const { return color; }
   bool setColor(long newColor);

   const Rect_Ax& Rect() const { return *this; }

   void Add_Info(const Elem_Info& info);
   void Del_Info();

   const Elem_Info *Info() const { return inf; }
   Elem_Info *Info() { return inf; }

   void Id(int id) { el_id = id; }
   int  Id() const { return el_id; }

   void Cnt_Id(int id) { cnt_id = id; }
   int  Cnt_Id() const { return cnt_id; }

   void P_Cnt_Id(int id) { p_cnt_id = id; }
   int  P_Cnt_Id() const { return p_cnt_id; }

   void Cam_Inf(int new_inf) { cam_inf = new_inf; }
   int  Cam_Inf() const      { return cam_inf; }

   void setInsArc(bool newInsArc) { insArc = newInsArc; }
   bool isInsArc() const { return insArc; }

   virtual const Vec3& P1() const = 0;
   virtual const Vec3& P2() const = 0;

   virtual void Z1(double new_z) = 0;
   virtual void Z2(double new_z) = 0;

   bool   Par_Valid(double par) const;

   void   Begin_Par (double par) { bpar = par; }
   void   End_Par   (double par) { plen = par - bpar; }
   
   double Begin_Par () const { return bpar; }
   double End_Par   () const   { return bpar + plen; }

   virtual bool At_Par(double par, Vec3& p) const = 0;

   void Mid_Par_XY(Vec2& p) const;
   void Mid_Par(Vec3& p) const;

   double Mid_Par_XY() const;
   double Mid_Par() const;

   bool Par_XY(double par, double& par_xy) const;
   bool Par   (double par_xy, double& par) const;

   virtual Vec2 Start_Tangent_XY () const = 0;
   virtual Vec2 End_Tangent_XY   () const = 0;
   virtual Vec2& Start_Tangent_XY(Vec2& tg) const = 0; // For efficiency freaks
   virtual Vec2& End_Tangent_XY  (Vec2& tg) const = 0; // For efficiency freaks
   virtual bool Tangent_At_XY    (double par, Vec2& tg) const = 0;

   virtual Vec3 Start_Tangent () const = 0;
   virtual Vec3 End_Tangent   () const = 0;
   virtual Vec3& Start_Tangent(Vec3& tg) const = 0; // For efficiency freaks
   virtual Vec3& End_Tangent  (Vec3& tg) const = 0; // For efficiency freaks
   virtual bool Tangent_At    (double par, Vec3& tg) const = 0;

   virtual double Start_Curve () const = 0;
   virtual double End_Curve   () const = 0;
   virtual bool   Curve_At    (double par, double& curve) const = 0;

   double Len_XY() const { return len_xy; }
   double Len() const { return len; }
   double Par_Len() const { return plen; }

   virtual double Area_XY_P1() const = 0;
   virtual double Moment_X() const = 0;
   virtual double Moment_Y() const = 0;

   virtual double Span_Angle() const = 0;

   virtual void Reverse() = 0;

   virtual bool Project_Pnt_XY(const Vec2& p, double tol, bool strict,
                               Vec3& pp,
                               double& parm, double &dist_xy) const = 0;

   virtual bool Project_Pnt_Strict_XY(const Vec2& p,
                                      double tol,
                                      Vec3& pp,
                                      double& parm,
                                      double &dist_xy) const = 0;
                                      
   virtual void Intersect_XY(const Elem& el, Isect_Lst& isect_lst) const = 0;
   virtual void Intersect_XY(const Elem& el, bool tang_ok,
                                             Isect_Lst& isect_lst) const = 0;
   virtual void Intersect_XY_Ext(const Elem& el, bool tang_ok,
                                             Isect_Lst& isect_lst) const = 0;

   virtual bool Offset_Into(double dist, Elem_List& offel,
                                          bool keep_info = false ) const = 0;
   virtual bool Offset_Into(double dist, double minlen, Elem_List& off_lst,
                                             Vec3& strtpt, Vec3& endpt,
                                                   double& mislen) const = 0;

   virtual bool Split(double par, Elem_List& spl_lst) const = 0;

   virtual void Stretch_Begin_XY(const Vec2& isp, bool check=true) = 0;
   virtual void Stretch_End_XY(const Vec2& isp, bool check=true) = 0;

   void Join_To_XY(Elem& el);
   void Join_To_XY(Elem& el, bool check);

   virtual void Transform(const Trf2& trf) = 0;

   virtual bool Fillet(const Elem& el2, double filletRad,
                                             Elem_Arc& fillet) const = 0;

   static void Sort(Elem_List& elLst);
   static bool IsContiguous(const Elem_List& elLst,
                            bool mustBeClosed = false,
                            double tol = Vec2::IdentDist);

   // Persistable Section:

   Elem(PersistentReader& pi);
   virtual void definePersistentFields(PersistentWriter& po) const;
   virtual void writePersistentObject(PersistentWriter& po) const;
};

/* ---------------------------------------------------------------------- */
/* ------- Optimized Element Sort --------------------------------------- */
/* ---------------------------------------------------------------------- */

const int Elem_Search_Size = 32;

class Elem_Sort
{
   void *arr;
   int upb_x, upb_y;

   Elem_List list;
   Vec2 base, delta;

   double tol;

   Elem_Sort(const Elem_Sort& cp);             // No Copying
   Elem_Sort& operator=(const Elem_Sort& src); // No Assignment

  public:
   Elem_Sort(Elem_List& el_lst, double dist_tol);
   ~Elem_Sort();

   int  Elements() const { return list.Length(); }

   bool Chain(Elem_List& new_cont);
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

extern bool Remove_Short_Elems(Elem_Cursor& elc,
                                            double maxlen, double maxdist);


} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
