/* ---------------------------------------------------------------------- */
/* ---------------- Element Lists & Closed/Open Contours ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

// $Id: Contour.h,v 1.2 2010/02/05 19:57:10 Clemens Exp $

#ifndef CONTOUR_INC
#define CONTOUR_INC

#include "Rect.h"
#include "Elem.h"
#include "it_gen.h"

#include "PersistentIO.h"

namespace Ino
{
  class Trf2;
}

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

extern const int Cont_Sub_Rect_Max_Elems;
extern const double Cont_Sub_Rect_Max_Area_Rel;

class Elem_Rect_List;

class Contour;
class Cont_Clsd;
class Cont_Nest;
class Cont_Area;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

extern void Cont_On_Error(void (*Error_Handler)(int error_no));

/* ---------------------------------------------------------------------- */
/* -------- Private Class for Inert Properties -------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_Inert
{
   bool area_valid, inert_valid;
   double area,cogx,cogy,ix,iy,ixy;

   Cont_Inert();
   Cont_Inert(const Cont_Inert& cp);
   
   Cont_Inert& operator=(const Cont_Inert& src);
   
   Cont_Inert& operator+= (const Cont_Inert& src);
   
   void invalidate();
   
   void calc_area(const Contour&   cont);      
   void calc_area(const Cont_Nest& nest);      
   void calc_area(const Cont_Area& ar);      

   void calc_inert(const Contour&   cont);      
   void calc_inert(const Cont_Nest& nest);      
   void calc_inert(const Cont_Area& ar);      
  public:
  
   double Area() const { return area; }
   Vec2   Cog()  const { return Vec2(cogx,cogy); }
   double Ix()   const { return ix;  }
   double Iy()   const { return iy;  }
   double Ixy()  const { return ixy; }

   friend class Contour;
   friend class Cont_Nest;
   friend class Cont_Area;
};

/* ---------------------------------------------------------------------- */
/* ------- Const Point on Contour --------------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_Pnt
{
    const Contour *mycnt;

    Elem_C_Cursor elemc;

    bool at_joint;

    Vec3 pt;
    Vec3 tg_bef;
    Vec2 tg_bef_xy;
    Vec3 tg_aft;
    Vec2 tg_aft_xy;

    double curve_bef, curve_aft;

    double rel_par;

    bool calc_point_attr();
    bool to_par(double par);

    Cont_Pnt(const Contour& cnt, const Elem_C_Cursor& elc, double rpar,
                                                      const Vec3& pnt);

  public:
    Cont_Pnt();
    Cont_Pnt(const Cont_Pnt& cp);
    Cont_Pnt(const Contour& src_cnt, double par);

    Cont_Pnt& operator=(const Cont_Pnt& src);

    const Contour* Parent_Contour() const { return mycnt; }

    bool operator==(const Cont_Pnt& pnt) const;
    bool operator!=(const Cont_Pnt& pnt) const
                                        { return !operator==(pnt); }

    bool At_Joint() const { return at_joint; }
    bool Par_Dist_To(const Cont_Pnt& pnt, double& pdist) const;
    bool Min_Par_Dist_To(const Cont_Pnt& pnt, double& pdist) const;

    bool Become_Midpoint_Of(const Cont_Pnt& pnt1, const Cont_Pnt& pnt2);
    
    double Par() const;
    double Rel_Par() const { return rel_par; }

    const Vec3& P() const            { return pt; }
    const Vec3& Tang_Before()    const { return tg_bef; }
    const Vec3& Tang_After()     const { return tg_aft; }

    const Vec2& Tang_Before_XY() const { return tg_bef_xy; }
    const Vec2& Tang_After_XY()  const { return tg_aft_xy; }

    double      Curve_Before() const { return curve_bef; }
    double      Curve_After()  const { return curve_bef; }

    Vec2        Normal_Before_XY() const;
    Vec2        Normal_After_XY () const;

    const Elem_C_Cursor& Cursor() const { return elemc; }
    operator const Elem_C_Cursor& () const { return elemc; }

    operator bool() const;

    bool To_Nearest_End(double tol);

    void Reverse(bool elem_reversed);
    bool Update();

    bool Contiguous_Upto(const Cont_Pnt& upto,
                        double tol = 2.0*Vec2::IdentDist) const;
    bool Contiguous_Upto(const Cont_Pnt& upto, double tol,
                                       Elem_C_Cursor& elc) const;

    bool Extract_Upto(const Cont_Pnt& upto, Contour& into) const;

    friend class Cont_PPair;
    friend class Cont1_Isect_List;
    friend class Cont2_Isect_List;
    friend class Contour;
    friend class Cont_Final;
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

extern bool Cont_Par_Between(const Cont_Pnt& p1, const Cont_Pnt& p2,
                                                           double par);

/* ---------------------------------------------------------------------- */
/* ------- Contour Point Pair (Range) ----------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_PPair
{
  Cont_Pnt one, two;
  bool full_range;

  Cont_PPair(const Cont_Pnt& pnt1, const Cont_Pnt& pnt2);
  // pnt1 end pnt2 MUST be on same contour: only for friends!

 public:
  Cont_PPair() : one(), two(), full_range(false) {}
  Cont_PPair(const Contour& cnt, double par1, double par2);

  Cont_PPair(const Cont_PPair& cp);

  Cont_PPair& operator=(const Cont_PPair& src);

  const Cont_Pnt& One() const { return one; }
  const Cont_Pnt& Two() const { return two; }

  Cont_Pnt Mid() const;

  operator bool() const { return one && two; }

  bool Is_Full() const { return full_range; }
  void Is_Full(bool full);
  
  void Reverse(bool elem_reversed);
  bool Update();
  void Swap();

  bool Par_In_Range(double par, double ext_tol = 0.0) const;
  bool Range_Inside_1(const Elem& el, double& bpar, double& epar) const;
  bool Range_Inside_2(const Elem& el, double& bpar, double& epar) const;
  
  bool Extract_Into(Contour& ex_cnt) const
                              { return one.Extract_Upto(two,ex_cnt); }
                              
  // Project on range
  bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp, double& dist_xy) const;

  friend class Cont1_Isect_List;
  friend class Cont2_Isect_List;
};

/* ---------------------------------------------------------------------- */
/* ------- List of Point Pairs ------------------------------------------ */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Cont_PPair> > Cont_PPair_Alloc;
typedef IT_D_List<Cont_PPair, Cont_PPair_Alloc> Cont_PPair_List;
typedef Cont_PPair_List::C_Cursor Cont_PPair_C_Cursor;
typedef Cont_PPair_List::Cursor   Cont_PPair_Cursor;

