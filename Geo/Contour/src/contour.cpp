/* ---------------------------------------------------------------------- */
/* ---------------- Element Lists & Closed/Open Contours ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "Contour.h"
#include "contouri.hi"
#include "cntpanic.hi"

#include "Base_Arr.h"

#include "it_gen.h"
#include "it_dlist.h"
#include "sub_rect.hi"
#include "Trf.h"

#include "contisct.hi"
#include "El_Line.h"
#include "El_Arc.h"
#include "El_Cir.h"
#include "Geo.h"

// #include "base_arr.h"

#include <math.h>
#include <stdlib.h>

#include "Exceptions.h"

namespace Ino
{

const int Cont_Sub_Rect_Max_Elems = 16;
const double Cont_Sub_Rect_Max_Area_Rel = 0.25;

/* ---------------------------------------------------------------------- */

template<> Isect_Alloc *Isect_Alloc::root = NULL;
template<> Elem_Alloc *Elem_Alloc::root = NULL;

typedef IT_Chain_Alloc<IT_D_Item<Elem_Cursor> > Elem_Cursor_Alloc;
template<> Elem_Cursor_Alloc* Elem_Cursor_Alloc::root = NULL;

template<> Sub_Rect_Alloc *Sub_Rect_Alloc::root = NULL;
template<> Cont_PPair_Alloc *Cont_PPair_Alloc::root = NULL;
template<> Cont_Ref_Alloc *Cont_Ref_Alloc::root = NULL;
template<> Cont_Isect_Alloc *Cont_Isect_Alloc::root = NULL;
template<> IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >
               *IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >::root = NULL;