/* ---------------------------------------------------------------------- */
/* ------- General Open or Closed Contour ------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_List;
class Contour: public Persistable, protected Rect_Ax
{
  double len_xy;
  double len;

  mutable Elem_List el_list;
  mutable Elem_Rect_List *el_rect_list;

  mutable bool intersecting_valid;
  mutable bool intersecting;

  bool is_closed;

  int mark;

  mutable Cont_Inert inert;

  mutable void *parent;

  mutable int persistLstLen;
  mutable Elem **persistLst;

  void calc_invar(double tol = Vec2::IdentDist);

  void inval_rects() const;
  void copy_invar_to(Contour& dst) const;
  void build_rect_list() const;
  void sharpen_Offset(double limAng, bool noArcs);

  void cleanSingle(bool closed, double offset);

 public:
 
  Contour();
  Contour(const Elem_List& ellist, Elem_List* waste = NULL);

  // This constructor may select an unexpected element as the
  // first element, if the contour is closed !!!
  // Needs rework. CW 2010-02-05
  // Contour(Elem_List& newElList, double tol);

  Contour(const Vec2& cntr, double rad, bool ccw = true);
  Contour(const Rect_Ax& rct);
  
  Contour(const Contour& cp);
  virtual ~Contour();

  Contour& operator=(const Contour& src);

  bool setColor(long newColor);
  bool setColor(const Elem_C_Cursor& elc, long newColor);

  void Move_To(Contour& dst);   // Move contents destructively
  void Move_To(Elem_List& dst); // Move contents destructively

  void Delete();

  operator const Elem_List& () const { return el_list; }
  const Elem_List& List() const { return el_list; }

  bool Closed() const { return is_closed; }

  bool FindClosedRange(const Elem_C_Cursor& elc,
                           Elem_C_Cursor& bElc, Elem_C_Cursor& eElc) const;
  bool Empty() const;
  int  Elem_Count() const;
  bool Self_Intersecting() const;

  const Rect_Ax& Rect() const { return *this; }

  Vec3 Begin_Point() const;
  Vec3 End_Point() const;

  void Begin_Par(double par);

  double Begin_Par() const;
  double End_Par()   const;

  bool At_Par(double par, Vec3& p) const;

  double Len_XY()  const { return len_xy; }
  double Len()     const { return len; }
  double Area_XY() const;
  bool Area_XY(const Elem_C_Cursor& bElc, const Elem_C_Cursor& eElc,
                                                       double& area) const;

  const Cont_Inert& Inert() const;

  void  Parent(void *newparent) { parent = newparent; }
  void *Parent() const { return parent; }

  bool Set_Elem_Id(const Elem_C_Cursor& elc, int newId);

  void Set_Elem_Ids(int newid);
  void Find_First_Id_With(int id, Elem_Cursor& elc) const;

  void Sequence_Elem_Ids(int start_id = 0);
  void End_With_High_Id();

  void Set_Elem_Cnt_Ids(int newid);
  void Set_Elem_P_Cnt_Ids(int newid);
  void Increment_Elem_Cnt_Ids(int diffid);
  void Increment_Elem_P_Cnt_Ids(int diffid);

  void Find_First_Cnt_Id_With(int id, Elem_Cursor& elc) const;

  void Intersect_With_XY(const Contour& cnt, 
                         Cont_PPair_List& isct_lst,
                         bool cleanup = true) const;

  bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp, double& dist_xy) const;

  bool Offset_Into(double offdist, Cont_List& cnt_list) const;
  bool Offset_Into(double offdist, Cont_List& cnt_list,
                                       double limAng,bool noArcs) const;
  bool Offset_Back(double offdist, Contour& bcnt) const;

  bool OffsetSingle_Into(double offdist, Contour& cnt,
                                       double limAng, bool noArcs) const;

  bool Split_At(Cont_Pnt& p);
  bool Split_At(Cont_Pnt& p, Contour& succ_cont);
  bool Start_At(Cont_Pnt& p);

  void Reverse();

  void Transform(const Trf2& trf);

  void Del_Info();

  void Merge_Elems(bool limit_arcs = false);

  void Set_Elem_Z(double newz);
  void Start_Outside(const Rect_Ax& rct);

  bool Min_Left_Rad(double& min_rad) const;
  bool Min_Right_Rad(double& min_rad) const;

  void MakeNonIntersecting(bool left);

  void SetIds(int parentId, int cntId);

  // Takes elements INCLUDING upto!
  bool TakeElems(Elem_C_Cursor& from, Elem_C_Cursor& upto,
                                      Contour& dst, Elem_C_Cursor& insElc);
  // Takes elements INCLUDING upto!
  bool TakeElems(Elem_C_Cursor& from, Elem_C_Cursor& upto,
                                         Contour& dst, bool prepend=false);

  bool IsContiguous(double tol=2.0*Vec2::IdentDist) const;
  bool IsContiguous(const Elem_C_Cursor& elc) const;

  bool ConnectTo(Contour& cnt2, double tol);

  bool OffsetElem(const Elem_C_Cursor& elc, double offset);
  bool TrimElem(const Elem_C_Cursor& elc, double tol);
  bool RemoveElem(Elem_C_Cursor& elc);

  // Circles are silently appended to the end!
  void InsertElem(Elem_C_Cursor& insElc,const Elem& el);

  void PrependElem(const Elem& el);
  void AppendElem(const Elem& el);

  bool SplitElem(const Elem_C_Cursor& celc, int parts);

  void Resort();

  static void CleanupMem();

  // Persistent Section

  Contour(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;
  virtual void postProcess(PersistentReader& pi);
  virtual void postProcess(PersistentWriter& po) const;

  friend class Cont_Clsd;
  friend class Cont_Pnt;
  friend class Cont_PPair;
  friend class Cont1_Isect_List;
  friend class Cont2_Isect_List;
  friend class Cont_List;
  friend class Cont_Area;
  friend class Cont_Pocket;
  friend class Cont_Final;
  friend class Cont_Mill_Old;
};

/* ---------------------------------------------------------------------- */
/* ------- Reverse Sort on rectangle area ------------------------------- */
/* ---------------------------------------------------------------------- */