template<> Cont_Alloc *Cont_Alloc::root = NULL;
template<> Cont_Clsd_Alloc *Cont_Clsd_Alloc::root = NULL;
template<> Cont_Nest_Alloc *Cont_Nest_Alloc::root = NULL;
template<> Cont_Area_Alloc *Cont_Area_Alloc::root = NULL;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::CleanupMem()
{
  Isect_Alloc::Cleanup();
  Elem_Alloc::Cleanup();
  Elem_Cursor_Alloc::Cleanup();
  Sub_Rect_Alloc::Cleanup();
  Cont_PPair_Alloc::Cleanup();
  Cont_Ref_Alloc::Cleanup();
  Cont_Isect_Alloc::Cleanup();
  IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >::Cleanup();
  Cont_Alloc::Cleanup();
  Cont_Clsd_Alloc::Cleanup();
  Cont_Nest_Alloc::Cleanup();
  Cont_Area_Alloc::Cleanup();
  
  Elem_Line::CleanupStore();
  Elem_Arc::CleanupStore();
  Elem_Circle::CleanupStore();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void (*Panic)(int error_no) = NULL;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_On_Error(void (*Error_Handler)(int error_no))
{
  Panic = Error_Handler;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Panic(int error_no)
{
  if (Panic) Panic(error_no);

  throw IllegalStateException("Cont_Panic");
  // exit(1);
}

/* ---------------------------------------------------------------------- */
/* -------- Inert Properties -------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Inert::Cont_Inert()
 : area_valid(false), inert_valid(false),
   area(0.0), cogx(0.0), cogy(0.0), ix(0.0), iy(0.0), ixy(0.0)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Inert::Cont_Inert(const Cont_Inert& cp)
 : area_valid(cp.area_valid), inert_valid(cp.inert_valid),
   area(cp.area), cogx(cp.cogx), cogy(cp.cogy),
   ix(cp.ix), iy(cp.iy), ixy(cp.ixy)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Inert& Cont_Inert::operator=(const Cont_Inert& src)
{
  area_valid  = src.area_valid;
  inert_valid = src.inert_valid;
  area        = src.area;
  cogx        = src.cogx;
  cogy        = src.cogy;
  ix          = src.ix;
  iy          = src.iy;
  ixy         = src.ixy;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Inert& Cont_Inert::operator+=(const Cont_Inert& src)
{
  if (!src.area_valid) return *this;
  
  if (!area_valid) area = 0.0;

  area += src.area;
  area_valid = true;

  if (src.inert_valid) {
    if (!inert_valid) {
      cogx = 0.0;
      cogy = 0.0;
      ix   = 0.0;
      iy   = 0.0;
      ixy  = 0.0;  
    }

    inert_valid = true;
  }

  cogx = (cogx*area + src.cogx*src.area)/(area+src.area);
  cogy = (cogy*area + src.cogy*src.area)/(area+src.area);

  if (src.area >= 0.0) {
    ix   += src.ix;
    iy   += src.iy;
    ixy  += src.ixy;
  }
  else {
    ix   -= src.ix;
    iy   -= src.iy;
    ixy  -= src.ixy;
  }

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::invalidate()
{
  area_valid  = false;
  inert_valid = false;

  area = 0.0;
  cogx = 0.0;
  cogy = 0.0;
  ix   = 0.0;
  iy   = 0.0;
  ixy  = 0.0;  
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static int abs_double_cmp(const void *v1, const void *v2)
{
  if (fabs(*(const double *)v1) < fabs(*(const double *)v2)) return -1;
  else                       return  1;
}

void Cont_Inert::calc_area(const Contour& cont)
{
  area_valid = false;
  area = 0.0;

  const Elem_List& ellst = cont.List();
  
  if (!ellst) return;

  IB_Dbl_Arr workarr(ellst.Length()*2+1);

  Elem_C_Cursor elc(ellst);

  double org_y = elc->El().P1().y;

  int nel = 0;

  for (;elc;++elc) {
    const Elem& el = elc->El();

    workarr[nel++] = el.Area_XY_P1();
    workarr[nel++] = (org_y - el.P1().y) * (el.P2().x - el.P1().x);

    // Elements assumed exactly connected !!!!
  }

  // Sort and sum is done for better accurracy! :

  qsort(&(workarr[0]), nel, sizeof(double), abs_double_cmp);
  
  int i = 0;
  area = 0.0;

  for (i=0; i<nel; i++) area += workarr[i];

  area_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::calc_area(const Cont_Nest& nest)
{
  area_valid = false;
  area = 0.0;

  int listlen = nest.List().Length();
  if (listlen < 1) return;

  IB_Dbl_Arr workarr(listlen);
  
  Cont_Clsd_C_Cursor cc(nest.List());

  listlen = 0;
  for (;cc;++cc) workarr[listlen++] = cc->Area_XY();
  
  qsort(&(workarr[0]),listlen,sizeof(double),abs_double_cmp);

  int i=0;
  area = 0.0;

  for (i=0; i<listlen; i++) area += workarr[i];

  area_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::calc_area(const Cont_Area& ar)
{
  area_valid = false;
  area = 0.0;

  int listlen = ar.Contour_Count();
  if (listlen < 1) return;

  IB_Dbl_Arr workarr(listlen);
  
  Cont_Nest_C_Cursor nsc(ar.List());

  listlen = 0;
  for (;nsc;++nsc) {
    Cont_Clsd_C_Cursor cc(*nsc);
    for (;cc;++cc) workarr[listlen++] = cc->Area_XY();
  }
  
  qsort(&(workarr[0]),listlen,sizeof(double),abs_double_cmp);

  int i=0;
  area = 0.0;

  for (i=0; i<listlen; i++) area += workarr[i];

  area_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::calc_inert(const Contour& cont)
{
  inert_valid = false;
 
  if (!area_valid) calc_area(cont);

  const Elem_List& ellst = cont.List();

  if (!area_valid || !ellst) return;
  
  IB_Dbl_Arr cogx_arr(ellst.Length()+1);
  IB_Dbl_Arr cogy_arr(ellst.Length()+1);

  Elem_C_Cursor elc(ellst);

  int nel = 0;

  for (;elc;++elc) {
    const Elem& el = elc->El();

    cogx_arr[nel]   =  el.Moment_X();
    cogy_arr[nel++] =  el.Moment_Y();
 }

  // Sort and sum is done for better accurracy! :

  qsort(&(cogx_arr[0]), nel, sizeof(double), abs_double_cmp);
  qsort(&(cogy_arr[0]), nel, sizeof(double), abs_double_cmp);
  
  int i = 0;
  cogx = 0.0;
  cogy = 0.0;

  for (i=0; i<nel; i++) {
    cogx += cogx_arr[i];
    cogy += cogy_arr[i];
  }

  cogx /= area;
  cogy /= area;
  
  // Still to do: ix,iy and ixy

  ix  = 0.0;
  iy  = 0.0;
  ixy = 0.0;

  inert_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::calc_inert(const Cont_Nest& nest)
{
  inert_valid = false;
 
  if (!area_valid) calc_area(nest);

  int listlen = nest.List().Length();
  if (listlen < 1) return;

  IB_Dbl_Arr cogx_arr(listlen);
  IB_Dbl_Arr cogy_arr(listlen);
  IB_Dbl_Arr ix_arr(listlen);
  IB_Dbl_Arr iy_arr(listlen);
  IB_Dbl_Arr ixy_arr(listlen);
  
  Cont_Clsd_C_Cursor cc(nest.List());

  listlen = 0;
  for (;cc;++cc) {
    const Cont_Inert& cc_inert = cc->Inert();

    cogx_arr[listlen]  = cc_inert.cogx * cc_inert.area;
    cogx_arr[listlen]  = cc_inert.cogy * cc_inert.area;
    ix_arr[listlen]    = cc_inert.ix;
    iy_arr[listlen]    = cc_inert.iy;
    ixy_arr[listlen++] = cc_inert.ixy;
  }
  
  qsort(&(cogx_arr[0]),listlen,sizeof(double),abs_double_cmp);
  qsort(&(cogy_arr[0]),listlen,sizeof(double),abs_double_cmp);
  qsort(&(ix_arr[0])  ,listlen,sizeof(double),abs_double_cmp);
  qsort(&(iy_arr[0])  ,listlen,sizeof(double),abs_double_cmp);
  qsort(&(ixy_arr[0]) ,listlen,sizeof(double),abs_double_cmp);

  cogx = 0.0;
  cogy = 0.0;
  ix   = 0.0;
  iy   = 0.0;
  ixy  = 0.0;

  for (int i=0; i<listlen; i++) {
    cogx += cogx_arr[i];
    cogy += cogy_arr[i];

    ix   += ix_arr[i];
    iy   += iy_arr[i];
    ixy  += ixy_arr[i];
  }

  cogx /= area;
  cogy /= area;

  inert_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Inert::calc_inert(const Cont_Area& ar)
{
  inert_valid = false;
 
  if (!area_valid) calc_area(ar);

  int listlen = ar.List().Length();
  if (listlen < 1) return;

  IB_Dbl_Arr cogx_arr(listlen);
  IB_Dbl_Arr cogy_arr(listlen);
  IB_Dbl_Arr ix_arr(listlen);
  IB_Dbl_Arr iy_arr(listlen);
  IB_Dbl_Arr ixy_arr(listlen);
  
  Cont_Nest_C_Cursor nsc(ar.List());

  listlen = 0;
  for (;nsc;++nsc) {
    const Cont_Inert& nst_inert = nsc->Inert();

    cogx_arr[listlen]  = nst_inert.cogx * nst_inert.area;
    cogx_arr[listlen]  = nst_inert.cogy * nst_inert.area;
    ix_arr[listlen]    = nst_inert.ix;
    iy_arr[listlen]    = nst_inert.iy;
    ixy_arr[listlen++] = nst_inert.ixy;
  }
  
  qsort(&(cogx_arr[0]),listlen,sizeof(double),abs_double_cmp);
  qsort(&(cogy_arr[0]),listlen,sizeof(double),abs_double_cmp);
  qsort(&(ix_arr[0])  ,listlen,sizeof(double),abs_double_cmp);
  qsort(&(iy_arr[0])  ,listlen,sizeof(double),abs_double_cmp);
  qsort(&(ixy_arr[0]) ,listlen,sizeof(double),abs_double_cmp);

  cogx = 0.0;
  cogy = 0.0;
  ix   = 0.0;
  iy   = 0.0;
  ixy  = 0.0;

  for (int i=0; i<listlen; i++) {
    cogx += cogx_arr[i];
    cogy += cogy_arr[i];

    ix   += ix_arr[i];
    iy   += iy_arr[i];
    ixy  += ixy_arr[i];
  }

  cogx /= area;
  cogy /= area;

  inert_valid = true;
}

/* ---------------------------------------------------------------------- */
/* ------- Calculate tangents etc. -------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::calc_point_attr()
{
  const Elem& el = elemc->El();

  if (rel_par < Vec2::IdentDist) {
    at_joint = true;

    el.Start_Tangent(tg_aft);
    el.Start_Tangent_XY(tg_aft_xy);
    curve_aft = el.Start_Curve();

    tg_bef = tg_aft;
    tg_bef_xy = tg_aft_xy;
    curve_bef = curve_aft;

    const Elem& prvel = elemc.Pred()->El();
    if (prvel.P2().distTo2(el.P1()) < Vec2::IdentDist) {
      prvel.End_Tangent(tg_bef);
      prvel.End_Tangent_XY(tg_bef_xy);
      curve_bef = prvel.End_Curve();
    }
  }
  else if (el.Par_Len() - rel_par < Vec2::IdentDist) {
    at_joint = true;

    el.End_Tangent(tg_bef);
    el.End_Tangent_XY(tg_bef_xy);
    curve_bef = el.End_Curve();

    const Elem& nxtel = elemc.Succ()->El();
    if (nxtel.P1().distTo2(el.P2()) < Vec2::IdentDist) {
      nxtel.Start_Tangent(tg_aft);
      nxtel.Start_Tangent_XY(tg_aft_xy);
      curve_aft = nxtel.Start_Curve();

      ++elemc; if (!elemc) elemc.To_Begin();
      rel_par = 0.0;
    }
    else {
      tg_aft = tg_bef;
      tg_aft_xy = tg_bef_xy;
      curve_aft = curve_bef;
    }
  }
  else {
    double par = rel_par + el.Begin_Par();

    el.Tangent_At(par,tg_bef);       tg_aft    = tg_bef;
    el.Tangent_At_XY(par,tg_bef_xy); tg_aft_xy = tg_bef_xy;

    el.Curve_At(par,curve_bef);      curve_aft = curve_bef;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::to_par(double par)
{
  if (!mycnt || mycnt->Empty()) return false;

  elemc = mycnt->el_list.End();

  double begpar = mycnt->Begin_Par();
  double endpar = mycnt->End_Par();

  if (par < begpar-Vec2::IdentDist ||
                             par > endpar + Vec2::IdentDist) return false;

  if (par < begpar) par = begpar;
  else if (par >= endpar) {
    par = endpar;
    if (mycnt->Closed()) par = begpar;
  }

  if (!mycnt->el_rect_list) {
    mycnt->build_rect_list();
    if (!mycnt->el_rect_list) return false;
  }

  if (!mycnt->el_rect_list->Find_Elem_At_Par(par, elemc)) {
    elemc.To_End();
    return false;
  }
  
  if (!elemc) elemc.To_Last();
  if (!elemc) return false;

  const Elem& el = elemc->El();

  rel_par = par - el.Begin_Par();

  el.At_Par(par, pt);

  return calc_point_attr();
}

/* ---------------------------------------------------------------------- */
/* ------- Special constructor for use by Contour ----------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt::Cont_Pnt(const Contour& cnt, const Elem_C_Cursor& elc,
                             double rpar, const Vec3& pnt)
 : mycnt(&cnt), elemc(elc), at_joint(false), pt(pnt),
   tg_bef(), tg_bef_xy(), tg_aft(), tg_aft_xy(),
   curve_bef(0.0), curve_aft(0.0), rel_par(rpar)
{
}

/* ---------------------------------------------------------------------- */
/* ------- Const Contour Cursor ----------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt::Cont_Pnt()
 : mycnt(NULL), elemc(), at_joint(false), pt(),
   tg_bef(), tg_bef_xy(), tg_aft(), tg_aft_xy(),
   curve_bef(0.0), curve_aft(0.0), rel_par(0.0)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt::Cont_Pnt(const Cont_Pnt& cp)
 : mycnt(cp.mycnt), elemc(cp.elemc), at_joint(cp.at_joint), pt(cp.pt),
   tg_bef(cp.tg_bef), tg_bef_xy(cp.tg_bef_xy),
   tg_aft(cp.tg_aft), tg_aft_xy(cp.tg_aft_xy),
   curve_bef(cp.curve_bef), curve_aft(cp.curve_aft),
   rel_par(cp.rel_par)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt::Cont_Pnt(const Contour& src_cnt, double par)
 : mycnt(&src_cnt), elemc(), at_joint(false),
   pt(), tg_bef(), tg_bef_xy(), tg_aft(), tg_aft_xy(),
   curve_bef(0.0), curve_aft(0.0), rel_par(0.0)
{
  if (!to_par(par)) mycnt = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt& Cont_Pnt::operator=(const Cont_Pnt& src)
{
  mycnt      = src.mycnt;
  elemc      = src.elemc;
  at_joint   = src.at_joint;
  pt         = src.pt;
  tg_bef     = src.tg_bef;
  tg_bef_xy  = src.tg_bef_xy;
  tg_aft     = src.tg_aft;
  tg_aft_xy  = src.tg_aft_xy;
  curve_bef  = src.curve_bef;
  curve_aft  = src.curve_aft;
  rel_par    = src.rel_par;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Pnt::Par() const
{
  if (mycnt) {
    if (elemc) return rel_par + elemc->El().Begin_Par();
    else return mycnt->End_Par();
  }
  return 0.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NAMED_RETURN
Vec2 Cont_Pnt::Normal_Before_XY() const return n(tg_bef_xy);
{
  if (!mycnt) return Vec2();

  n.rot90();
}
#else
Vec2 Cont_Pnt::Normal_Before_XY() const
{
  if (!mycnt) return Vec2();

  Vec2 n(tg_bef_xy);

  n.rot90();

  return n;
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NAMED_RETURN
Vec2 Cont_Pnt::Normal_After_XY() const return n(tg_aft_xy);
{
  if (!mycnt) return Vec2();

  n.rot90();
}
#else
Vec2 Cont_Pnt::Normal_After_XY() const
{
  if (!mycnt) return Vec2();

  Vec2 n(tg_aft_xy);

  n.rot90();

  return n;
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Min_Par_Dist_To(const Cont_Pnt& pnt, double& pdist) const
{
  if (!mycnt || mycnt != pnt.mycnt) return false;

  pdist = pnt.Par() - Par();

  if (mycnt->Closed()) {
    double clen = mycnt->End_Par() - mycnt->Begin_Par();

    if (pdist < 0.0) pdist += clen;
    if (pdist > clen/2.0) pdist = pdist - clen;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Par_Dist_To(const Cont_Pnt& pnt, double& pdist) const
{
  if (!mycnt || mycnt != pnt.mycnt) return false;

  pdist = pnt.Par() - Par();

  if (mycnt->Closed()) {
    double clen = mycnt->End_Par() - mycnt->Begin_Par();

    if (pdist < 0.0) pdist += clen;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Become_Midpoint_Of(const Cont_Pnt& pnt1,
                                  const Cont_Pnt& pnt2)
{
  if (!pnt1.mycnt || pnt1.mycnt != pnt2.mycnt) return false;

  mycnt = pnt1.mycnt;

  double pdist = pnt2.Par() - pnt1.Par();

  double clen = mycnt->End_Par() - mycnt->Begin_Par();

  if (mycnt->Closed() && pdist < 0.0) pdist += clen;

  pdist /= 2.0;
  pdist += pnt1.Par();
  
  if (pdist < mycnt->Begin_Par()) pdist += clen;
  if (pdist > mycnt->End_Par()  ) pdist -= clen;
  
  return to_par(pdist);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::operator==(const Cont_Pnt& pnt) const
{
  double pdist;

  if (Par_Dist_To(pnt,pdist) && fabs(pdist) < Vec2::IdentDist) return true;
  if (pnt.Par_Dist_To(*this,pdist) &&
                                fabs(pdist) < Vec2::IdentDist) return true;

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt::operator bool() const
{
  return mycnt && elemc.Container();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::To_Nearest_End(double tol)
{
  if (!mycnt || !elemc) return false;

  const Elem& el = elemc->El();

  tol = fabs(tol);
  if (tol > el.Par_Len()/2.0) tol = el.Par_Len()/2.0;
  
  if (rel_par <= tol) {
    rel_par = 0.0;
    calc_point_attr();
    
    return true;
  }

  if (el.Par_Len()-rel_par <= tol) {
    ++elemc; if (!elemc) elemc.To_Begin();
    rel_par = 0.0;
    calc_point_attr();

    return true;
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Pnt::Reverse(bool elem_reversed)
{
  Vec3 tg_hold = tg_bef;
  tg_bef = tg_aft;  tg_bef.rot180(); tg_bef.z = -tg_bef.z;
  tg_aft = tg_hold; tg_aft.rot180(); tg_aft.z = -tg_aft.z;

  Vec2 tg_hold_xy = tg_bef_xy;
  tg_bef_xy = tg_aft_xy;  tg_bef_xy.rot180();
  tg_aft_xy = tg_hold_xy; tg_aft_xy.rot180();

  double c_hold =  curve_bef;
  curve_bef     = -curve_aft;
  curve_aft     = -c_hold;

  if (elem_reversed && elemc) {
    const Elem& el = elemc->El();

    double parlen = el.End_Par() - el.Begin_Par();
    if (parlen < 0.0) parlen = -parlen;

    rel_par = parlen - rel_par;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Update()
{
  return to_par(rel_par + elemc->El().Begin_Par()) && calc_point_attr();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
/* ---------------------------------------------------------------------- */

#include <stdio.h>

static FILE *fd = NULL;

static void poutput(const char *head, const Elem_C_Cursor& isc)
{
  if (!fd) fd = fopen("/tmp/clemens","w");

  if (!fd) return;

  Elem_C_Cursor ic(isc);

  fprintf(fd,"\n%s\n",head);
  for (;ic;++ic) {
    fprintf(fd,"P1 = (%.4f,%.4f), P2 = (%.4f,%.4f)\n",
                   ic->El().P1().x,ic->El().P1().y,
                   ic->El().P2().x,ic->El().P2().y);
  }
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Contiguous_Upto(const Cont_Pnt& upto, double tol) const
{
  double pardist;
  if (!Par_Dist_To(upto,pardist) || pardist < 0.0)
                                      return false;

  if (!elemc || !upto.elemc)
                                      return false;

  if (fabs(pardist) <= Vec2::IdentDist)
                                       return true;

  if (elemc == upto.elemc) return true;

  Elem_C_Cursor elc(elemc);
  ++elc; if (!elc) elc.To_Begin();

  for (;;) {
    const Elem& prvel = elc.Pred()->El();
    const Elem& curel = elc->El();

    double dist = prvel.P2().distTo2(curel.P1());
    if (dist > tol)
                    return false;

    if (elc == upto.elemc) break;

    ++elc; if (!elc) elc.To_Begin();
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Contiguous_Upto(const Cont_Pnt& upto, double tol,
                                                  Elem_C_Cursor& elc) const
{
  elc.To_End();

  double pardist;
  if (!Par_Dist_To(upto,pardist) || pardist < 0.0) return false;

  if (!elemc || !upto.elemc) return false;

  elc = elemc;

  if (fabs(pardist) <= Vec2::IdentDist) return true;

  if (elemc == upto.elemc) return true;

  ++elc; if (!elc) elc.To_Begin();

  for (;;) {
    const Elem& prvel = elc.Pred()->El();
    const Elem& curel = elc->El();

    double dist = prvel.P2().distTo2(curel.P1());
    if (dist > tol) return false;

    if (elc == upto.elemc) break;

    ++elc; if (!elc) elc.To_Begin();
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Pnt::Extract_Upto(const Cont_Pnt& upto, Contour& into) const
{
  into.inval_rects();
  into.el_list.Delete();
  into.calc_invar();

  double pardist;
  if (!Par_Dist_To(upto,pardist) || pardist < 0.0) return false;

  if (!elemc || !upto.elemc) return false;

  if (fabs(pardist) >= Vec2::IdentDist) {

    Elem_Cursor intoc(into.el_list);

    // Get first element
    Elem_List spllst; 
    Elem_Cursor splc(spllst);

    if (elemc->El().Split(Par(),spllst)) {
      splc.To_Last();
      splc->El().setInsArc(false);
      intoc.Re_Insert(splc);
    }
    else if (rel_par < elemc->El().Par_Len()/2.0) {
      splc.To_Begin();
      splc->El().setInsArc(false);
      intoc.Re_Insert(splc);
    }
   
    if (elemc == upto.elemc && rel_par < upto.rel_par) {
      spllst.Delete();

      if (intoc->El().Split(upto.Par(),spllst)) {

        intoc.To_Last(); intoc.Delete();

        splc.To_Begin();
        splc->El().setInsArc(false);

        intoc.Re_Insert(splc);
      }
    }
    else {
      intoc.To_End();
      Elem_C_Cursor cpc(elemc); ++cpc; if (!cpc) cpc.To_Begin();

      intoc.Copy_From(cpc,upto.elemc);

      intoc.To_End();
      spllst.Delete();

      if (upto.elemc->El().Split(upto.Par(),spllst) ||
          upto.rel_par > upto.elemc->El().Par_Len()/2.0) {
        splc.To_Begin();
        splc->El().setInsArc(false);
        intoc.Re_Insert(splc);
      }
    }

    intoc.To_Begin(); intoc->El().Stretch_Begin_XY(P());
    intoc.To_Last (); intoc->El().Stretch_End_XY(upto.P());
  }

  into.calc_invar();
  into.Begin_Par(Par());
 
  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Par_Between(const Cont_Pnt& p1, const Cont_Pnt& p2,
                                                           double par)
{
  if (!p1 || p1.Parent_Contour() != p2.Parent_Contour()) return false;
  
  double par1 = p1.Par();
  double par2 = p2.Par();
  
  // Leave extra parenthesis below in if statements: compiler bug!!

  if (p1.Parent_Contour()->Closed()) {
    if (par1 <= par2) {
      if ((par1 <= par) && (par <= par2)) return true;
    }
    else {
      if ((par1 <= par) || (par <= par2)) return true;
    }
  }
  else {
    if (par1 <= par2) {
      if ((par1 <= par) && (par <= par2)) return true;
    }
    else {
      if ((par2 <= par) && (par <= par1)) return true;
    }
  }
  
  return false;
}

/* ---------------------------------------------------------------------- */
/* ------- Point Pairs -------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_PPair::Cont_PPair(const Cont_Pnt& pnt1, const Cont_Pnt& pnt2)
 : one(pnt1), two(pnt2), full_range(false)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_PPair::Cont_PPair(const Cont_PPair& cp)
 : one(cp.one), two(cp.two), full_range(cp.full_range)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_PPair::Cont_PPair(const Contour& cnt, double par1, double par2)
 : one(cnt,par1), two(cnt,par2), full_range(false)
{
  if (!cnt.Closed() && one && two && par2 < par1) {  // Make invalid
    one = Cont_Pnt();
    two = Cont_Pnt();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_PPair& Cont_PPair::operator=(const Cont_PPair& src)
{
  one        = src.one;
  two        = src.two;
  full_range = src.full_range;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Pnt Cont_PPair::Mid() const
{
  Cont_Pnt midpnt;

  if (!midpnt.Become_Midpoint_Of(one,two)) return Cont_Pnt();
  
  return midpnt;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_PPair::Is_Full(bool full)
{
  full_range = full;
  two = one;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_PPair::Reverse(bool elem_reversed)
{
  one.Reverse(elem_reversed);
  two.Reverse(elem_reversed);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_PPair::Update()
{
  return one.Update() && two.Update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_PPair::Swap()
{
  Cont_Pnt pnt(one); one = two; two = pnt;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_PPair::Par_In_Range(double par, double ext_tol) const
{
  if (!one || !two) return false;
  
  if (full_range) return true;

  double par1 = one.Par(), par2 = two.Par();
  
  if (par1 <= par2) {
    if (par < par1-ext_tol || par > par2+ext_tol) return false; 
  }
  else {
    if (par < par1-ext_tol && par > par2+ext_tol) return false; 

    const Contour *cont = one.Parent_Contour();
    if (par < cont->Begin_Par()-ext_tol) return false;
    if (par > cont->End_Par()  +ext_tol) return false;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_PPair::Range_Inside_1(const Elem& el,
                                        double& bpar, double& epar) const
{
  double el_bpar = el.Begin_Par();
  double el_epar = el.End_Par();

  double opar = one.Par();
  double tpar = two.Par();

  if (Par_In_Range(el_bpar)) {
    bpar = el_bpar;
    
    if (Par_In_Range(el_epar)) {
      if (opar > tpar && opar > el_bpar && opar <= el_epar) bpar = opar;
      epar = el_epar;
    }
    else epar = tpar;
  }
  else if (Par_In_Range(el_epar)) {
      bpar = opar;
      epar = el_epar;
  }
  else {

    if (opar < el_bpar || opar > el_epar) return false;
   
    bpar = opar;
    epar = tpar;
  }

  return true;
}
                                        
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_PPair::Range_Inside_2(const Elem& el,
                                        double& bpar, double& epar) const
{
  double el_bpar = el.Begin_Par();
  double el_epar = el.End_Par();

  double opar = one.Par();
  double tpar = two.Par();

  if (Par_In_Range(el_bpar)) {
    bpar = el_bpar;
    
    if (Par_In_Range(el_epar)) {
      if (opar > tpar && tpar >= el_bpar && tpar < el_epar) epar = tpar;
      else             epar = el_epar;
    }
    else epar = tpar;
  }
  else if (Par_In_Range(el_epar)) {
      bpar = opar;
      epar = el_epar;
  }
  else {

    if (opar < el_bpar || opar > el_epar) return false;
   
    bpar = opar;
    epar = tpar;
  }

  return true;
}
                                        
/* ---------------------------------------------------------------------- */
/* ----- Project on enclosed range only (always strict) ----------------- */
/* ---------------------------------------------------------------------- */

bool Cont_PPair::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                  double& dist_xy) const
{
  if (!one || !two) return false;

  const Contour *cont = one.Parent_Contour();

  cntp = Cont_Pnt();
  dist_xy = 0.0;

  if (!cont->el_list) return false;

  if (!cont->el_rect_list) cont->build_rect_list();
  
  Elem_Cursor elc;
  Vec3 pp;
  double parm;

  if (full_range) {
    if (!cont->el_rect_list->Project_Pnt_XY(p,elc,pp,
                                             parm,dist_xy)) return false;
  }
  else if (!cont->el_rect_list->Project_Pnt_XY(p,one.Par(),two.Par(),elc,
                                          pp,parm,dist_xy)) return false;

  Cont_Pnt cp(*cont,elc,parm - elc->El().Begin_Par(),pp);
  cp.calc_point_attr();

  cntp = cp;

  return true;
}

  
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool Filter_Arc_Range(Elem_Cursor& elc, bool ccw, double tol)
{
  Elem_Cursor elc1(elc); ++elc1; if (!elc1) elc1.To_Begin();
  
  if (elc1->El().Type() != Elem_Type_Arc) return false;
  
  Elem_Arc& elarc1 = (Elem_Arc &)elc1->El();
  if (elarc1.Ccw() != ccw) return false;
  
  Elem_Cursor elc2(elc1); ++elc2; if (!elc2) elc2.To_Begin();
  
  if (elc2->El().Type() != Elem_Type_Arc) return false;
  
  Elem_Arc& elarc2 = (Elem_Arc &)elc2->El();
  if (elarc2.Ccw() != ccw) return false;
  
  // Check the transition from elarc1 to elarc2
  
  Vec2 endtg, sttg;
  elarc1.End_Tangent_XY(endtg); endtg.rot90();
  elarc2.Start_Tangent_XY(sttg);

  if ((sttg * endtg > 0.0) == ccw) return false;
  
  Elem_Cursor elc3(elc2); ++elc3; if (!elc3) elc3.To_Begin();
  
  if (elc3->El().Type() != elc->El().Type()) return false;
  

  Vec3 midp1, midp2;  // Midway points of both arcs
  double mpar = (elarc1.Begin_Par() + elarc1.End_Par())/2.0;

  elarc1.At_Par(mpar,midp1);  
  
  mpar = (elarc2.Begin_Par() + elarc2.End_Par())/2.0;

  elarc2.At_Par(mpar,midp2);  
  
  
  // Ok, here we have a potential sequence
  
  if (elc->El().Type() == Elem_Type_Line) {
    Elem_Line& line1 = (Elem_Line &)elc->El();
    Elem_Line& line2 = (Elem_Line &)elc3->El();
    
    Elem_Line new_line(line1.P1(),line2.P2());
    
    double parm, dist_xy;
    Vec3 pp;
    if (!new_line.Project_Pnt_XY(line1.P2(),tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_line.Project_Pnt_XY(line2.P1(),tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_line.Project_Pnt_XY(elarc1.P2(),tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_line.Project_Pnt_XY(midp1,tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_line.Project_Pnt_XY(midp2,tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;
                                           
}
  else if (elc->El().Type() == Elem_Type_Arc) {
    Elem_Arc& arc1 = (Elem_Arc &)elc->El();
    Elem_Arc& arc2 = (Elem_Arc &)elc3->El();
    
    if (arc1.Ccw() != arc2.Ccw()) return false;
    if (arc1.C().distTo2(arc2.C()) > tol) return false;
    
    Elem_Arc new_arc(arc1.P1(),arc2.P2(),arc1.C(),arc1.Ccw());
    
    double parm, dist_xy;
    Vec3 pp;

    if (!new_arc.Project_Pnt_XY(elarc1.P2(),tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_arc.Project_Pnt_XY(midp1,tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;

    if (!new_arc.Project_Pnt_XY(midp2,tol,true,pp,parm,dist_xy) ||
                                           fabs(dist_xy) > tol) return false;
                                           
  }
  
  // Ok lets replace it:

  elc->El().Stretch_End_XY(elc3->El().P2(),false);

  while (elc1 != elc3) {
    if (!elc1) elc1.To_Begin();
    elc1.Delete();
  }
  
  elc3.Delete();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool Filter_Ins_Arcs(Elem_List& ellst, bool ccw, double tol)
{
  Elem_Cursor elc(ellst);

  bool modified = false;
  
  for (;elc;++elc) {
    if (Filter_Arc_Range(elc,ccw,tol)) modified = true;
  }
  
  return modified;
}

/* ---------------------------------------------------------------------- */
/* ---------------- Element Lists & Closed/Open Contours ---------------- */
/* ---------------------------------------------------------------------- */

void Contour::inval_rects() const
{
  if (el_rect_list) delete el_rect_list;
  el_rect_list = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::copy_invar_to(Contour& dst) const
{
  dst.Rect_Ax::operator=(*this);

  dst.len_xy             = len_xy;
  dst.len                = len;
  dst.inert              = inert;
  dst.intersecting_valid = intersecting_valid;
  dst.intersecting       = intersecting;
  dst.is_closed          = is_closed;
  dst.mark               = mark;
  dst.parent             = parent;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::build_rect_list() const
{
  if (el_rect_list) delete el_rect_list;
  el_rect_list = new Elem_Rect_List(el_list,Cont_Sub_Rect_Max_Elems,
                                            Cont_Sub_Rect_Max_Area_Rel *
                                                     Rect_Ax::Area_XY());
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::sharpen_Offset(double limAng, bool noArcs)
{
  limAng = fabs(limAng);

  Elem_Cursor elc(el_list);
  if (!elc) return;

  double bpar = elc->El().Begin_Par();

  for (;elc;++elc) {
    Elem& el = elc->El();

    if (el.isInsArc()) {
      Elem_Arc& arc = (Elem_Arc&) el;
      Vec2 midP;
      arc.Mid_Par_XY(midP);

      if (fabs(arc.Span_Angle()) < limAng) {
        Elem& prvEl = elc.Pred()->El();
        Elem& nxtEl = elc.Succ()->El();

        Isect_Lst isctLst;
        prvEl.Intersect_XY_Ext(nxtEl,false,isctLst);

        if (!isctLst) continue;

        Isect_C_Cursor isc(isctLst);
        Vec2 ip(isc->P);

        if (++isc && isc->P.distTo2(midP) < ip.distTo2(midP)) ip = isc->P;

        prvEl.Stretch_End_XY(ip,false);
        nxtEl.Stretch_Begin_XY(ip,false);

        elc.Delete();
      }
      else if (noArcs) {
        // Replace arc by miter line

        Elem_Arc& arc = (Elem_Arc&) el;

        Elem& prvEl = elc.Pred()->El();
        Elem& nxtEl = elc.Succ()->El();

        Vec2 midP;
        arc.Mid_Par_XY(midP);

        Vec2 mp2(arc.P2()); mp2 -= arc.P1();
        mp2.unitLen2(); mp2 += midP;

        Vec2 ip1,ip2;
        double pr1,pr2;

        if (Geo_Isct_Lines(prvEl.P1(),prvEl.P2(),midP,mp2,
                           false,1e-12,ip1,pr1,pr2) < 1) continue;

        if (Geo_Isct_Lines(nxtEl.P2(),nxtEl.P1(),midP,mp2,
                           false,1e-12,ip2,pr1,pr2) < 1) continue;

        prvEl.Stretch_End_XY(ip1,false);
        nxtEl.Stretch_Begin_XY(ip2,false);

        elc.Delete(); // Remove the arc

        // Replace by miter line
        Elem_Line miterLine(ip1,ip2);
        elc.Insert(miterLine);
      }
    }
  }

  calc_invar();
  Begin_Par(bpar);
}

/* ---------------------------------------------------------------------- */
/* ------- Calculate invariant of contour ------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::calc_invar(double tol)
{
  Rect_Ax::Rect_Update(Rect_Ax());
  len_xy = 0.0;
  len    = 0.0;

  Elem_C_Cursor elc(el_list);

  elc.To_Begin();
  if (!elc) {
    intersecting_valid = false;
    intersecting       = false;
    is_closed          = false;
    return;
  }

  double ppDist = elc->El().P1().distTo2(elc.Pred()->El().P2());
  is_closed = ppDist < tol;

  Rect_Ax::operator=(elc->El().Rect());
  len_xy = elc->El().Len_XY();
  len    = elc->El().Len();

  double last_z = elc->El().P2().z;

  while (++elc) {
    const Elem& el = elc->El();

    Rect_Ax::operator+=(el.Rect());
    len_xy += el.Len_XY();
    len    += el.Len();
    len    += fabs(el.P1().z - last_z);
    last_z  = el.P2().z;
  }

  if (is_closed) {
    elc.To_Begin();
    len += fabs(elc->El().P1().z - last_z);
  }

  intersecting_valid = false;
  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::Contour()
  : Rect_Ax(), len_xy(0.0), len(0.0),
    el_list(), el_rect_list(NULL),
    intersecting_valid(false), intersecting(false),
    is_closed(false), mark(false), inert(), parent(NULL),
    persistLstLen(0), persistLst(NULL)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::Contour(const Elem_List& newellist, Elem_List* waste)
  : Rect_Ax(), len_xy(0.0), len(0.0),
    el_list(), el_rect_list(NULL),
    intersecting_valid(false), intersecting(false),
    is_closed(false), mark(false), inert(), parent(NULL),
    persistLstLen(0), persistLst(NULL)
{
  Elem_C_Cursor elc(newellist);

  if (elc) el_list.Push_Back(elc->El());

  double bpar = elc->El().Begin_Par();

  Vec2 lstp = elc->El().P2();
  ++elc;

  for (;elc;++elc) {
    const Elem& curel = elc->El();

    if (lstp != curel.P1()) break;
    lstp = curel.P2();

    el_list.Push_Back(curel);
  }

  if (waste) {
    waste->Delete();
    for (;elc;++elc) waste->Push_Back(elc->El());
  }

  calc_invar();
  Begin_Par(bpar);
}

/* ---------------------------------------------------------------------- */

//static void updateEndPoints(const Elem_List& elLst, Vec2& fstp, Vec2& lstp)
//{
//  Elem_C_Cursor elc(elLst);
//  if (!elc) return;
//
//  elc.To_Begin();
//  fstp = elc->El().P1();
//
//  elc.To_Last();
//  lstp = elc->El().P2();
//}

/* ---------------------------------------------------------------------- */

//static void findNearestLineArc(const Vec2& pt, Elem_Cursor& elc,
//                                            double& minDist, bool& reverse)
//{
//  Elem_Cursor nearElc(elc);
//  nearElc.To_End();
//
//  elc.To_Begin();
//
//  for (;elc;++elc) {
//    const Elem& curel = elc->El();
//
//    if (curel.Type() == Elem_Type_Circle) return;
//    
//    double dst = curel.P1().distTo2(pt);
//
//    if (!nearElc || dst < minDist) {
//      nearElc = elc;
//      minDist = dst;
//      reverse = false;
//    }
//    
//    dst = curel.P2().distTo2(pt);
//
//    if (!nearElc || dst < minDist) {
//      nearElc = elc;
//      minDist = dst;
//      reverse = true;
//    }
//  }
//
//  elc = nearElc;
//}

/* ---------------------------------------------------------------------- */
// This constructor may select an unexpected element as the
// first element, if the contour is closed !!!
// Needs rework. CW 2010-02-05

// Should first work forward through newElList and if anything
// is left over, only then work backwards.

//Contour::Contour(Elem_List& newElList, double tol)
//  : Rect_Ax(), len_xy(0.0), len(0.0),
//    el_list(), el_rect_list(NULL),
//    intersecting_valid(false), intersecting(false),
//    is_closed(false), mark(false), inert(), parent(NULL),
//    persistLstLen(0), persistLst(NULL)
//{
//  tol = fabs(tol);
//
//  Elem_Cursor elcHead(newElList), elcTail(newElList);
//  Elem_Cursor dstElc(el_list);
//
//  double bpar = 0.0;
//
//  if (elcHead) {
//    elcHead.To_Begin();
//    bpar = elcHead->El().Begin_Par();
//
//    dstElc.To_End();
//    dstElc.Re_Insert(elcHead);
//    dstElc.To_Last();
//
//    if (dstElc->El().Type() == Elem_Type_Circle) {
//      calc_invar();
//      Begin_Par(bpar);
//      return;
//    }
//  }
//
//  double distHead=0.0, distTail = 0.0;
//  bool   revHead = false, revTail = 0.0;
//  Vec2 fstp,lstp;
//
//  while (newElList) {
//    updateEndPoints(el_list,fstp,lstp);
//
//    findNearestLineArc(fstp,elcHead,distHead,revHead);
//    findNearestLineArc(lstp,elcTail,distTail,revTail);
//
//    revHead = !revHead;
//
//    dstElc.To_Begin();
//
//    if (elcTail && (!elcHead || distHead > distTail)) {
//      dstElc.To_End();
//      elcHead = elcTail;
//      distHead = distTail;
//      revHead = revTail;
//    }
//
//    if (!elcHead || distHead > tol) break;
//
//    if (fstp.distTo2(lstp) < distHead) break;
//
//    if (revHead) elcHead->El().Reverse();
//    dstElc.Re_Insert(elcHead);
//  }
//
//  calc_invar(tol);
//  Begin_Par(bpar);
//}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::Contour(const Contour& cp)
  : Persistable(cp), Rect_Ax(cp), len_xy(cp.len_xy), len(cp.len),
    el_list(cp.el_list), el_rect_list(NULL),
    intersecting_valid(cp.intersecting_valid),
    intersecting(cp.intersecting), is_closed(cp.is_closed),
    mark(cp.mark), inert(cp.inert), parent(cp.parent),
    persistLstLen(0), persistLst(NULL)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::Contour(const Vec2& cntr, double rad, bool ccw)
  : Rect_Ax(), len_xy(0.0), len(0.0),
    el_list(), el_rect_list(NULL),
    intersecting_valid(true),
    intersecting(false), is_closed(true),
    mark(false), inert(), parent(NULL),
    persistLstLen(0), persistLst(NULL)
{
   Vec3 p1(cntr,0.0); p1 += Vec2(rad,0.0);

   Elem_Circle circle(p1, 0.0, cntr, ccw);

   el_list.Push_Back(circle);

   calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::Contour(const Rect_Ax& rct)
  : Rect_Ax(), len_xy(0.0), len(0.0),
    el_list(), el_rect_list(NULL),
    intersecting_valid(true),
    intersecting(false), is_closed(true),
    mark(false), inert(), parent(NULL),
    persistLstLen(0), persistLst(NULL)
{
  Vec3 lr(rct.Ur().x,rct.Ll().y);
  Vec3 ul(rct.Ll().x,rct.Ur().y);
  
  el_list.Push_Back(Elem_Line(rct.Ll(),lr));
  el_list.Push_Back(Elem_Line(lr,rct.Ur()));
  el_list.Push_Back(Elem_Line(rct.Ur(),ul));
  el_list.Push_Back(Elem_Line(ul,rct.Ll()));
  
  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour::~Contour()
{
  if (el_rect_list) delete el_rect_list;
  if (persistLst) delete[] persistLst;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Contour& Contour::operator=(const Contour& src)
{
  inval_rects();

  src.copy_invar_to(*this);

  el_list = src.el_list;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::setColor(long newColor)
{
  Elem_Cursor elc(el_list);

  bool changed = false;

  for (;elc;++elc) {
    if (elc->El().setColor(newColor)) changed = true;
  }

  return changed;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::setColor(const Elem_C_Cursor& elc, long newColor)
{
  if (!elc) return false;

  Elem_Cursor lElc(el_list);

  for (;lElc;++lElc) {
    if (lElc == elc) break;
  }

  if (!lElc) return false;

  return lElc->El().setColor(newColor);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Move_To(Contour& dst)
{
  dst.inval_rects();

  copy_invar_to(dst);

  Move_To(dst.el_list);  // See below
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Move_To(Elem_List& dst)
{
  el_list.Move_To(dst);

  inval_rects();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Delete()
{
  el_list.Delete();

  inval_rects();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Empty() const
{
  return !el_list;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Contour::Elem_Count() const
{
  return el_list.Length();
}

/* ---------------------------------------------------------------------- */
// Assumes elements are oriented in right direction
// And circles are always at the end or beginning of the list

bool Contour::FindClosedRange(const Elem_C_Cursor& elc,
                              Elem_C_Cursor& bElc, Elem_C_Cursor& eElc) const
{
  if (!elc || elc.Container() != &el_list) return false;

  double sqTol = sqr(Vec2::IdentDist * 2.0);

  if (elc->El().Type() == Elem_Type_Circle) {
    bElc = elc;
    eElc = elc; ++eElc;
    if (!eElc) eElc.To_Begin();
    return true;
  }

  if (el_list.Length() < 2) {
    bElc.To_End();
    eElc.To_End();

    return false;
  }

  bElc = elc;
  
  // Go backwards until not contiguous

  for (;;) {
    Elem_C_Cursor prvElc = bElc; --prvElc;
    if (!prvElc) prvElc.To_Last();

    if (prvElc->El().Type() == Elem_Type_Circle) break;

    const Elem& curEl = bElc->El();
    const Elem& prvEl = prvElc->El();

    if (curEl.P1().sqDistTo2(prvEl.P2()) > sqTol) break;

    bElc = prvElc;

    if (bElc == elc) {
      eElc = bElc;
      return true;
    }
  }
  
  // Go forward until not contiguous or closed

  const Vec2& bPt = bElc->El().P1();

  eElc = elc;

  for (;;) {
    const Elem& el1 = eElc->El();

    ++eElc;
    if (!eElc) eElc.To_Begin();

    if (bPt.sqDistTo2(el1.P2()) <= sqTol) return true;

    if (eElc->El().Type() == Elem_Type_Circle) break;

    if (eElc == bElc) break;
  }

  bElc.To_End();
  eElc.To_End();

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Self_Intersecting() const
{
  if (!intersecting_valid) {
     Cont_Ref ref(*this);
     Cont1_Isect_List ilst(ref);

     intersecting = !ilst.Empty();

     intersecting_valid = true;
  }

  return intersecting;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::MakeNonIntersecting(bool left)
{
  Cont_Ref ref(*this);
  Cont1_Isect_List iLst(ref,true,left,false);

  if (iLst.Empty()) return;

  Cont_Clsd_D_List cLst;
  iLst.Extract_Closed(cLst);

  if (!cLst) return;

  Cont_Clsd_Cursor cc(cLst), ccMax(cLst);
  double len = 0.0;
  bool fst = true;

  for (;cc;++cc) {
    double cLen = cc->Len_XY();
    if (fst || cLen > len) {
      len = cLen;
      ccMax = cc;
    }

    fst = false;
  }

  *this = *ccMax;
}

/* ---------------------------------------------------------------------- */

bool Contour::IsContiguous(double tol) const
{
  return Elem::IsContiguous(el_list,is_closed,tol);
}

/* ---------------------------------------------------------------------- */

bool Contour::IsContiguous(const Elem_C_Cursor& elc) const
{
  Elem_C_Cursor bElc, eElc;

  return FindClosedRange(elc,bElc,eElc);
}

/* ---------------------------------------------------------------------- */

bool Contour::ConnectTo(Contour& cnt2, double tol)
{
  Elem_Cursor elc1(el_list); elc1.To_Last();
  Elem_Cursor elc2(cnt2.el_list);

  if (!elc1 || !elc2) return false;

  Vec3 p1 = elc1->El().P2();
  Vec3 p2 = elc2->El().P1();

  if (tol > 0.0 && p1.distTo2(p2) > tol) return false;

  elc1->El().Join_To_XY(elc2->El(),false);

  return true;
}

/* ---------------------------------------------------------------------- */

void Contour::SetIds(int parentId, int cntId)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) {
    Elem& el = elc->El();

    el.Cam_Inf(parentId);
    el.Cnt_Id(cntId);
  }
}

/* ---------------------------------------------------------------------- */

bool Contour::TakeElems(Elem_C_Cursor& from, Elem_C_Cursor& upto,
                                              Contour& dst, bool prepend)
{
  if (!from || !upto) return false;
  if (from.Container() != &el_list || upto.Container() != &el_list) return false;

  Elem_Cursor fr(el_list),up(el_list);

  for (;fr;++fr) {
    if (fr == from) break;
  }

  for (;up;++up) {
    if (up == upto) break;
  }

  Elem_Cursor dstElc(dst.el_list);
  if (!prepend) dstElc.To_End();

  while (fr != up) {
    dstElc.Re_Insert(fr);
    if (!fr) fr.To_Begin();
    ++dstElc;
  }

  dstElc.Re_Insert(fr);

  inval_rects();
  calc_invar();

  dst.inval_rects();
  dst.calc_invar();

  from = upto = fr;

  return true;
}

/* ---------------------------------------------------------------------- */

bool Contour::TakeElems(Elem_C_Cursor& from, Elem_C_Cursor& upto,
                                      Contour& dst, Elem_C_Cursor& insElc)
{
  if (!from || !upto) return false;
  if (from.Container() != &el_list || upto.Container() != &el_list) return false;

  if (!insElc.Container()) return false;

  Elem_Cursor fr(el_list),up(el_list);

  for (;fr;++fr) {
    if (fr == from) break;
  }

  for (;up;++up) {
    if (up == upto) break;
  }

  Elem_Cursor dstElc(dst.el_list);
  for (;dstElc;++dstElc) {
    if (dstElc == insElc) break;
  }

  while (fr != up) {
    dstElc.Re_Insert(fr);
    if (!fr) fr.To_Begin();
    ++dstElc;
  }

  dstElc.Re_Insert(fr);

  inval_rects();
  calc_invar();

  dst.inval_rects();
  dst.calc_invar();

  from = upto = fr;

  return true;
}

/* ---------------------------------------------------------------------- */

bool Contour::OffsetElem(const Elem_C_Cursor& elc, double offset)
{
  if (!elc) return false;

  Elem_Cursor lElc(el_list);

  for (;lElc;++lElc) {
    if (lElc == elc) break;
  }

  if (!lElc) return false;

  Elem_List eLst;
  if (!lElc->El().Offset_Into(offset,eLst)) {
    lElc.Delete();
    build_rect_list();
    return true;
  }

  lElc.Delete();
  Elem_Cursor sElc(eLst), eElc(eLst); eElc.To_End();
  lElc.Re_Insert(sElc,eElc);

  inval_rects();
  calc_invar();

  return true;
}

/* ---------------------------------------------------------------------- */

bool Contour::TrimElem(const Elem_C_Cursor& elc, double tol)
{
  if (!elc || el_list.Length() < 2) return false;

  Elem_Cursor lElc(el_list);

  for (;lElc;++lElc) {
    if (lElc == elc) break;
  }

  if (!lElc) return false;

  Elem& el1 = lElc->El();

  ++lElc; if (!lElc) lElc.To_Begin();

  Elem& el2 = lElc->El();

  Isect_Lst iLst;
  el1.Intersect_XY_Ext(el2,true,iLst);

  if (iLst.Length() < 1) { // No Intersection, connect with a line
    if (el1.P2().distTo2(el2.P1()) < tol) el1.Join_To_XY(el2);
    else {
      Elem_Line line(el1.P2(),el2.P1());
      lElc.Insert(line);
    }
  }
  else if (iLst.Length() < 2) {
    Isect_C_Cursor ilc(iLst);

    el1.Stretch_End_XY(ilc->P,false);
    el2.Stretch_Begin_XY(ilc->P,false);
  }
  else {
    Isect_C_Cursor ilc(iLst);

    if (ilc->First.Par < el1.Begin_Par()) ++ilc;
    if (ilc->Last.Par >= el2.End_Par()) {
      if (el1.P2().distTo2(el2.P1()) < tol) el1.Join_To_XY(el2);
      else {
        Elem_Line line(el1.P2(),el2.P1());
        lElc.Insert(line);
      }

      return true;
    }

    el1.Stretch_End_XY(ilc->P,false);
    el2.Stretch_Begin_XY(ilc->P,false);
  }

  inval_rects();
  calc_invar();

  return true;
}

/* ---------------------------------------------------------------------- */

bool Contour::RemoveElem(Elem_C_Cursor& elc)
{
  if (!elc || el_list.Length() < 1) return false;

  double bPar = Begin_Par();

  Elem_Cursor lElc(el_list);

  for (;lElc;++lElc) {
    if (lElc == elc) break;
  }

  if (!lElc) return false;

  lElc.Delete();

  elc = lElc;

  inval_rects();
  calc_invar();

  Begin_Par(bPar);

  return true;
}

/* ---------------------------------------------------------------------- */

void Contour::InsertElem(Elem_C_Cursor& insElc,const Elem& el)
{
  if (insElc.Container() != &el_list) return;

  double bPar = Begin_Par();

  Elem_Cursor elc(el_list);
  for (;elc;++elc) {
    if (elc == insElc) break;
  }

  if (el.Type() == Elem_Type_Circle) elc.To_End();

  elc.Insert(el);

  inval_rects();
  calc_invar();

  Begin_Par(bPar);

  insElc = elc;
}

/* ---------------------------------------------------------------------- */

void Contour::PrependElem(const Elem& el)
{
  double bPar = Begin_Par();

  Elem_Cursor elc(el_list);
  elc.To_Begin();

  elc.Insert(el);

  inval_rects();
  calc_invar();

  Begin_Par(bPar);
}

/* ---------------------------------------------------------------------- */

void Contour::AppendElem(const Elem& el)
{
  Elem_Cursor elc(el_list);
  elc.To_End();

  elc.Insert(Elem_Ref(el));

  inval_rects();
  calc_invar();
}

/* ---------------------------------------------------------------------- */

bool Contour::SplitElem(const Elem_C_Cursor& celc, int splits)
{
  if (splits < 1 || !celc) return false;

  Elem_Cursor elc(el_list);
  for (;elc;++elc) {
    if (elc == celc) break;
  }

  if (!elc) return false;

  double len = elc->El().Len();
  len /= (splits+1);

  Elem_List splLst;

  for (int i=0; i<splits; ++i) {
    Elem& el = elc->El();

    if (!el.Split(el.Begin_Par()+len,splLst)) return false;
    Elem_Cursor frElc(splLst);
    Elem_Cursor toElc(splLst); toElc.To_End();

    elc.Delete();
    elc.Re_Insert(frElc,toElc);
    ++elc;
  }

  inval_rects();
  calc_invar();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Resort()
{
  double bPar = Begin_Par();

  Elem::Sort(el_list);

  inval_rects();
  calc_invar();
  Begin_Par(bPar);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Contour::Begin_Point() const
{
  if (!el_list) return Vec3();
  else return el_list.Begin()->El().P1();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Contour::End_Point() const
{
  if (!el_list) return Vec3();
  else return el_list.Last()->El().P2();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Contour::Begin_Par() const
{
  Elem_C_Cursor elc(el_list);
  if (!elc) return 0.0;

  return elc->El().Begin_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Contour::End_Par() const
{
  Elem_C_Cursor elc(el_list);
  if (!elc) return 0.0;

  elc.To_Last();
  return elc->El().End_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Begin_Par(double par)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) {
    Elem &el = elc->El();

    el.Begin_Par(par); par += el.Par_Len();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::At_Par(double par, Vec3& p) const
{
  if (!el_rect_list) build_rect_list();
  if (!el_rect_list) return false;

  Elem_C_Cursor elc(el_list); elc.To_Last();
  if (!elc) return false;

  double epar = elc->El().End_Par() + Vec2::IdentDist;

  elc.To_Begin();
  double spar = elc->El().Begin_Par() - Vec2::IdentDist;
  
  if (par < spar || par > epar) return false;

  if (!el_rect_list->Find_Elem_At_Par(par,elc) || !elc) return false;

  return elc->El().At_Par(par,p);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Contour::Area_XY() const
{
  if (!inert.area_valid) inert.calc_area(*this);
  
  return inert.area;
}

/* ---------------------------------------------------------------------- */

bool Contour::Area_XY(const Elem_C_Cursor& bElc,
                          const Elem_C_Cursor& eElc, double& area) const
{
  area = 0.0;

  if (!bElc || !eElc || bElc.Container() != &el_list ||
                             eElc.Container() != &el_list) return false;

  Elem_C_Cursor elc = bElc;

  double org_y = elc->El().P1().y;

  for (;;) {
    const Elem& el = elc->El();

    // Elements assumed exactly connected !!!!
    area += el.Area_XY_P1();
    area += (org_y - el.P1().y) * (el.P2().x - el.P1().x);

    ++elc;
    if (!elc) elc.To_Begin();

    if (elc == eElc) return true;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

const Cont_Inert& Contour::Inert() const
{
  if (!inert.area_valid)  inert.calc_area(*this);
  if (!inert.inert_valid) inert.calc_inert(*this);
  
  return inert;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Set_Elem_Id(const Elem_C_Cursor& elc, int newId)
{
  if (!elc) return false;

  Elem_Cursor lElc(el_list);

  for (;lElc;++lElc) {
    if (lElc == elc) break;
  }

  if (!lElc) return false;

  Elem& el = lElc->El();

  if (el.Id() == newId) return false;

  el.Id(newId);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Set_Elem_Ids(int newid)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Id(newid);

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Find_First_Id_With(int id, Elem_Cursor& elc) const
{
  if (elc.Container() != &el_list) elc = el_list.Begin();
  else if (!elc) elc.To_Begin();

  for (;elc;++elc) {
    if (elc->El().Id() == id) return;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Sequence_Elem_Ids(int start_id)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Id(start_id++);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::End_With_High_Id()
{
  Elem_Cursor elc(el_list);

  if (!elc) return;

  inval_rects();

  double begpar = Begin_Par();

  Elem_Cursor helc(elc);
  int hid = elc->El().Id();

  ++elc;

  for (;elc;++elc) {
    Elem &el = elc->El();

    if (el.Id() >= hid) {
      hid = el.Id();
      helc = elc;
    }
  }

  ++helc; if (!helc) helc.To_Begin();

  helc.Become_First();

  Begin_Par(begpar);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Set_Elem_Cnt_Ids(int newid)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Cnt_Id(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Set_Elem_P_Cnt_Ids(int newid)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().P_Cnt_Id(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Increment_Elem_Cnt_Ids(int diffid)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) {
    Elem& el = elc->El();

    el.Cnt_Id(el.Cnt_Id() + diffid);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Increment_Elem_P_Cnt_Ids(int diffid)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) {
    Elem& el = elc->El();

    el.P_Cnt_Id(el.P_Cnt_Id() + diffid);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Find_First_Cnt_Id_With(int id, Elem_Cursor& elc) const
{
  if (elc.Container() != &el_list) elc = el_list.Begin();
  else if (!elc) elc.To_Begin();

  for (;elc;++elc) {
    if (elc->El().Cnt_Id() == id) return;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Intersect_With_XY(const Contour& cnt,
                                Cont_PPair_List& isct_lst,
                                bool cleanup) const
{
  Cont_Ref_List lst1(*this);
  Cont_Ref_List lst2(cnt);

  Cont2_Isect_List ilist(lst1,false,lst2,false,false,cleanup);

  ilist.Intersections_Into(isct_lst);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                    double& dist_xy) const
{
  cntp = Cont_Pnt();
  dist_xy = 0.0;

  if (!el_list) return false;

  if (!el_rect_list) build_rect_list();
  
  Elem_Cursor elc;
  Vec3 pp;
  double parm;

  if (!el_rect_list->Project_Pnt_XY(p,elc,pp,parm,dist_xy)) return false;

  Cont_Pnt cp(*this,elc,parm - elc->El().Begin_Par(),pp);
  cp.calc_point_attr();

  cntp = cp;

  // Determine proper sign of dist_xy

  dist_xy = fabs(dist_xy);

  if (dist_xy <= 10.0 * NumAccuracy * pp.len2()) {
    dist_xy = 0.0;
    return true;
  }

  if (cp.tg_bef_xy.oppositeTo2(cp.tg_aft_xy)) {
    if (cp.curve_bef < cp.curve_aft) dist_xy = -dist_xy;
  }
  else {
    Vec2 nrm_bef(cp.tg_bef_xy); nrm_bef.rot90();

    Vec2 dp(p); dp -= cp.pt;

    double inpr = cp.tg_bef_xy * cp.tg_aft_xy;

    if (nrm_bef * cp.tg_aft_xy < 0.0) {   // Turning to the right
      if (inpr >= -0.5) {
        Vec2 nrm_aft(cp.tg_aft_xy); nrm_aft.rot90();

        if ((nrm_bef * dp) < 0.0 && (nrm_aft * dp) < 0.0) dist_xy = -dist_xy;
      }
    }
    else {             // Straight or turning to the left
      if (inpr >= -0.5) {
        Vec2 nrm_aft(cp.tg_aft_xy); nrm_aft.rot90();

        if ((nrm_bef * dp) < 0.0 || (nrm_aft * dp) < 0.0) dist_xy = -dist_xy;
      }
      else dist_xy = -dist_xy;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void join_to_prv(Elem_Cursor& olc, double tol = 10.0 * Vec2::IdentDist)
{
  if (!olc) return;

  Elem& prvel = olc.Pred()->El();

  if (prvel.P2().distTo2(olc->El().P1()) > tol)
                                         Cont_Panic(Cont_Large_Join_Gap);

  prvel.Join_To_XY(olc->El(),false);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
static void connect_begin(Elem_Cursor& olc)
{
  if (!olc) return;

  Elem& prvel = olc.Pred()->El();

  if (olc->El().P1().distTo2(prvel.P2()) > 10.0 * Vec2::IdentDist)
                                       Cont_Panic(Cont_Large_Connect_Gap);

  olc->El().Stretch_Begin_XY(prvel.P2());
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void connect_up(Elem_C_Cursor& ilc, Elem_Cursor& olc,
                       const Vec3& lastpt, const Vec3& curpt,
                       double offdist,
                       bool& missing, double& missing_len,
                       bool& first_missing, double& first_missing_len)
{
  const Elem& prvel = ilc.Pred()->El();
  const Elem& curel = ilc->El();

  Vec2 tg_bef; prvel.End_Tangent_XY(tg_bef);
  Vec2 tg_aft; curel.Start_Tangent_XY(tg_aft);
  Vec2 nrm_bef(tg_bef); nrm_bef.rot90();

  bool insert_arc = false, stretch = false;

  double arclen_est = 0.0;

  if (tg_bef.oppositeTo2(tg_aft)) {
    if (offdist > 0.0)
         insert_arc = curel.Start_Curve() > -prvel.End_Curve()-Vec2::IdentDist;
    else insert_arc = curel.Start_Curve() < -prvel.End_Curve()+Vec2::IdentDist; 
  }
  else {
    arclen_est = nrm_bef * tg_aft * offdist;

    if (arclen_est < -5.0 * Vec2::IdentDist) insert_arc = true;
    else if (arclen_est < 5.0 * Vec2::IdentDist) stretch = true;
    
    if (stretch && tg_bef * tg_aft < 0.0) {
      stretch = false;
      
      if (arclen_est < 0.0) {
        if (fabs(Vec2::Pi*offdist) > 5.0*Vec2::IdentDist) insert_arc = true;
        else stretch = true;
      }
    }
  }

  if (insert_arc) {
    Elem_Arc newarc(lastpt,curpt,ilc->El().P1(),offdist < 0.0);
    newarc.Stretch_End_XY(curpt,true);
    newarc.Id(prvel.Id());
    newarc.Cnt_Id(prvel.Cnt_Id());
    newarc.P_Cnt_Id(prvel.Cnt_Id()); // Yes!
    newarc.Cam_Inf(prvel.Cam_Inf());
    newarc.setInsArc(true);

    olc.Insert(newarc);

    if (missing) {
      Elem_Cursor olc2(olc); olc2.To_Begin();
      if (olc2 != olc) {  // This new arc is not the first element
        double tol = 10.0 * Vec2::IdentDist;
        if (missing_len < tol) join_to_prv(olc,3.0*tol);
      }
      else {
        first_missing = missing;
        first_missing_len = missing_len;
      }
    }

    missing = false;
    missing_len = 0.0;
  }
  else if (stretch) {
    if (missing) {
      if (olc) olc->El().Stretch_Begin_XY(lastpt);
    }
    else {
      if (olc) join_to_prv(olc);
      else {
        Elem_Cursor olc2(olc); olc2.To_Last();
        if (olc2) olc2->El().Stretch_End_XY(curpt);
      }
    }
  }
  else missing_len += arclen_est;
}

/* ---------------------------------------------------------------------- */
/* -------- Calculate join point in proportion to element lengths ------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
static void join_point_relative(const Vec3& p1, double len1,
                                const Vec3& p2, double len2, Vec3& join_pt)
{
  double lensum = len1 + len2;

  if (lensum > 0.0) len1 /= lensum;
  else              len1 = 0.5;

  join_pt = p2; join_pt -= p1;

  join_pt *= len1;
  join_pt += p2;
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
static Isect_Lst last_isect;

static void process_adjacent(Elem_List& olst)
{
// Tijdelijk
  last_isect.Delete();

  double tol = 5.0 * Vec2::Ident_Dist;

  Elem_Cursor olc(olst);

//   Markup errmark(3.0,false,"kruis","","","",true);
//   Markup errmark2(3.0,false,"krs","","","",true);
 
  for (;olst.Length() > 1 && olc;++olc) {
    Elem& curel(olc->El());
    Elem& prvel(olc.Pred()->El());


    Isect_Lst isct_lst;
    prvel.Intersect_XY_Ext(curel,true,isct_lst);

    Isect_Cursor isc(isct_lst);

// Tijdelijk
    for (;isc;++isc) last_isect.Push_Back(*isc);
    isc.To_Begin();

//     int llpen = 3;
//     for (;isc;++isc) {
//       errmark.Set_Penlt(llpen++,1);
//       errmark.Draw_Cross(isc->P);
//     }
//     errmark.Reset();
//     
//     errmark2.Set_Penlt(2,1);
// 
//     isc.To_Begin();
    while (isc && (isc->First.Par <= prvel.Begin_Par() ||
                   isc->First.Par  > prvel.End_Par() + tol ||
                   isc->Last.Par  <  curel.Begin_Par() - tol ||
                   isc->Last.Par  >= curel.End_Par())) {
//       errmark2.Draw_Cross(isc->P);
      Isect_Pair& is(*isc);
      isc.Delete();
    }
    
//     errmark2.Reset();
    
    if (isc) {
      Isect_Pair& is(*isc);

      const double& fpar(isc->First.Par);
      const double& lpar(isc->Last.Par);

      bool del_prv = false, del_cur = false;

      if (fpar < prvel.Begin_Par()+tol) {
      	// Delete prvel

      	Elem_Cursor prvolc(olc);
      	--prvolc; if (!prvolc) prvolc.To_Last();
      	prvolc.Delete();

      	del_prv = true;
      }

      if (lpar > curel.End_Par()-tol) {
        // Delete curel

        del_cur = true;
      }

      if (olst.Length() < 2) return;

      if (del_prv && del_cur) { // Both too short, prv already deleted
      
        Elem_Line new_line(olc.Pred()->El().P2(),olc.Succ()->El().P1());

        new_line.Id(curel.Id());
        new_line.Cnt_Id(curel.Cnt_Id());
        new_line.P_Cnt_Id(curel.P_Cnt_Id());
        new_line.Cam_Inf(curel.Cam_Inf());
        
        if (curel.Info()) new_line.Add_Info(*curel.Info());


      	olc.Delete();

        olc.Insert(new_line);

        continue;
      }
      else if (del_prv) { // Prv to short, already deleted

        Vec3 join_pt;

        Elem& prvprvel = olc.Pred()->El();
        
        curel.Stretch_Begin_XY(isc->P,false);

        // End points are stretched in proportion to their lengths
        join_point_relative(prvprvel.P2(), prvprvel.Len_XY(),
                            isc->P, curel.Len_XY(), join_pt);
                            
        prvprvel.Stretch_End_XY(join_pt,false);
        curel.Stretch_Begin_XY(join_pt,false);

        continue;
      }
      else if (del_cur) { // Cur too short, not deleted yet

        olc.Delete();

        Elem_Cursor nolc(olc); 
        if (!nolc) nolc.To_Begin();

        Elem& newcurel(nolc->El());

        prvel.Stretch_End_XY(isc->P,false);
        
        Vec3 join_pt;
        join_point_relative(isc->P,prvel.Len_XY(),
                            newcurel.P1(),newcurel.Len_XY(),join_pt);

        prvel.Stretch_End_XY(join_pt,false);
        newcurel.Stretch_Begin_XY(join_pt,false);

        continue;
      }


      // We get here if the intersection point is on and not near the start
      // of prvel and not on and near the end of curel
      
      prvel.Stretch_End_XY(isc->P,false);
      curel.Stretch_Begin_XY(isc->P,false);

      continue;
    }


    // There was no valid intersection point. Now check to see if
    // maybe the elements are (almost) parallel,such that they
    // may be stretched together

    // Project the endpoints

    Vec3 prvpp, curpp;
    double curparm, curdist_xy;
    double prvparm, prvdist_xy;

    prvel.Project_Pnt_XY(curel.P1(),tol,false,prvpp,prvparm,prvdist_xy);
    if (prvparm < prvel.Begin_Par() ||
	prvparm > prvel.End_Par() + tol) continue;
    
    curel.Project_Pnt_XY(prvel.P2(),tol,false,curpp,curparm,curdist_xy);
    if (curparm < curel.Begin_Par() ||
	curparm > curel.End_Par() - tol) continue;

    if (fabs(prvdist_xy) > tol && fabs(curdist_xy) > tol) continue;
    
    if (prvparm < prvel.Begin_Par() + tol) {
      // Delete prvel and join to prvprvel

      Elem_Cursor prvolc(olc);
      --prvolc; if (!prvolc) prvolc.To_Last();
      prvolc.Delete();
      
      join_to_prv(olc,2.0*tol);
      
      continue;
    }

    if (curparm > curel.End_Par() - tol) {
      olc.Delete();

      Elem_Cursor nolc(olc); 
      if (!nolc) nolc.To_Begin();

      join_to_prv(nolc,2.0*tol);
      
      continue;
    }

    Vec3 join_pt;
    
    if (fabs(prvdist_xy) < fabs(curdist_xy)) {
      join_pt  = prvpp;
      join_pt += curel.P1();
    }
    else {
      join_pt  = curpp;
      join_pt += prvel.P2();
    }
    
    join_pt /= 2.0;
    
    prvel.Stretch_End_XY(join_pt,false);
    curel.Stretch_Begin_XY(join_pt,false);
  }
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool check_join(Elem_Cursor& olc, double minlen,
                                           double prvparm, double curparm)
{
  // if prvparm is near the start of the previous element (--olc)
  // delete the previous element (it would become too short)
  // else if curparm is near the end of the current element (olc)
  // delete it (it would become too short)

  if (!olc) return false;

  Elem& curel = olc->El();
  Elem& prvel = olc.Pred()->El();

  if (prvparm < prvel.Begin_Par() + minlen) {
    // Delete prvel and join to prvprvel

    Elem_Cursor prvolc(olc);
    --prvolc; if (!prvolc) prvolc.To_Last();
    prvolc.Delete();

    join_to_prv(olc,2.1*minlen);

    return true;
  }

  if (curparm > curel.End_Par() - minlen) {
    // Delete 
    olc.Delete();

    Elem_Cursor nolc(olc); 
    if (!nolc) nolc.To_Begin();

    join_to_prv(nolc,2.1*minlen);

    if (olc) --olc;
    
    return true;
  }
  
  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void process_adjacent(Elem_List& olst, bool closed)
{
  double ident_sq = sqr(Vec2::IdentDist);
  double minlen   = 3.0 * Vec2::IdentDist; // Minimum element length

  Elem_Cursor olc(olst);

  if (!closed) ++olc;

  for (;olst.Length() > 1 && olc;++olc) {
    Elem& curel = olc->El();
    Elem& prvel = olc.Pred()->El();


    if (prvel.P2().sqDistTo2(curel.P1()) <= ident_sq) continue;

    // Check to see if maybe the elements are (almost) parallel,
    // such that they may be stretched together

    // Project the endpoints

    Vec3 prvpp, curpp;
    double prvparm, curparm, prvdist_xy, curdist_xy;

    if (!prvel.Project_Pnt_Strict_XY(curel.P1(),0.0,prvpp,
                                              prvparm,prvdist_xy) ||
                              fabs(prvdist_xy) > Vec2::IdentDist) continue;
    
    if (!curel.Project_Pnt_Strict_XY(prvel.P2(),0.0,curpp,curparm,curdist_xy) ||
                              fabs(curdist_xy) > Vec2::IdentDist) continue;
                                 

    // Check halfway prvparm and curparm as well
    
    double dist_xy1;
    Vec3 pt1,pp1;

    double midparm1 = (curel.Begin_Par() + curparm) / 2.0;

    if (!curel.At_Par(midparm1,pt1)) continue;
    if (!prvel.Project_Pnt_Strict_XY(pt1,0.0,pp1,midparm1,dist_xy1) ||
                               fabs(dist_xy1) > Vec2::IdentDist) continue;

    double dist_xy2;
    Vec3 pt2,pp2;

    double midparm2 = (prvparm + prvel.End_Par()) / 2.0;

    if (!prvel.At_Par(midparm2,pt2)) continue;
    if (!curel.Project_Pnt_Strict_XY(pt2,0.0,pp2,midparm2,dist_xy2) ||
                               fabs(dist_xy2) > Vec2::IdentDist) continue;


    if (check_join(olc, minlen, prvparm, curparm)) continue;

    // Maybe they intersect:
    
    Isect_Lst isct_lst;
    prvel.Intersect_XY(curel,true,isct_lst);

    Isect_Cursor isc(isct_lst);

    // May be delete out-of-range intersection here????

    if (isc) {
      if (check_join(olc, minlen, isc->First.Par, isc->Last.Par)) continue;

      prvel.Stretch_End_XY(isc->P,false);
      curel.Stretch_Begin_XY(isc->P,false);
      
      continue;
    }

    Vec3 join_pt;
    
    if (fabs(prvdist_xy) < fabs(curdist_xy)) {
      join_pt  = prvpp;
      join_pt += curel.P1();
    }
    else {
      join_pt  = curpp;
      join_pt += prvel.P2();
    }
    
    join_pt /= 2.0;
    
    prvel.Stretch_End_XY(join_pt,false);
    curel.Stretch_Begin_XY(join_pt,false);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void connect_up_last(Elem_C_Cursor& ilc, Elem_Cursor& olc,
                            const Vec3& lastpt, const Vec3& curpt,
                            double offdist,
                            bool& missing, double& missing_len)
{
  const Elem& prvel = ilc.Pred()->El();
  const Elem& curel = ilc->El();

  Vec2 tg_bef; prvel.End_Tangent_XY(tg_bef);
  Vec2 tg_aft; curel.Start_Tangent_XY(tg_aft);
  Vec2 nrm_bef(tg_bef); nrm_bef.rot90();

  bool insert_arc = false, stretch = false;

  double arclen_est = 0.0;

  if (tg_bef.oppositeTo2(tg_aft)) {
    if (offdist > 0.0)
         insert_arc = curel.Start_Curve() > -prvel.End_Curve()-Vec2::IdentDist;
    else insert_arc = curel.Start_Curve() < -prvel.End_Curve()+Vec2::IdentDist; 
  }
  else {
    arclen_est = nrm_bef * tg_aft * offdist;

    if (arclen_est < -5.0 * Vec2::IdentDist) insert_arc = true;
    else if (arclen_est < 5.0 * Vec2::IdentDist) stretch = true;
    
    if (stretch && tg_bef * tg_aft < 0.0) {
      stretch = false;
      
      if (arclen_est < 0.0) {
        if (fabs(Vec2::Pi*offdist) > 5.0*Vec2::IdentDist) insert_arc = true;
        else stretch = true;
      }
    }
  }

  if (insert_arc) {
    Elem_Arc newarc(lastpt,curpt,ilc->El().P1(),offdist < 0.0);
    newarc.Stretch_End_XY(curpt,true);
    newarc.Id(prvel.Id());
    newarc.Cnt_Id(prvel.Cnt_Id());
    newarc.P_Cnt_Id(prvel.Cnt_Id()); // Yes!
    newarc.Cam_Inf(prvel.Cam_Inf());
    newarc.setInsArc(true);

    Elem_Cursor olc2(olc); olc2.To_End();
    olc2.Insert(newarc);

    int listlen = olc2.Container()->Length();

    if (missing) {
      if (listlen > 1) {  // This new arc is not the first element
        double tol = 10.0 * Vec2::IdentDist;
        if (missing_len < tol) join_to_prv(olc2,2.0*tol);
      }
    }

    missing = false;
    missing_len = 0.0;
  }
  else if (stretch) {
    if (missing) {
      if (olc) olc->El().Stretch_Begin_XY(lastpt);
    }
    else {
      if (olc) join_to_prv(olc);
      else {
        Elem_Cursor olc2(olc); olc2.To_Last();
        if (olc2) olc2->El().Stretch_End_XY(curpt);
      }
    }
  }
  else missing_len += arclen_est;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static Elem_List last_offset1, last_offset2;

static void offset_elems(const Elem_List& ilst, bool closed,
                                            double offdist, Elem_List& olst)
{
  double tol = 10.0 * Vec2::IdentDist;

  olst.Delete();
  Elem_Cursor olc(olst);

  Elem_C_Cursor ilc(ilst);

  if (ilst.Length() == 1) {
    Vec3 curpt, nextpt;
    double mislen;
    ilc->El().Offset_Into(offdist,tol/2.0,olst,curpt,nextpt,mislen);

    return;
  }

  Vec3 lastpt, first_curpt, curpt, nextpt;
  double missing_len = 0.0, first_missing_len = 0.0;

  Elem_List o_lst;
  ilc.Pred()->El().Offset_Into(offdist,tol/2.0,
                                          o_lst,curpt,lastpt,missing_len);

  bool missing = false, first_missing = false;

  missing_len = 0.0;

  bool first = true;

  for (;ilc;++ilc) {
    Elem_List off_lst;
    double mislen;
    bool have_elem = ilc->El().Offset_Into(offdist,tol/2.0,off_lst,
                                                    curpt,nextpt,mislen);

    olc.To_End();

    if (have_elem) {
      Elem_Cursor elc(off_lst);

      olc.Re_Insert(elc);
    }
    else {
      // Update curpt to nextpt for very short positive but missing
      // arcs. This avoids missing an intersection, because the
      // end of the element now last in the output list will be
      // stretched to curpt if need be

      if (mislen >= 0.0 && mislen < tol/2.0) {
        curpt = nextpt;
        mislen = 0.0;
      }
    }

    if (first) first_curpt = curpt;
    else       connect_up(ilc,olc,lastpt,curpt,offdist,missing,missing_len,
                                         first_missing,first_missing_len);
    first = false;

    if (have_elem) {
      if (missing) {
        if (olst.Length() > 1) { // Just appended an element
          if (missing_len < tol) join_to_prv(olc, 2.0*tol);
        }
        else {
          first_missing = missing;
          first_missing_len = missing_len;
        }
      }

      missing_len = 0.0;
    }
    else missing_len += fabs(mislen);

    missing = !have_elem;
    lastpt = nextpt;
  }

  ilc.To_Begin();
  if (first_missing) olc.To_End();
  else               olc.To_Begin();

  connect_up_last(ilc,olc,lastpt,first_curpt,offdist,missing,missing_len);

  if (missing || first_missing) {
     missing_len += first_missing_len;
     missing = true;
  }
  
  if (missing && missing_len < tol) {
    olc.To_Begin();
    if (olst.Length() > 1) join_to_prv(olc,2.0*tol);
  }


//Tijdelijk
   last_offset1 = olst;
   
//  draw_el_list(olst, "off1", 4);

   process_adjacent(olst,closed);

   last_offset2 = olst;

//  draw_el_list(olst, "off2", 5);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Offset_Into(double offdist, Cont_List& cnt_list) const
{
  cnt_list.contlst.Delete();
  cnt_list.calc_invar();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) {
    Cont_Cursor cc(cnt_list.contlst);

    cc.Insert(*this);

    cnt_list.calc_invar();
    
    return true;
  }

  Contour offcnt;

  offset_elems(el_list, Closed(), offdist, offcnt.el_list);

  offcnt.calc_invar();
  offcnt.is_closed = is_closed;

  offcnt.Begin_Par(Begin_Par());

  Cont_Ref offref(offcnt,false);

  Cont1_Isect_List isect(offref, Closed(), offdist > 0.0);

//  Cont_D_List offlist;
//  isect.Extract_Offset_Open(offlist);

  Cont_Clsd_D_List offlist;
  isect.Extract_Offset_Closed(*this,offdist,offlist);

  Cont_Cursor cc(cnt_list.contlst);
  Cont_Clsd_Cursor clc(offlist);

  for (;clc;++clc) {
    cc.To_End();
    cc.Insert(Contour());

    clc->Move_To(*cc);
    cc->calc_invar();
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Offset_Into(double offdist, Cont_List& cnt_list,
                                       double limAng, bool noArcs) const
{
  if (!Offset_Into(offdist,cnt_list)) return false;

  Cont_Cursor cc(cnt_list.contlst);

  for (;cc;++cc) cc->sharpen_Offset(limAng,noArcs);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void offsetElemsSingle(const Elem_List& ilst, bool closed,
                                            double offdist, Elem_List& olst)
{
  double tol = 10.0 * Vec2::IdentDist;

  olst.Delete();
  Elem_Cursor olc(olst);

  Elem_C_Cursor ilc(ilst);

  if (ilst.Length() == 1) {
    Vec3 curpt, nextpt;
    double mislen;
    ilc->El().Offset_Into(offdist,tol/2.0,olst,curpt,nextpt,mislen);

    return;
  }

  Vec3 lastpt, first_curpt, curpt, nextpt;
  double missing_len = 0.0, first_missing_len = 0.0;

  Elem_List o_lst;
  ilc.Pred()->El().Offset_Into(offdist,tol/2.0,
                                          o_lst,curpt,lastpt,missing_len);

  bool missing = false, first_missing = false;

  missing_len = 0.0;

  bool first = true;

  for (;ilc;++ilc) {
    Elem_List off_lst;
    double mislen;
    bool have_elem = ilc->El().Offset_Into(offdist,tol/2.0,off_lst,
                                                    curpt,nextpt,mislen);

    olc.To_End();

    if (have_elem) {
      Elem_Cursor elc(off_lst);

      olc.Re_Insert(elc);
    }
    else {
      // Update curpt to nextpt for very short positive but missing
      // arcs. This avoids missing an intersection, because the
      // end of the element now last in the output list will be
      // stretched to curpt if need be

      if (mislen >= 0.0 && mislen < tol/2.0) {
        curpt = nextpt;
        mislen = 0.0;
      }
    }

    if (first) first_curpt = curpt;
    else       connect_up(ilc,olc,lastpt,curpt,offdist,missing,missing_len,
                                         first_missing,first_missing_len);
    first = false;

    if (have_elem) {
      if (missing) {
        if (olst.Length() > 1) { // Just appended an element
          if (missing_len < tol) join_to_prv(olc, 2.0*tol);
        }
        else {
          first_missing = missing;
          first_missing_len = missing_len;
        }
      }

      missing_len = 0.0;
    }
    else missing_len += fabs(mislen);

    missing = !have_elem;
    lastpt = nextpt;
  }

  if (closed) {
    ilc.To_Begin();
    if (first_missing) olc.To_End();
    else               olc.To_Begin();

    connect_up_last(ilc,olc,lastpt,first_curpt,offdist,missing,missing_len);

    if (missing || first_missing) {
       missing_len += first_missing_len;
       missing = true;
    }
  
    if (missing && missing_len < tol) {
      olc.To_Begin();
      if (olst.Length() > 1) join_to_prv(olc,2.0*tol);
    }
  }

  process_adjacent(olst,closed);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool elemBeyond(const Vec3& p, const Elem& el, double offdist)
{
  const double tol = 100 * NumAccuracy;

  Vec3 pp;
  double parm, dist_xy;
  bool strict = false;

  if (el.Type() != Elem_Type_Line) strict = true;

  el.Project_Pnt_XY(p,tol,strict,pp,parm,dist_xy);

  return dist_xy * offdist < 0.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::cleanSingle(bool closed, double offset)
{
  if (el_list.Length() < 2) return;

  Elem_Cursor elc(el_list);

  for (;elc; ++elc) {
    Elem& curEl = elc->El();

    curEl.Begin_Par(0);
    curEl.End_Par(curEl.Len_XY());
  }

  bool deleted = false;

  do {
    deleted = false;

    elc.To_Begin();
    if (!closed) ++elc;

    for (;elc; ++elc) {
      Elem& prvEl = elc.Pred()->El();
      Elem& curEl = elc->El();

      if (prvEl.P2().distTo2(curEl.P1()) < 10 * NumAccuracy) continue;

      Isect_Lst isctLst;
      prvEl.Intersect_XY(curEl,false,isctLst);

      if (isctLst) {
        Isect_C_Cursor isc(isctLst);
        isc.To_Last();

        prvEl.End_Par(isc->First.Par);
        curEl.Begin_Par(isc->Last.Par);
      }
      else {
        if (elemBeyond(prvEl.P1(),curEl,offset)) { // discard
          prvEl.End_Par(prvEl.Begin_Par()-1.0);
        }

        if (elemBeyond(curEl.P2(),prvEl,offset)) { //discard
          curEl.End_Par(curEl.Begin_Par()-1.0);
        }
      }
    }

    elc.To_Begin();
    while (elc) {
      Elem& curEl = elc->El();

      if (curEl.End_Par() <= curEl.Begin_Par()) {
        elc.Delete();
        deleted = true;
      }
      else ++elc;
    }
  } while (deleted && el_list.Length() > 1);

  elc.To_Begin();
  if (!closed) ++elc;

  for (;elc; ++elc) {
    Elem& prvEl = elc.Pred()->El();
    Elem& curEl = elc->El();

    if (prvEl.P2().distTo2(curEl.P1()) < 100 * NumAccuracy) continue;

    Isect_Lst isctLst;
    prvEl.Intersect_XY(curEl,false,isctLst);

    if (isctLst) {
      Isect_C_Cursor isc(isctLst);
      isc.To_Last();

      const Vec3& ip = isc->P;

      prvEl.Stretch_End_XY(ip,false);
      curEl.Stretch_Begin_XY(ip,false);
    }
  }

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::OffsetSingle_Into(double offdist, Contour& cnt,
                                       double limAng, bool noArcs) const
{
  cnt.Delete();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) {
    cnt = *this;
    return true;
  }

  offsetElemsSingle(el_list,Closed(),offdist,cnt.el_list);
  cnt.sharpen_Offset(limAng,noArcs);

  if (cnt.el_list.Length() < 2) {
    cnt.calc_invar();

    return !cnt.Empty();
  }

  cnt.cleanSingle(Closed(),offdist);
  
  cnt.calc_invar();
  
  return !cnt.Empty();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Offset_Back(double offdist, Contour& bcnt) const
{
  bcnt.Delete();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) {
    bcnt = *this;
    
    return true;
  }

  offset_elems(el_list, Closed(), offdist, bcnt.el_list);

  bcnt.calc_invar();
  bcnt.is_closed = is_closed;

  bcnt.Begin_Par(Begin_Par());

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Split_At(Cont_Pnt& p)
{
  if (p.Parent_Contour() != this) return false;

  if (p.At_Joint()) {
    const Elem& el = p.elemc->El();
    if (el.Par_Len() - p.rel_par < Vec2::IdentDist) {
      p.rel_par -= el.Par_Len();
      ++p.elemc; if (!p.elemc) p.elemc.To_Begin();
    }

    return true;
  }
  else {
    Elem_List newelst;
    if (p.elemc->El().Split(p.Par(),newelst)) {

      inval_rects();

      Elem_Cursor bc(newelst);
      Elem_Cursor ec(newelst); ec.To_End();

      Elem_Cursor elc(el_list);
 
      for (;elc;++elc) {
        if (p.elemc == elc) break;
      }

      if (!elc) Cont_Panic(Cont_Cant_Find_Pnt_Elem);

      double splpar = p.Par();

      elc.Delete(); elc.Re_Insert(bc,ec);

      p.to_par(splpar);
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Split_At(Cont_Pnt& p, Contour& succ_cont)
{
  succ_cont.el_list.Delete();
  succ_cont.inval_rects(); succ_cont.calc_invar();
  succ_cont.inert.invalidate();
  succ_cont.intersecting_valid = false;

  if (!Split_At(p)) return false;

  inval_rects();
  inert.invalidate();
  intersecting_valid = false;

  Elem_Cursor selc(el_list);
  while (selc && selc != p.elemc) ++selc;
  Elem_Cursor delc(succ_cont.el_list);

  while (selc) {
    delc.To_End();
    delc.Re_Insert(selc);
  }

  calc_invar();
  succ_cont.calc_invar();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Contour::Start_At(Cont_Pnt& p)
{
  double begpar = Begin_Par();

  if (!Split_At(p)) return false;

  inval_rects();

  Elem_Cursor elc(el_list);

  while (elc && elc != p.Cursor()) ++elc;
  if (!elc) Cont_Panic(Cont_Cant_Find_Pnt_Elem);

  elc.Become_First();

  Begin_Par(begpar);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Reverse()
{
  inval_rects();

  if (inert.area_valid) inert.area = -inert.area;

  double beginpar = Begin_Par();

  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Reverse();

  el_list.Reverse();

  Begin_Par(beginpar);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Transform(const Trf2& trf)
{
  inval_rects();

  inert.invalidate();

  double beginpar = Begin_Par();

  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Transform(trf);

  calc_invar();

  Begin_Par(beginpar);
}

/* ---------------------------------------------------------------------- */
/* ------- Delete Element Info ------------------------------------------ */
/* ---------------------------------------------------------------------- */

void Contour::Del_Info()
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) elc->El().Del_Info();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Merge_Elems(bool limit_arcs)
{
  double bpar = Begin_Par();
  
  bool modified = false;

  Elem_Cursor elc(el_list);

  while (elc) {
    Elem_Cursor prvelc(elc);
    --prvelc; if (!prvelc) prvelc.To_Last();
    
    if (prvelc == elc) break;

    Elem *newel = NULL;

    if (elc->El().Type() == prvelc->El().Type()) {

      switch (elc->El().Type()) {
        case Elem_Type_Line:
          { Elem_Line& lin    = (Elem_Line&)elc->El();
            Elem_Line& prvlin = (Elem_Line&)prvelc->El();

            newel = prvlin.Merge_With(lin);
          }
        break;
        
        case Elem_Type_Arc:
          { Elem_Arc& arc    = (Elem_Arc&)elc->El();
            Elem_Arc& prvarc = (Elem_Arc&)prvelc->El();

            newel = prvarc.Merge_With(arc);
          }
        break;
      }
    }

    if (newel) {
      modified = true;
      
      prvelc.Delete();
      elc.Delete();
      
      elc.Insert(*newel);
      
      delete newel;
    }
    else ++elc;
  }

  // Now try to remove short elements
  double max_elem_len  = 500.0*Vec2::IdentDist;
  double max_elem_dist = 10.0*Vec2::IdentDist;
  
  if (Remove_Short_Elems(elc,max_elem_len,max_elem_dist)) modified = true;
  
  if (limit_arcs) {
    elc.To_Begin();

    while (elc) {
      if (elc->El().Type() == Elem_Type_Arc) {

        Elem_Arc& elarc = (Elem_Arc &)(elc->El());

        if (elarc.Limit_Span_180(elc)) {
          ++elc; ++elc; elc.Delete();

          modified = true;
        }
        else ++elc;
      }
      else if (elc->El().Type() == Elem_Type_Circle) {

        Elem_Circle& elcir = (Elem_Circle &)(elc->El());

        elcir.Limit_Span_180(elc);
        ++elc; ++elc; elc.Delete();

        modified = true;
      }
      else ++elc;
    }
  }

  if (modified) {
    inval_rects();
    
    calc_invar();

    Begin_Par(bpar);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Set_Elem_Z(double newz)
{
  Elem_Cursor elc(el_list);

  for (;elc;++elc) {
    Elem& el = elc->El();
    el.Z1(newz);
    el.Z2(newz);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Contour::Start_Outside(const Rect_Ax& rct)
{
  if (Empty()) return;

  Vec2 ll = rct.Ll();
  Vec2 ul = ll; ul.y = rct.Ur().y;

  Vec2 ur = rct.Ur();
  Vec2 lr = ur; lr.y = rct.Ll().y;

  Cont_Pnt minpnt; double mindist;

  Project_Pnt_XY(ll,minpnt,mindist); mindist = fabs(mindist);

  Cont_Pnt pnt; double dist;

  Project_Pnt_XY(ul,pnt,dist); dist = fabs(dist);
  if (dist < mindist) {
    mindist = dist;
    minpnt  = pnt;
  }

  Project_Pnt_XY(ur,pnt,dist); dist = fabs(dist);
  if (dist < mindist) {
    mindist = dist;
    minpnt  = pnt;
  }

  Project_Pnt_XY(lr,pnt,dist); dist = fabs(dist);
  if (dist < mindist) {
    mindist = dist;
    minpnt  = pnt;
  }

  double stpar = Begin_Par();
  inval_rects();

  Elem_Cursor elc(this->el_list);
  while (elc != minpnt.Cursor()) ++elc;

  if (elc) elc.Become_First();

  Begin_Par(stpar);
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Contour::Min_Left_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Elem_C_Cursor elc(el_list);
  
  bool found = false;
  
  for (;elc;++elc) {
    const Elem& el = (const Elem&) elc->El();
    
    if (el.Type() == Elem_Type_Arc) {
      const Elem_Arc& elarc = (const Elem_Arc&) el;

      if (!elarc.Ccw()) continue;
      
      if (!found || elarc.R() < min_rad) min_rad = elarc.R();
      found = true;
    }
  }
  
  return found;
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Contour::Min_Right_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Elem_C_Cursor elc(el_list);
  
  bool found = false;
  
  for (;elc;++elc) {
    const Elem& el = (const Elem&) elc->El();
    
    if (el.Type() == Elem_Type_Arc) {
      const Elem_Arc& elarc = (const Elem_Arc&) el;

      if (elarc.Ccw()) continue;
      
      if (!found || elarc.R() < min_rad) min_rad = elarc.R();
      found = true;
    }
  }
  
  return found;
}
 
//---------------------------------------------------------------------------
// Contour Persistence Section

static const char fldElemLst[] = "elemLst";

//---------------------------------------------------------------------------

void Contour::definePersistentFields(PersistentWriter& po) const
{
  po.addObjectArrayField(fldElemLst,(Elem **)NULL);
}

//---------------------------------------------------------------------------

Contour::Contour(PersistentReader& pi)
: Rect_Ax(), len_xy(0.0), len(0.0),
  el_list(), el_rect_list(NULL),
  intersecting_valid(false), intersecting(false),
  is_closed(false), mark(false), inert(), parent(NULL),
  persistLstLen(pi.readArraySize(fldElemLst,0)),
  persistLst((Elem **)pi.readObjArray(fldElemLst,NULL))
{
  pi.callPostProcess();
}

//---------------------------------------------------------------------------


void Contour::writePersistentObject(PersistentWriter& po) const
{
  persistLstLen = el_list.Length();
  persistLst = new Elem*[persistLstLen];

  Elem_Cursor elc(el_list);
  int idx = 0;

  while (elc) {
    persistLst[idx++] = &elc->El();
    ++elc;
  }

  po.writeArray(fldElemLst,persistLst,persistLstLen);

  po.callPostProcess();
}

//---------------------------------------------------------------------------

void Contour::postProcess(PersistentReader& /*pi*/)
{
  if (!persistLst) return;

  el_list.Delete();
  Elem_Cursor elc(el_list);

  for (int i=0; i<persistLstLen; ++i) {
    elc.To_End();
    elc.Insert(Elem_Ref());
    elc->setElem(persistLst[i]);
  }

  if (persistLst) delete[] persistLst;
  persistLst = NULL;
  persistLstLen = 0;

  calc_invar();
}

//---------------------------------------------------------------------------

void Contour::postProcess(PersistentWriter& /*po*/) const
{
  if (persistLst) delete[] persistLst;

  persistLst    = NULL;
  persistLstLen = 0;
}

/* ---------------------------------------------------------------------- */
/* ------- Reverse Sort on Area ----------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Area_Cmp::Compare(const Contour& cnt1, const Contour& cnt2)
{
  if (cnt1.Rect().Area_XY() < cnt2.Rect().Area_XY()) return 1;
  else return -1;
}

/* ---------------------------------------------------------------------- */
/* ------- Closed Contours ---------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ------- Find sense of closed contour --------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::calc_invar()
{
  lccw = false;

  Elem_C_Cursor elc(cont.el_list);
  if (!elc) return;

  const Elem *prvel = &(elc.Pred()->El());

  double angle = 0.0;
  
  for (;elc;++elc) {
    const Elem *curel = &(elc->El());

    Vec2 end_tg, st_tg;
    prvel->End_Tangent_XY(end_tg);
    curel->Start_Tangent_XY(st_tg);

    if (end_tg.oppositeTo2(st_tg,10.0*Vec2::IdentDir)) {
//      if (prvel->End_Curve() < curel->Start_Curve()) angle += Vec2::Pi;
//      else angle -= Vec2::Pi;

      lccw = cont.Area_XY() >= 0.0;
      return;

    }
    else angle += end_tg.angleTo2(st_tg);

    angle += curel->Span_Angle();

    prvel = curel;
  }

  // As an extra safety
  if (fabs(fabs(angle) - Vec2::Pi2) > 0.01 * Vec2::Pi2)
       lccw = cont.Area_XY() >= 0.0;
  else lccw = angle > 0.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd::Cont_Clsd()
  : cont(), lccw(false)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd::Cont_Clsd(const Cont_Clsd& cp)
  : cont(cp.cont), lccw(cp.lccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd::Cont_Clsd(const Vec2& cntr, double rad, bool ccw)
  : cont(cntr,rad,ccw), lccw(ccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd::Cont_Clsd(const Rect_Ax& rct)
  : cont(rct), lccw(true)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd& Cont_Clsd::operator=(const Cont_Clsd& src)
{
  cont = src.cont;
  lccw = src.lccw;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Move_To(Cont_Clsd& dst)
{
  cont.Move_To(dst.cont);
  dst.lccw = lccw;

  lccw = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Move_To(Contour& dst)
{
  cont.Move_To(dst);

  lccw = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Move_To(Elem_List& dst)
{
  cont.Move_To(dst);

  lccw = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Move_To(Cont_Nest& dst)
{
  dst.contlst.Delete();

  if (!Empty()) {
    Cont_Clsd_Cursor cc(dst.contlst);

    cc.Insert(Cont_Clsd());

    Move_To(*cc);
  }

  dst.calc_invar();

  lccw = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Move_To(Cont_Area& dst)
{
  dst.nestlst.Delete();

  if (!Empty()) {
    Cont_Nest_Cursor nsc(dst.nestlst);

    nsc.Insert(Cont_Nest());

    Move_To(*nsc);
  }

  dst.calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Delete()
{
  cont.Delete();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Cont_Clsd::Begin_Point() const
{
  if (!cont.el_list) return Vec3();
  else return cont.el_list.Begin()->El().P1();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Cont_Clsd::End_Point() const
{
  if (!cont.el_list) return Vec3();
  else return cont.el_list.Last()->El().P2();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Reverse()
{
  cont.Reverse();
  lccw = !lccw;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Clsd::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                    double& dist_xy) const
{
  return cont.Project_Pnt_XY(p,cntp,dist_xy);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Clsd::Offset_Into(double offdist, Cont_Area& ar) const
{
  ar.nestlst.Delete();
  ar.calc_invar();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) { // Just copy
    Cont_Nest_Cursor nstc(ar.nestlst);
    nstc.Insert(Cont_Nest());

    Cont_Clsd_Cursor cc(nstc->contlst);

    cc.Insert(*this);

    nstc->calc_invar();
    ar.calc_invar();

    return true;
  }

  Contour offcnt;

  offset_elems(cont.el_list, true, offdist, offcnt.el_list);

  offcnt.calc_invar();
  offcnt.is_closed = true;

  offcnt.Begin_Par(Begin_Par());

  Cont_Ref offref(offcnt,lccw);

  Cont1_Isect_List isect(offref, true, offdist > 0.0);


  // For now: As an extra security measure:
  // Check the locations of the intersection points
  
  isect.Check_Against_Cont(*this,offdist);


  Cont_Clsd_D_List offlist;
  isect.Extract_Offset_Closed(*this,offdist,offlist);

  Cont_Nest_Cursor nstc(ar.nestlst);

  if ((offdist > 0.0) == lccw) {  // Offset inward

    Cont_Clsd_Cursor cc(offlist);

    while (cc) {
      nstc.Insert(Cont_Nest());

      Cont_Clsd_Cursor ncc(nstc->contlst);
      ncc.Re_Insert(cc);

      nstc->calc_invar();

      nstc.To_End();
    }
  }
  else { // Offset outward

    if (!offlist) Cont_Panic(Cont_Nest_Cant_Offset);


    Cont_Clsd_Cursor fstc(offlist);
    Cont_Clsd_Cursor cc(offlist);

    if (fstc->lccw != lccw) {
      for (;cc;++cc) {
        if (cc->lccw == lccw) {
          fstc.Re_Insert(cc);
          break;
        }
      }
    }

    // Move entire list to nest:

    nstc.Insert(Cont_Nest());

    offlist.Move_To(nstc->contlst);

    nstc->calc_invar();
  }

  ar.calc_invar();

  ar.Begin_Par(Begin_Par());

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Clsd::Offset_Back(double offdist, Contour& bcnt) const
{
  return cont.Offset_Back(offdist,bcnt);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Sequence_Elem_Ids(int start_id)
{
  cont.Sequence_Elem_Ids(start_id);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::End_With_High_Id()
{
  cont.End_With_High_Id();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Intersect_With_XY(const Contour& cnt, 
                                        Cont_PPair_List& isct_lst) const
{
  cont.Intersect_With_XY(cnt,isct_lst);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Clsd::Split_At(Cont_Pnt& p)
{
  return cont.Split_At(p);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Clsd::Start_At(Cont_Pnt& p)
{
  return cont.Start_At(p);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Transform(const Trf2& trf)
{
  cont.Transform(trf);

  if (trf.mirrors()) lccw = !lccw;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Del_Info()
{
  cont.Del_Info();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Clsd::Merge_Elems(bool limit_arcs)
{
  cont.Merge_Elems(limit_arcs);
}

/* ---------------------------------------------------------------------- */
/* ------- Contour Lists ------------------------------------------------ */
/* ---------------------------------------------------------------------- */

void Cont_List::calc_invar()
{
  Cont_Cursor cc(contlst);

  Rect_Ax::operator=(Rect_Ax());

  bool first = true;

  while (cc) {
    if (cc->Empty()) cc.Delete();
    else {
      if (first) Rect_Ax::operator =(cc->Rect());
      else       Rect_Ax::operator+=(cc->Rect());

      ++cc;
      first = false;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_List::Cont_List()
 : Rect_Ax(), contlst()
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_List::Cont_List(const Cont_List& cp)
 : Rect_Ax(cp), contlst(cp.contlst)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_List& Cont_List::operator=(const Cont_List& src)
{
  Rect_Ax::operator=(src);

  contlst = src.contlst;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_List& Cont_List::operator=(const Cont_Nest& src)
{
  contlst.Delete();
  
  Cont_Clsd_C_Cursor clsdc(src);

  for (;clsdc;++clsdc) contlst.Push_Back(*clsdc);

  calc_invar();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_List& Cont_List::operator=(const Cont_Area& src)
{
  contlst.Delete();
  
  Cont_Nest_C_Cursor nestc(src);

  for (;nestc;++nestc) {
    Cont_Clsd_C_Cursor clsdc(*nestc);

    for (;clsdc;++clsdc) contlst.Push_Back(*clsdc);
  }

  calc_invar();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Move_To(Cont_List& dst)
{
  dst.Rect_Ax::operator=(*this);

  contlst.Move_To(dst.contlst);

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Delete()
{
  contlst.Delete();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Begin_Par(double par)
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) {
    cc->Begin_Par(par);

    double epar = cc->End_Par();

    par = 2.0 * epar - par;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_List::Elem_Count() const
{
  int count = 0;
  
  Cont_C_Cursor cc(contlst);
  
  for (;cc;++cc) count += cc->Elem_Count();

  return count;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_List::Begin_Par() const
{
  if (!contlst) return 0.0;
  else return contlst.Begin()->Begin_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_List::End_Par()   const
{
  if (!contlst) return 0.0;
  else return contlst.Last()->End_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Parent(void *newparent)
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) cc->Parent(newparent);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Set_Elem_Ids(int newid)
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Sequence_Elem_Ids(int start_id)
{
  Cont_Cursor cc(contlst);

  int bid = start_id;

  for (;cc;++cc) {
    cc->Sequence_Elem_Ids(bid);

    bid += cc->el_list.Length();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Set_Elem_Cnt_Ids(int newid)
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_Cnt_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_List::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                  double& dist_xy) const
{
  cntp = Cont_Pnt();

  Cont_C_Cursor cc(contlst);

  if (!cc) return false;

  Cont_C_Cursor mincc(cc);
  double mindist = cc->Rect().Dist_To_XY(p);

  while (++cc && mindist > 0.0) {
    double curdist = cc->Rect().Dist_To_XY(p);

    if (curdist < mindist) {
      mindist = curdist;
      mincc   = cc;
    }
  }

  cc = mincc;

  if (!cc->Project_Pnt_XY(p,cntp,dist_xy)) Cont_Panic(Cont_List_Cant_Project);

  ++cc; if (!cc) cc.To_Begin();

  while (cc != mincc) {
    if (cc->Rect().Dist_To_XY(p) < fabs(dist_xy) + Vec2::IdentDist) {
      Cont_Pnt curpnt;
      double curdist;

      if (!cc->Project_Pnt_XY(p,curpnt,curdist))
                                           Cont_Panic(Cont_List_Cant_Project);

      if (fabs(curdist) < fabs(dist_xy)) {
        dist_xy = curdist;
        cntp = curpnt;
      }
    }

    ++cc;
    if (!cc) cc.To_Begin();
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Reverse()
{
  Cont_Cursor cc(contlst);

  if (!cc) return;

  double begpar = Begin_Par();

  for (;cc;++cc) cc->Reverse();

  contlst.Reverse();

  Begin_Par(begpar);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Transform(const Trf2& trf)
{
  Cont_Cursor cc(contlst);
  
  for (;cc;++cc) cc->Transform(trf);

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Del_Info()
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) cc->Del_Info();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_List::Merge_Elems(bool limit_arcs)
{
  Cont_Cursor cc(contlst);

  for (;cc;++cc) cc->Merge_Elems(limit_arcs);
}

/* ---------------------------------------------------------------------- */
/* ------- Nested Contours ---------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::calc_invar()
{
  Rect_Ax::operator=(Rect_Ax());
  lccw = false;

  Cont_Clsd_Cursor clsc(contlst);

  while (clsc)  {
    if (clsc->Empty()) clsc.Delete();
    else ++clsc;
  }

  clsc.To_Begin();

  if (clsc) {
    Rect_Ax::operator=(clsc->Rect());
    lccw = clsc->Ccw();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::uni_source() const
{
  Cont_Clsd_C_Cursor cc(contlst);
  if (!cc) return true;

  Elem_C_Cursor elc1(cc->List());
  if (!elc1) return true;

  int el_id = elc1->El().Id();

  for (;cc;++cc) {
    Elem_C_Cursor elc(cc->List());

    for (;elc;++elc) {
      if (elc->El().Id() != el_id) return false;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::uni_source(int el_id) const
{
  Cont_Clsd_C_Cursor cc(contlst);
  if (!cc) return true;

  for (;cc;++cc) {
    Elem_C_Cursor elc(cc->List());

    for (;elc;++elc) {
      if (elc->El().Id() != el_id) return false;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest::Cont_Nest()
  : Rect_Ax(), contlst(), lccw(false), inert()
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest::Cont_Nest(const Cont_Clsd& cl_cont)
 : Rect_Ax(cl_cont.cont.Rect()), contlst(), lccw(cl_cont.lccw),
   inert()
{
  contlst.Push_Back(cl_cont);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest::Cont_Nest(const Cont_Nest& cp)
  : Rect_Ax(cp), contlst(cp.contlst), lccw(cp.lccw),
    inert(cp.inert)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest& Cont_Nest::operator=(const Cont_Nest& src)
{
  Rect_Ax::operator=(src);

  contlst    = src.contlst;
  lccw       = src.lccw;
  inert      = src.inert;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest& Cont_Nest::operator=(const Cont_Clsd& src)
{
  contlst.Delete();

  if (!src.Empty()) {
    Cont_Clsd_Cursor cc(contlst);
    if (cc) cc.Insert(src);
  }

  calc_invar();

  inert.invalidate();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Move_To(Cont_Clsd_D_List& dst)
{
  dst.Delete();

  Cont_Clsd_Cursor clsdc(contlst);
  Cont_Clsd_Cursor cntc(dst);

  for (;clsdc;++clsdc) {
    cntc.To_End(); cntc.Insert(Cont_Clsd());

    clsdc->Move_To(*cntc);
  }

  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Move_To(Cont_List& dst)
{
  dst.contlst.Delete();
  dst.calc_invar();

  Cont_Clsd_Cursor clsdc(contlst);
  Cont_Cursor      cntc(dst.contlst);

  for (;clsdc;++clsdc) {
    cntc.To_End(); cntc.Insert(Cont_Clsd());

    clsdc->cont.Move_To(*cntc);
  }

  dst.calc_invar();
  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Move_To(Cont_Nest& dst)
{
  dst.Rect_Ax::operator=(*this);

  contlst.Move_To(dst.contlst);
 
  dst.lccw = lccw;
  dst.inert = inert;

  inert.invalidate();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Move_To(Cont_Area& dst)
{
  dst.nestlst.Delete();

  if (contlst) {
    Cont_Nest_Cursor nsc(dst.nestlst);

    nsc.Insert(Cont_Nest());

    Move_To(*nsc);
  }

  calc_invar();
  
  dst.inert.invalidate();
  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Delete()
{
  contlst.Delete();

  calc_invar();
  
  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Nest::Elem_Count() const
{
  int count = 0;
  
  Cont_Clsd_C_Cursor cc(contlst);
  
  for (;cc;++cc) count += cc->Elem_Count();

  return count;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Begin_Par(double par)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) {
    cc->Begin_Par(par);

    double epar = cc->End_Par();

    par = 2.0* epar - par;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Nest::Begin_Par() const
{
  if (!contlst) return 0.0;

  return contlst.Begin()->Begin_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Nest::End_Par() const
{
  if (!contlst) return 0.0;

  return contlst.Last()->End_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Set_Elem_Ids(int newid)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Sequence_Elem_Ids(int start_id)
{
  Cont_Clsd_Cursor cc(contlst);

  int bid = start_id;

  for (;cc;++cc) {
     cc->Sequence_Elem_Ids(bid);

     bid += cc->cont.List().Length();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Set_Elem_Cnt_Ids(int newid)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_Cnt_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Set_Elem_P_Cnt_Ids(int newid)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_P_Cnt_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Sequence_Elem_Cnt_Ids(int& start_id)
{
  Cont_Clsd_Cursor cc(contlst);

  if (start_id < 0) {
    for (;cc;++cc) cc->Set_Elem_Cnt_Ids(start_id--);
  }
  else {
    for (;cc;++cc) cc->Set_Elem_Cnt_Ids(start_id++);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Nest::Area_XY() const
{
  if (inert.area_valid) inert.calc_area(*this);

  return inert.area;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

const Cont_Inert& Cont_Nest::Inert() const
{
  if (!inert.area_valid)  inert.calc_area(*this);
  if (!inert.inert_valid) inert.calc_inert(*this);
  
  return inert;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                 double& dist_xy) const
{
  cntp = Cont_Pnt();

  Cont_Clsd_C_Cursor cc(contlst);

  if (!cc) return false;

  if (!cc->Project_Pnt_XY(p,cntp,dist_xy)) Cont_Panic(Cont_Nest_Cant_Project);

  ++cc;

  if ((dist_xy < 0.0) == lccw || !cc) return true;

  Cont_Clsd_C_Cursor mincc(cc);
  double mindist = cc->Rect().Dist_To_XY(p);

  while (cc && mindist > 0.0) {
    double curdist = cc->Rect().Dist_To_XY(p);

    if (curdist < mindist) {
      mindist = curdist;
      mincc   = cc;
    }

    ++cc;
  }

  cc = mincc;

  do {
    if (cc->Rect().Dist_To_XY(p) < fabs(dist_xy) + Vec2::IdentDist) {
      Cont_Pnt curpnt;
      double curdist;

      if (!cc->Project_Pnt_XY(p,curpnt,curdist))
                                       Cont_Panic(Cont_Nest_Cant_Project);

      if ((curdist < 0.0) == lccw) {
        dist_xy = curdist;
        cntp = curpnt;
        return true;
      }

      if (fabs(curdist) < fabs(dist_xy)) {
        dist_xy = curdist;
        cntp = curpnt;
      }
    }

    ++cc; if (!cc) { cc.To_Begin(); ++cc; }
  } while (cc != mincc);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::Offset_Into(double offdist, Cont_Area& ar_list) const
{
  ar_list.nestlst.Delete();
  ar_list.inert.invalidate();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) {  // Just copy
    Cont_Nest_Cursor nsc(ar_list.nestlst);

    nsc.Insert(*this);

    ar_list.calc_invar();

    return true;
  }

  double begpar = Begin_Par();

  // Offset outer
  Cont_Clsd_C_Cursor cc(contlst);

  if (!cc->Offset_Into(offdist,ar_list)) return false;

  // Offset inner contours

  if ((offdist > 0.0) == lccw) {   // Outer offset inward

    if (!ar_list.Empty()) {
      while (++cc) {
        Cont_Area ar2;

        if (!cc->Offset_Into(offdist,ar2)) Cont_Panic(Cont_Nest_Cant_Offset);

        Cont_Area arhlp;
        ar_list.Combine_With(false,ar2,false,offdist > 0.0,arhlp);

        arhlp.Move_To(ar_list);
      }
    }
  }
  else {                 // Outer offset outward
    if (ar_list.Empty()) {
      Cont_Panic(Cont_Nest_Cant_Offset);
    }

    Cont_Nest_Cursor nsc(ar_list.nestlst);

    while (++cc) {
      Cont_Area ar2;
      if (!cc->Offset_Into(offdist,ar2)) Cont_Panic(Cont_Nest_Cant_Offset);

      Cont_Nest_Cursor nsc2(ar2.nestlst);

      for (;nsc2;++nsc2) nsc2->contlst.Append_To(nsc->contlst);

      nsc->calc_invar();
    }
  }

  ar_list.calc_invar();

  ar_list.Begin_Par(begpar);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Reverse()
{
  Cont_Clsd_Cursor cc(contlst);

  if (cc) {
    double begpar = Begin_Par();

    for (;cc;++cc) cc->Reverse();

    Begin_Par(begpar);
  }

  lccw = !lccw;

  if (inert.area_valid) inert.area = -inert.area;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Del_Info()
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Del_Info();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Merge_Elems(bool limit_arcs)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Merge_Elems(limit_arcs);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Nest::Set_Elem_Z(double new_z)
{
  Cont_Clsd_Cursor cc(contlst);

  for (;cc;++cc) cc->Set_Elem_Z(new_z);
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::Min_Left_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Cont_Clsd_C_Cursor cc(contlst);
  
  bool found = false;
  
  for (;cc;++cc) {
    double crad;
    
    if (cc->Min_Left_Rad(crad)) {
      if (!found || crad < min_rad) min_rad = crad;
      found = true;
    }
  }
  
  return found;
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Cont_Nest::Min_Right_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Cont_Clsd_C_Cursor cc(contlst);
  
  bool found = false;
  
  for (;cc;++cc) {
    double crad;
    
    if (cc->Min_Right_Rad(crad)) {
      if (!found || crad < min_rad) min_rad = crad;
      found = true;
    }
  }
  
  return found;
}
  
/* ---------------------------------------------------------------------- */
/* ------- Contour Areas ------------------------------------------------ */
/* ---------------------------------------------------------------------- */

void Cont_Area::calc_invar()
{
  Rect_Ax::operator=(Rect_Ax());
  lccw = false;

  Cont_Nest_Cursor nsc(nestlst);

  if (!nsc) return;

  while (nsc) {
    if (nsc->Empty()) nsc.Delete();
    else ++nsc;
  }

  nsc.To_Begin(); if (!nsc) return;

  lccw = nsc->lccw;

  Rect_Ax::operator=(nsc->Rect());

  while (++nsc) Rect_Ax::operator+=(nsc->Rect());
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area::Cont_Area(const Cont_Clsd& cl_cont)
 : Rect_Ax(cl_cont.cont.Rect()), nestlst(), lccw(cl_cont.lccw), z(0.0),
   inert()
{
  nestlst.Push_Back(Cont_Nest(cl_cont));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area::Cont_Area(const Cont_Nest& nest)
 : Rect_Ax(nest), nestlst(), lccw(nest.lccw), z(0.0), inert()
{
  nestlst.Push_Back(nest);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area::Cont_Area(const Cont_Area& cp)
 : Rect_Ax(cp), nestlst(cp.nestlst), lccw(cp.lccw), z(cp.z),
   inert(cp.inert)
{
}

/* ---------------------------------------------------------------------- */
/* ------- Load up elements and sort them ------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area::Cont_Area(Elem_List& src, Cont_List& lst,
                                          Cont_PPair_List& intersections)
 : Rect_Ax(), nestlst(), lccw(true), z(0.0), inert()
{
  nestlst.Delete(); calc_invar();
  lst.contlst.Delete(); lst.calc_invar();
  intersections.Delete();

  Cont_Cursor cc(lst.contlst);
  
  Elem_Sort elsrt(src, 0.001);

  for (;;) {
    cc.To_End(); cc.Insert(Contour());
    
    if (elsrt.Chain(cc->el_list))
       cc->calc_invar();
    else {
       cc.Delete();
       break;
    }
  }

  Cont_Clsd_D_List clist;
  Cont_Clsd_Cursor clc(clist);

  cc.To_Begin();
  while (cc) {
    if (cc->Closed() && !cc->Self_Intersecting()) {
      clc.Insert(Cont_Clsd());

      cc->Move_To(clc->cont); clc->calc_invar();

      cc.Delete();
    }
    else ++cc;
  }

  if (clc) {
    clc.To_Begin(); 
    Cont_Clsd_Cursor clcend(clist); clcend.To_End();

    IT_Sort<Cont_Clsd_Cursor,Cont_Area_Cmp>::
          Sort(clc,clcend,Cont_Area_Cmp());

    clc.To_Begin(); if (!clc->Ccw()) clc->Reverse();

    Cont_Nest_Cursor nsc(nestlst); nsc.Insert(Cont_Nest());

    nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();

    calc_invar();

    while (clc) {
      Cont_Ref_List arref(*this); Cont_Ref_List cntref(*clc);
  
      Cont2_Isect_List isctlst(arref,false,cntref,false,true);
  
      if (!isctlst.Empty()) {
        Cont_PPair_List pplst;
        isctlst.Intersections_Into(pplst); pplst.Append_To(intersections);
        ++clc;
      }
      else {
        nsc.To_Begin();
        for (;nsc;++nsc) {
          const Contour *colinear = NULL;
  
          bool inside = Cnt_Inside(*clc,*nsc,colinear);
          if (colinear) {
            ++clc;
            break;
          }
   
          if (inside) {
            // Remove wrong contours from nest
            Cont_Clsd_Cursor nclc(nsc->contlst);

            ++nclc;
            while (nclc) {
              if (Cnt_Inside(*nclc,*clc,colinear)) {
                clist.End().Re_Insert(nclc);
              }
              else ++nclc;
            }
            
            if (clc->Ccw()) clc->Reverse();
            nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();
            break;
          }
        }
  
        if (!nsc) {
          nsc.Insert(Cont_Nest());
          if (!clc->Ccw()) clc->Reverse();
          nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();
        }
      }
  
      calc_invar();
    }
  }

  // Push back unused closed contours

  clc.To_Begin();

  while (clc) {
    cc.To_End(); cc.Insert(Contour());
    clc->cont.Move_To(*cc);
    clc.Delete();
  }

  lst.calc_invar();

  // Now the area should be checked for fold backs
  // The removed parts to be returned to list src

  Merge_Elems();
}

/* ---------------------------------------------------------------------- */
/* ------- Load up elements and sort them ------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area::Cont_Area(Elem_List& src, Cont_List& lst,
                                Cont_PPair_List& intersections, double tol)
 : Rect_Ax(), nestlst(), lccw(true), z(0.0), inert()
{
  tol = fabs(tol);

  nestlst.Delete(); calc_invar();
  lst.contlst.Delete(); lst.calc_invar();
  intersections.Delete();

  Cont_Cursor cc(lst.contlst);
  
  Elem_Sort elsrt(src, tol);

  for (;;) {
    cc.To_End(); cc.Insert(Contour());
    
    if (elsrt.Chain(cc->el_list))
       cc->calc_invar(tol);
    else {
       cc.Delete();
       break;
    }
  }

  Cont_Clsd_D_List clist;
  Cont_Clsd_Cursor clc(clist);

  cc.To_Begin();
  while (cc) {
    if (cc->Closed() && !cc->Self_Intersecting()) {
      clc.Insert(Cont_Clsd());

      cc->Move_To(clc->cont); clc->calc_invar();

      cc.Delete();
    }
    else ++cc;
  }

  if (clc) {
    clc.To_Begin(); 
    Cont_Clsd_Cursor clcend(clist); clcend.To_End();

    IT_Sort<Cont_Clsd_Cursor,Cont_Area_Cmp>::
          Sort(clc,clcend,Cont_Area_Cmp());

    clc.To_Begin(); if (!clc->Ccw()) clc->Reverse();

    Cont_Nest_Cursor nsc(nestlst); nsc.Insert(Cont_Nest());

    nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();

    calc_invar();

    while (clc) {
      Cont_Ref_List arref(*this); Cont_Ref_List cntref(*clc);
  
      Cont2_Isect_List isctlst(arref,false,cntref,false,true);
  
      if (!isctlst.Empty()) {
        Cont_PPair_List pplst;
        isctlst.Intersections_Into(pplst); pplst.Append_To(intersections);
        ++clc;
      }
      else {
        nsc.To_Begin();
        for (;nsc;++nsc) {
          const Contour *colinear = NULL;
  
          bool inside = Cnt_Inside(*clc,*nsc,colinear);
          if (colinear) {
            ++clc;
            break;
          }
   
          if (inside) {
            // Remove wrong contours from nest
            Cont_Clsd_Cursor nclc(nsc->contlst);

            ++nclc;
            while (nclc) {
              if (Cnt_Inside(*nclc,*clc,colinear)) {
                clist.End().Re_Insert(nclc);
              }
              else ++nclc;
            }
            
            if (clc->Ccw()) clc->Reverse();
            nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();
            break;
          }
        }
  
        if (!nsc) {
          nsc.Insert(Cont_Nest());
          if (!clc->Ccw()) clc->Reverse();
          nsc->contlst.End().Re_Insert(clc); nsc->calc_invar();
        }
      }
  
      calc_invar();
    }
  }

  // Push back unused closed contours

  clc.To_Begin();

  while (clc) {
    cc.To_End(); cc.Insert(Contour());
    clc->cont.Move_To(*cc);
    clc.Delete();
  }

  lst.calc_invar();

  // Now the area should be checked for fold backs
  // The removed parts to be returned to list src

  Merge_Elems();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area& Cont_Area::operator=(const Cont_Area& src)
{
  Rect_Ax::operator=(src);
  nestlst    = src.nestlst;
  lccw       = src.lccw;
  z          = src.z;
  inert      = src.inert;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area& Cont_Area::operator=(const Cont_Nest& src)
{
  z = 0.0;
  nestlst.Delete();

  if (!src.Empty()) {
    Cont_Nest_Cursor nsc(nestlst);

    nsc.Insert(src);
  }
  
  calc_invar();
  
  inert.invalidate();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area& Cont_Area::operator=(const Cont_Clsd& src)
{
  z = 0.0;
  nestlst.Delete();

  if (!src.Empty()) {
    Cont_Nest_Cursor nsc(nestlst);

    nsc.Insert(Cont_Nest());

    Cont_Clsd_Cursor cc(nsc->contlst);

    cc.Insert(src);

    nsc->calc_invar();
  }

  calc_invar();

  inert.invalidate();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Move_To(Elem_List& dst)
{
  dst.Delete();
  
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor cc(nsc->contlst);

    for (;cc;++cc) {
      Elem_List clst;
      cc->Move_To(clst);
      clst.Append_To(dst);
    }
  }

  nestlst.Delete();
  
  calc_invar();
  
  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Move_To(Cont_Clsd_D_List& dst)
{
  dst.Delete();

  Cont_Clsd_Cursor cntc(dst);

  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor clsdc(nsc->contlst);

    for (;clsdc;++clsdc) {
      cntc.To_End(); cntc.Insert(Cont_Clsd());

      clsdc->Move_To(*cntc);
    }

    nsc->calc_invar();
  }

  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Move_To(Cont_List& dst)
{
  dst.contlst.Delete();
  dst.calc_invar();

  Cont_Cursor cntc(dst.contlst);

  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor clsdc(nsc->contlst);

    for (;clsdc;++clsdc) {
      cntc.To_End(); cntc.Insert(Cont_Clsd());

      clsdc->cont.Move_To(*cntc);
    }

    nsc->calc_invar();
  }

  dst.calc_invar();
  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Move_To(Cont_Area& dst)
{
  dst.Rect_Ax::operator=(*this);

  nestlst.Move_To(dst.nestlst);

  dst.lccw       = lccw;
  dst.z          = z;
  dst.inert      = inert;

  inert.invalidate();

  calc_invar();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Delete()
{
  nestlst.Delete();

  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Area::Elem_Count() const
{
  int count = 0;
  
  Cont_Nest_C_Cursor nsc(nestlst);
  
  for (;nsc;++nsc) count += nsc->Elem_Count();

  return count;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Area::Contour_Count() const
{
  int count = 0;

  Cont_Nest_C_Cursor nsc(nestlst);

  for (;nsc;++nsc) count += nsc->List().Length();

  return count;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Area::Nest_Count() const
{
  return nestlst.Length();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Begin_Par(double par)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    nsc->Begin_Par(par);

    double epar = nsc->End_Par();

    par = 2.0 * epar - par;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Area::Begin_Par() const
{
  if (!nestlst) return 0.0;
  else return nestlst.Begin()->Begin_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Area::End_Par() const
{
  if (!nestlst) return 0.0;
  else return nestlst.Last()->End_Par();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Set_Elem_Ids(int newid)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Set_Elem_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Sequence_Elem_Ids(int start_id)
{
  int bid = start_id;

  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor cc(nsc->contlst);

    for (;cc;++cc) {
      cc->Sequence_Elem_Ids(bid);

      bid += cc->cont.List().Length();
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Find_Last_Pred_Of(int id, Elem_Cursor& elc) const
{
  Elem_Cursor minelc, maxelc;
  int minid = -1, maxid = -1;

  Cont_Nest_C_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_C_Cursor cc(*nsc);

    for (;cc;++cc) {
      elc = cc->cont.el_list.Begin();

      for (;elc;++elc) {
        const Elem& el = elc->El();

        if (el.Id() == id) return;

        if (el.Id() < id) {
          if (!minelc || el.Id() > minid) {
            minelc = elc; minid = el.Id();
          }
        }
        else {
          if (!maxelc || el.Id() < maxid) {
            maxelc = elc; maxid = el.Id();
          }
        }
      }
    }
  }

  elc.To_End();

  if (minelc) elc = minelc;
  else if (maxelc) {
    elc = maxelc;
    --elc; if (!elc) elc.To_Begin();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Find_First_Id_With(int id, Elem_Cursor& elc) const
{
  elc.To_End();

  Cont_Nest_C_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_C_Cursor cc(*nsc);

    for (;cc;++cc) {
      elc = cc->cont.el_list.Begin();

      for (;elc;++elc) {
        if (elc->El().Id() == id) return;
      }
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Set_Elem_Cnt_Ids(int newid)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Set_Elem_Cnt_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Set_Elem_P_Cnt_Ids(int newid)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Set_Elem_P_Cnt_Ids(newid);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Sequence_Elem_Cnt_Ids(int& start_id)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Sequence_Elem_Cnt_Ids(start_id);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Cont_Area::Area_XY() const
{
  if (!inert.area_valid) inert.calc_area(*this);
  
  return inert.area;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

const Cont_Inert& Cont_Area::Inert() const
{
  if (!inert.area_valid)  inert.calc_area(*this);
  if (!inert.inert_valid) inert.calc_inert(*this);
  
  return inert;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Remove_Inners()
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor cc(nsc->contlst); ++cc;

    while (cc) cc.Delete();
  }
  
  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Extract_Via(const Vec2& p1, const Vec2& probedir1,
                   const Vec2& p2, const Vec2& probedir2,
                   Contour& path, double& pathlen) const
{
  path.Delete();
  pathlen = 0.0;

  Cont_Pnt pnt1, pnt2;
  double dist1, dist2, pdist1, pdist2;

  if (!Project_Pnt_XY(p1 + probedir1,pnt1,pdist1))
                                       Cont_Panic(Cont_Area_Cant_Project);
  if (!Project_Pnt_XY(p2 + probedir2,pnt2,pdist2))
                                       Cont_Panic(Cont_Area_Cant_Project);

  if (pnt1.Parent_Contour() != pnt2.Parent_Contour()) return false;

  pnt1.Par_Dist_To(pnt2,dist1);
  pnt2.Par_Dist_To(pnt1,dist2);

  if (dist1 < dist2) {
    if (dist1 > 3.0 * Vec2::IdentDist) {
      if (!pnt1.Extract_Upto(pnt2,path)) Cont_Panic(Cont_Area_Cant_Extract);
    }
  }
  else {
    if (dist2 > 3.0 * Vec2::IdentDist) {
      if (!pnt2.Extract_Upto(pnt1,path)) Cont_Panic(Cont_Area_Cant_Extract);
      path.Reverse();
    }
  }

  if (!path.Empty()) {
    Elem_Cursor pelc(path.el_list);

    if (pelc->El().Len_XY() < 3.0 * Vec2::IdentDist) {
      pelc.Delete();
      path.inval_rects();
      path.calc_invar();
    }

    pathlen = path.Len_XY();
  }

  if (!path.Empty()) {
    Elem_Cursor pelc(path.el_list); pelc.To_Last();
    double ellen = pelc->El().Len_XY();

    if (ellen < 3.0 * Vec2::IdentDist) {
      pelc.Delete();
      path.inval_rects();
      path.calc_invar();

      pathlen = path.Len_XY();
    }
  }

  Elem_Cursor elc(path.el_list);

  if (path.Empty()) {
    Elem_Line line(p1,p2);
    elc.Insert(line);
  }
  else {
    dist1 = p1.distTo2(path.Begin_Point());

    if (dist1 > 3.0 * Vec2::IdentDist) {
      Elem_Line line1(p1,path.Begin_Point());
      elc.Insert(line1);
    }

    dist2 = p2.distTo2(path.End_Point());

    if (dist2 > 3.0 * Vec2::IdentDist) {
      Elem_Line line2(path.Begin_Point(),p2);
      elc.To_End();
      elc.Insert(line2);
    }

    if (!path.Empty()) {
      elc.To_Begin(); elc->El().Stretch_Begin_XY(p1);
      elc.To_Last();  elc->El().Stretch_End_XY(p2);
    }
  }

  path.inval_rects();
  path.calc_invar();
  path.Begin_Par(0.0);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Project_Pnt_XY(const Vec2& p, Cont_Pnt& cntp,
                                                 double& dist_xy) const
{
  cntp = Cont_Pnt();

  if (!nestlst) return false;

  Cont_Nest_C_Cursor nsc(nestlst);
  Cont_Nest_C_Cursor minsc(nsc);

  double mindist = nsc->Rect().Dist_To_XY(p);

  while (++nsc && mindist > 0.0) {
    double curdist = nsc->Rect().Dist_To_XY(p);

    if (curdist < mindist) {
      mindist = curdist;
      minsc   = nsc;
    }
    
    ++nsc;
  }

  nsc = minsc;

  if (!nsc->Project_Pnt_XY(p,cntp,dist_xy)) Cont_Panic(Cont_Area_Cant_Project);

  if ((dist_xy > 0.0) == lccw) return true;

  ++nsc; if (!nsc) nsc.To_Begin();

  while (nsc != minsc) {
    Cont_Pnt curpnt;
    double curdist;

    if (!nsc->Project_Pnt_XY(p,curpnt,curdist))
                                            Cont_Panic(Cont_Area_Cant_Project);

    if ((curdist > 0.0) == lccw) {
      cntp = curpnt;
      dist_xy = curdist;
      return true;
    }

    if (fabs(curdist) < fabs(dist_xy)) {
      cntp = curpnt;
      dist_xy = curdist;
    }

    ++nsc; if (!nsc) nsc.To_Begin();
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ------- Project sp and ep on area, and find parametric distance ------ */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Par_Dist_Along(const Vec2& sp, const Vec2& ep,
                               Cont_Pnt& spnt, double &sdist,
                               Cont_Pnt& epnt, double& edist,
                               double& pdist, bool& rev) const
{
  if (!Project_Pnt_XY(sp,spnt,sdist)) Cont_Panic(Cont_Area_Cant_Project);
  if (!Project_Pnt_XY(ep,epnt,edist)) Cont_Panic(Cont_Area_Cant_Project);

  if (!spnt.Par_Dist_To(epnt,pdist)) return false;

  const Contour *cnt = spnt.Parent_Contour();

  rev = false;

  if (cnt->Closed()) {
    if (pdist > cnt->Len()/2.0) {
      rev = true;
      pdist = cnt->Len() - pdist;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Offset_Into(double offdist, Cont_Area& ar_list) const
{
  ar_list.nestlst.Delete();
  ar_list.calc_invar();
  ar_list.inert.invalidate();

  if (Empty()) return false;

  if (fabs(offdist) < Vec2::IdentDist) {  // Just copy

    ar_list = *this;

    return true;
  }

  double begpar = Begin_Par();

  Cont_Nest_C_Cursor srcc(nestlst);

  if (!srcc->Offset_Into(offdist,ar_list))
                                       Cont_Panic(Cont_Area_Cant_Offset);

  if ((offdist > 0.0) == lccw) {   // Offset inward
    while (++srcc) {
      Cont_Area ar2;
      if (!srcc->Offset_Into(offdist,ar2))
                                       Cont_Panic(Cont_Area_Cant_Offset);

      ar2.nestlst.Append_To(ar_list.nestlst);
    }
  }
  else {                          // Offset outward
    if (ar_list.Empty()) Cont_Panic(Cont_Area_Cant_Offset);

    while (++srcc) {
      Cont_Area ar2;

      if (!srcc->Offset_Into(offdist,ar2) || ar2.Empty())
                                       Cont_Panic(Cont_Area_Cant_Offset);

      Cont_Area arhlp;
      ar_list.Combine_With(false,ar2,false,offdist > 0.0,arhlp);

      arhlp.Move_To(ar_list);
    }
  }

  ar_list.calc_invar();

  ar_list.Begin_Par(begpar);

  ar_list.z = z;

  // Tijdelijk
//   Markup errmark(3.0,false,"clemens","","","",true);
// 
//   errmark.Set_Penlt(5,1);
//   errmark.Draw(ar_list);
//   errmark.Reset();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::build_from(Cont_Clsd_D_List& cnt_list)
{
  nestlst.Delete();
  calc_invar();
  inert.invalidate();

  if (!cnt_list) return false;

  Cont_Clsd_Cursor begc(cnt_list);
  Cont_Clsd_Cursor endc(cnt_list); endc.To_End();

  IT_Sort<Cont_Clsd_Cursor,Cont_Area_Cmp>::
                      Sort(begc,endc,Cont_Area_Cmp());

  Cont_Clsd_Cursor srcc(cnt_list);

  // Delete contours with an almost zero average thickness
  
  while (srcc) {
    srcc->Begin_Par(0.0);
    
    double artol = 10.0 * Vec2::IdentDist;
    
    if (srcc->End_Par() < artol ||
        fabs(srcc->Area_XY()/srcc->End_Par()*2.0) < artol) srcc.Delete();
    else ++srcc;
  }

  srcc.To_Begin();
  if (!srcc) return true;

  bool ccw = srcc->Ccw();

  Cont_Nest_Cursor nsc(nestlst); nsc.Insert(Cont_Nest());

  nsc->contlst.End().Re_Insert(srcc);
  nsc->calc_invar();

  while (srcc) {
    bool llccw = srcc->Ccw();

    if (llccw == ccw) {   // Add another Cont_Nest
      nsc.To_End(); nsc.Insert(Cont_Nest());

      nsc->contlst.End().Re_Insert(srcc);

      nsc->calc_invar();
    }
    else {                      // Must be inside an existing nest
      nsc.To_Begin();

      for (;nsc;++nsc) {
        const Contour *colinear = NULL;

        if (Cnt_Inside(*srcc,*nsc,colinear)) {
          nsc->contlst.End().Re_Insert(srcc);

          nsc->calc_invar();
          break;
        }
      }

      if (!nsc) {
        Cont_Panic(Cont_Cant_Sort_Contours);
               // Something is very wrong!!!!
      }
    }
  }

  calc_invar();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::compile_from_cont(Elem_List& el_lst,
                                  bool is_ccw, bool to_left, double tol)
{
  Delete();

  Contour offcnt;

  el_lst.Move_To(offcnt.el_list);

  offcnt.calc_invar();
  offcnt.is_closed = true;

  offcnt.Begin_Par(0.0);

  Cont_Ref offref(offcnt,is_ccw);

  Cont1_Isect_List isect(offref, true, to_left);

  // draw_isect_list(isect,"isects",5);

  Cont_Clsd_D_List offlist;

  if (!isect.Extract_Offset_Closed_Special(tol,offlist)) return false;

  if (offlist.Length() < 1) {
    return is_ccw == to_left;
  }

  bool ok = build_from(offlist);

  calc_invar();

  Begin_Par(0.0);

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::append_clsd_cnt(const Cont_Clsd& cont)
{
  Cont_Nest_Cursor nsc(nestlst);

  if (!nsc) nsc.Insert(Cont_Nest());
  else {

    Cont_Ref_List arref(*this); Cont_Ref_List cntref(cont);
    Cont2_Isect_List isctlst(arref,false,cntref,false,true);
  
    // New contour must not intersect the ones already present

    if (!isctlst.Empty()) return false;
  }
  
  nsc.To_Last();

  Cont_Clsd_Cursor cc(nsc->contlst); cc.To_End();
  cc.Insert(cont);
  
  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Combine_With(bool rev1, const Cont_Area& ar2, bool rev2,
                             bool to_left, Cont_Area& into,
                             Cont_List *rest1, Cont_List *rest2) const
{
  if (&into == this || &into == &ar2) return false;

  into.nestlst.Delete();
  into.calc_invar();
  into.inert.invalidate();

  if (rest1) {
    rest1->contlst.Delete(); rest1->calc_invar();
  }

  if (rest2) {
    rest2->contlst.Delete(); rest2->calc_invar();
  }

  if (Empty() || ar2.Empty()) return false;

  Cont_Ref_List arref1(*this);
  Cont_Ref_List arref2(ar2);

  Cont2_Isect_List isctlst(arref1,rev1,arref2,rev2,to_left);

  Cont_Clsd_D_List clsd_list;

  Cont_D_List *r1 = NULL; if (rest1) r1 = &(rest1->contlst);
  Cont_D_List *r2 = NULL; if (rest2) r2 = &(rest2->contlst);
  
  isctlst.Extract_Combine(clsd_list,r1,r2);

  bool ok = into.build_from(clsd_list);

  into.Begin_Par(Begin_Par());

  if (rest1) rest1->calc_invar();
  if (rest2) rest2->calc_invar();

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Reverse()
{
  Cont_Nest_Cursor nsc(nestlst);

  if (nsc) {
    double begpar = Begin_Par();

    for (;nsc;++nsc) nsc->Reverse();

    nestlst.Reverse();

    Begin_Par(begpar);
  }

  lccw = !lccw;
  
  if (inert.area_valid) inert.area = -inert.area;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Del_Info()
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Del_Info();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Delete_Unisource_Nests()
{
  Cont_Nest_Cursor nsc(nestlst);

  while (nsc) {
    if (nsc->uni_source()) nsc.Delete();
    else ++nsc;
  }

  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Delete_Non_Unisource_Nests(int el_id)
{
  Cont_Nest_Cursor nsc(nestlst);

  while (nsc) {
    if (!nsc->uni_source(el_id)) nsc.Delete();
    else ++nsc;
  }

  calc_invar();

  inert.invalidate();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Merge_Elems(bool limit_arcs)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Merge_Elems(limit_arcs);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Set_Elem_Z(double new_z)
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) nsc->Set_Elem_Z(new_z);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Start_Outside()
{
  Cont_Nest_Cursor nsc(nestlst);

  for (;nsc;++nsc) {
    Cont_Clsd_Cursor cc(nsc->contlst);

    for (;cc;++cc) cc->cont.Start_Outside(*this);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Area::Filter_Arcs(bool ccw, double tol)
{
  bool modified_ar = false;

  Cont_Nest_Cursor nsc(nestlst);
  
  for (;nsc;++nsc) {
    bool modified_nest = false;
    
    Cont_Clsd_Cursor cc(nsc->contlst);
    
    for (;cc;++cc) {
      double bpar = cc->Begin_Par();
      
      if (Filter_Ins_Arcs(cc->cont.el_list, ccw, tol)) {

        // Ovl_Display("modified");
        modified_nest = true;
        
        cc->Begin_Par(bpar);
        cc->cont.inval_rects();
        cc->calc_invar();
        cc->cont.inert.invalidate();
      }
    }
    
    if (modified_nest) {
      modified_ar = true;
      nsc->calc_invar();
      nsc->inert.invalidate();
    }
  }
  
  if (modified_ar) {
    calc_invar();
    
    inert.invalidate();
  }
  // else Ovl_Display("no modify");
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Min_Left_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Cont_Nest_C_Cursor nsc(*this);
  
  bool found = false;
  
  for (;nsc;++nsc) {
    double nrad;
    
    if (nsc->Min_Left_Rad(nrad)) {
      if (!found || nrad < min_rad) min_rad = nrad;
      found = true;
    }
  }
  
  return found;
}

/* ---------------------------------------------------------------------- */
/* ----- Find Minimum Lefthand Arc Radius ------------------------------- */
/* ----- Return true if found (sharp transitions may still be present!) - */
/* ---------------------------------------------------------------------- */

bool Cont_Area::Min_Right_Rad(double& min_rad) const
{
  min_rad = 0.0;
  
  Cont_Nest_C_Cursor nsc(*this);
  
  bool found = false;
  
  for (;nsc;++nsc) {
    double nrad;
    
    if (nsc->Min_Right_Rad(nrad)) {
      if (!found || nrad < min_rad) min_rad = nrad;
      found = true;
    }
  }
  
  return found;
}

} // namespace Ino
  
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