struct Cont_Area_Cmp
{
  static int Compare(const Contour& cnt1, const Contour& cnt2);
};

/* ---------------------------------------------------------------------- */
/* ------- Closed Contour ----------------------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_Clsd
{
   Contour cont;

   bool lccw;

   void calc_invar();

  public:

   Cont_Clsd();
   Cont_Clsd(const Cont_Clsd& cp);
   
   Cont_Clsd(const Vec2& cntr, double rad, bool ccw = true);
   Cont_Clsd(const Rect_Ax& rct);

   Cont_Clsd& operator=(const Cont_Clsd& src);

   void Move_To(Contour& dst);   // Move contents destructively
   void Move_To(Elem_List& dst); // Move contents destructively
   void Move_To(Cont_Clsd& dst); // Move contents destructively
   void Move_To(Cont_Nest& dst); // Move contents destructively
   void Move_To(Cont_Area& dst); // Move contents destructively

   void Delete();

   operator const Contour& () const { return cont; }

   operator const Elem_List& () const { return cont.el_list; }
   const Elem_List& List() const { return cont.el_list; }

   bool Ccw()   const { return lccw; }
   bool Empty() const { return cont.Empty(); }
   int  Elem_Count() const { return cont.Elem_Count(); }

   const Rect_Ax& Rect() const { return cont; }

   Vec3 Begin_Point() const;
   Vec3 End_Point() const;

   void Begin_Par(double par) { cont.Begin_Par(par); }

   double Begin_Par() const { return cont.Begin_Par(); }
   double End_Par()   const { return cont.End_Par  (); }

   bool At_Par(double par, Vec3& p) const { return cont.At_Par(par,p); }

   double Len_XY()  const { return cont.len_xy; }
   double Len()     const { return cont.len; }
   double Area_XY() const { return cont.Area_XY(); }

   const Cont_Inert& Inert() const { return cont.Inert(); }

   void  Parent(void *newparent) { cont.parent = newparent; }
   void *Parent() const { return cont.parent; }

   void Reverse();

   void Set_Elem_Z(double z) { cont.Set_Elem_Z(z); }
   
   void Set_Elem_Ids(int newid) { cont.Set_Elem_Ids(newid); }

   void Sequence_Elem_Ids(int start_id = 0);
   void End_With_High_Id();

   void Set_Elem_Cnt_Ids(int newid)   { cont.Set_Elem_Cnt_Ids(newid); }
   void Set_Elem_P_Cnt_Ids(int newid) { cont.Set_Elem_P_Cnt_Ids(newid); }

   void Intersect_With_XY(const Contour& cnt, 
                          Cont_PPair_List& isct_lst) const;

   bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                   double& dist_xy) const;

   bool Offset_Into(double offdist, Cont_Area& ar_list) const;
   bool Offset_Back(double offdist, Contour& bcnt) const;

   bool Split_At(Cont_Pnt& p);
   bool Start_At(Cont_Pnt& p);

   void Transform(const Trf2& trf);

   void Del_Info();

   void Merge_Elems(bool limit_arcs = false);

   bool Min_Left_Rad(double& min_rad) const
                                    { return cont.Min_Left_Rad(min_rad);  }
   bool Min_Right_Rad(double& min_rad) const
                                    { return cont.Min_Right_Rad(min_rad); }

   friend class Cont_Pnt;
   friend class Cont_Nest;
   friend class Cont_Area;
   friend class Cont1_Isect_List;
   friend class Cont2_Isect_List;
   friend class Cont_Pocket;
   friend class Cont_Final;
   friend class Cont_Mill_Old;
};

/* ---------------------------------------------------------------------- */
/* ------- List of Contours --------------------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Contour> > Cont_Alloc;
typedef IT_D_List<Contour, Cont_Alloc> Cont_D_List;
typedef Cont_D_List::C_Cursor Cont_C_Cursor;
typedef Cont_D_List::Cursor Cont_Cursor;

/* ---------------------------------------------------------------------- */
/* ------- General Contour List ----------------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_List : protected Rect_Ax
{
   Cont_D_List contlst;

   void calc_invar();

  public:
   Cont_List();
   Cont_List(const Cont_List& cp);

   Cont_List& operator=(const Cont_List& src);
   Cont_List& operator=(const Cont_Nest& src);
   Cont_List& operator=(const Cont_Area& src);

   void Move_To(Cont_List& dst);  // Move contents destructively

   void Delete();

   operator const Cont_D_List& () const { return contlst; }
   const Cont_D_List& List() const { return contlst; }

   bool Empty() const { return !contlst; }

   const Rect_Ax& Rect() const { return *this; }

   int Elem_Count() const;

   void Begin_Par(double par);

   double Begin_Par() const;
   double End_Par()   const;

   void Parent(void *newparent);
   void Sequence_Elem_Ids(int start_id = 0);

   void Set_Elem_Ids(int newid);
   void Set_Elem_Cnt_Ids(int newid);

   bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp, double& dist_xy) const;

   void Reverse();

   void Transform(const Trf2& trf);

   void Del_Info();

   void Merge_Elems(bool limit_arcs = false);

   friend class Contour;
   friend class Cont_Nest;
   friend class Cont_Area;
   friend class Cont1_Isect_List;
   friend class Cont2_Isect_List;
   friend class Cont_Pocket;
   friend class Cont_Final;
   friend class Cont_Mill_Old;
};

/* ---------------------------------------------------------------------- */
/* ------- List of Closed Contours -------------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Cont_Clsd> > Cont_Clsd_Alloc;
typedef IT_D_List<Cont_Clsd, Cont_Clsd_Alloc> Cont_Clsd_D_List;
typedef Cont_Clsd_D_List::C_Cursor Cont_Clsd_C_Cursor;
typedef Cont_Clsd_D_List::Cursor Cont_Clsd_Cursor;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_Nest : protected Rect_Ax
{
  Cont_Clsd_D_List contlst;

  bool lccw;

  mutable Cont_Inert inert;

  void calc_invar();

  bool uni_source() const;
  bool uni_source(int el_id) const;

 public:
  Cont_Nest();
  Cont_Nest(const Cont_Clsd& cl_cont);
  Cont_Nest(const Cont_Nest& cp);

  Cont_Nest& operator=(const Cont_Nest& src);
  Cont_Nest& operator=(const Cont_Clsd& src);

  void Move_To(Cont_Clsd_D_List& dst);
  void Move_To(Cont_List& dst);
  void Move_To(Cont_Nest& dst);
  void Move_To(Cont_Area& dst);

  void Delete();

  bool Empty() const { return !contlst; }

  const Rect_Ax& Rect() const { return *this; }
  operator const Cont_Clsd_D_List& () const { return contlst; }
  const Cont_Clsd_D_List& List() const { return contlst; }

  int Elem_Count() const;
  bool Ccw() const { return lccw; }

  void   Begin_Par(double par);
  double Begin_Par() const;
  double End_Par  () const;

  double Area_XY() const;
  const Cont_Inert& Inert() const;

  void Set_Elem_Ids(int newid);
  void Sequence_Elem_Ids(int start_id = 0);
  void Set_Elem_Cnt_Ids(int newid);
  void Set_Elem_P_Cnt_Ids(int newid);
  void Sequence_Elem_Cnt_Ids(int& start_id);

  bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                double& dist_xy) const;

  bool Offset_Into(double offdist, Cont_Area& ar_list) const;

  void Reverse();

  void Del_Info();

  void Set_Elem_Z(double new_z);

  void Merge_Elems(bool limit_arcs = false);

  bool Min_Left_Rad(double& min_rad) const;
  bool Min_Right_Rad(double& min_rad) const;

  friend class Cont_Clsd;
  friend class Cont_Area;
  friend class Cont1_Isect_List;
  friend class Cont2_Isect_List;
  friend class Cont_Pocket;
  friend class Cont_Final;
  friend class Cont_NcJob;
  friend class Cont_Mill_Old;
};

/* ---------------------------------------------------------------------- */
/* ------- List of Cont_Nest's ------------------------------------------ */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Cont_Nest> > Cont_Nest_Alloc;
typedef IT_D_List<Cont_Nest, Cont_Nest_Alloc> Cont_Nest_D_List;
typedef Cont_Nest_D_List::C_Cursor Cont_Nest_C_Cursor;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

class Cont_Area : protected Rect_Ax
{
  Cont_Nest_D_List nestlst;

  bool lccw;

  double z;

  mutable Cont_Inert inert;

  void calc_invar();

  bool build_from(Cont_Clsd_D_List& cnt_list);

  bool append_clsd_cnt(const Cont_Clsd& cont);

  bool compile_from_cont(Elem_List& el_lst, bool is_ccw,
                                            bool to_left, double tol);

 public:
  Cont_Area() : Rect_Ax(), nestlst(), lccw(false), z(0.0) {}
  Cont_Area(const Cont_Clsd& cl_cont);
  Cont_Area(const Cont_Nest& nest);

  Cont_Area(const Cont_Area& cp);

  Cont_Area(Elem_List& src, Cont_List& lst,
                                         Cont_PPair_List& intersections);
  Cont_Area(Elem_List& src, Cont_List& lst,
                             Cont_PPair_List& intersections, double tol);

  Cont_Area& operator=(const Cont_Area& src);
  Cont_Area& operator=(const Cont_Nest& src);
  Cont_Area& operator=(const Cont_Clsd& src);

  void Move_To(Elem_List& dst);
  void Move_To(Cont_Clsd_D_List& dst);
  void Move_To(Cont_List& dst);
  void Move_To(Cont_Area& dst);
 
  void Delete();

  bool Empty() const { return !nestlst; }

  double Z() const { return z; }
  void   Z(double newz) { z = newz; }

  const Rect_Ax& Rect() const { return *this; }
  operator const Cont_Nest_D_List& () const { return nestlst; }
  const Cont_Nest_D_List& List() const { return nestlst; }

  int Elem_Count() const;
  int Contour_Count() const;
  int Nest_Count() const;

  bool Ccw() const { return lccw; }

  void   Begin_Par(double par);
  double Begin_Par() const;
  double End_Par  () const;

  void Set_Elem_Ids(int newid);
  void Sequence_Elem_Ids(int start_id = 0);

  void Find_Last_Pred_Of(int id, Elem_Cursor& elc) const;
  void Find_First_Succ_Of(int id, Elem_Cursor& elc) const;

  void Find_First_Id_With(int id, Elem_Cursor& elc) const;
  void Set_Elem_Cnt_Ids(int newid);
  void Set_Elem_P_Cnt_Ids(int newid);
  void Sequence_Elem_Cnt_Ids(int& start_id);

  double Area_XY() const;
  const Cont_Inert& Inert() const;

  void Set_Elem_Z(double new_z);

  void Remove_Inners();

  bool Extract_Via(const Vec2& p1, const Vec2& probedir1,
                   const Vec2& p2, const Vec2& probedir2,
                   Contour& path,  double& pathlen) const;

  bool Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                 double& dist_xy) const;
  bool Par_Dist_Along(const Vec2& sp, const Vec2& ep,
                      Cont_Pnt& spnt, double& sdist,
                      Cont_Pnt& epnt, double& edist,
                      double& pdist, bool& rev) const;

  bool Offset_Into(double offdist, Cont_Area& ar_list) const;

  bool Combine_With(bool rev1, const Cont_Area& ar2, bool rev2,
                                bool to_left, Cont_Area& into,
                                Cont_List *rest1 = NULL,
                                Cont_List *rest2 = NULL) const;
  void Reverse();

  void Del_Info();

  void Delete_Unisource_Nests();
  void Delete_Non_Unisource_Nests(int el_id);

  void Merge_Elems(bool limit_arcs = false);

  void Start_Outside();

  void Filter_Arcs(bool ccw, double tol);
  
  bool Min_Left_Rad(double& min_rad) const;
  bool Min_Right_Rad(double& min_rad) const;
  
  friend class Cont_Clsd;
  friend class Cont_Nest;
  friend class Cont1_Isect_List;
  friend class Cont2_Isect_List;
  friend class Cont_Pocket;
  friend class Cont_Final;
  friend class Cont_Mill_Old;
  friend class Cont_NcJob;
  friend class Cont_Pr_Lvl;
  friend class Cont_Slice;
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Cont_Area> > Cont_Area_Alloc;
typedef IT_D_List<Cont_Area,Cont_Area_Alloc>  Cont_Area_List;
typedef Cont_Area_List::C_Cursor              Cont_Area_C_Cursor;
typedef Cont_Area_List::Cursor                Cont_Area_Cursor;

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
