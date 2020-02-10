/* ---------------------------------------------------------------------- */
/* ---------------- 3D Line Element ------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "El_Line.h"

#include "Geo.h"
#include "Isect.h"
#include "it_gen.h"

#include "cntpanic.hi"

#include <math.h>

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void *store = NULL;

struct store_line
{
  store_line *next;
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void *Elem_Line::operator new(size_t)
{
  if (!store) return new char[sizeof(Elem_Line)];

  void *newl = store;
  store = ((store_line *)store)->next;

  return newl;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::operator delete(void *old_line)
{
  ((store_line *)old_line)->next = (store_line *)store;
  store = old_line;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::CleanupStore()
{
  store_line *fst = (store_line *)store;

  while (fst) {
    store_line *nxt = fst->next;

    delete[] (char *)fst;
    fst = nxt;
  }

  store = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::update()
{
  len_xy = lp1.distTo2(lp2);
  len = lp1.distTo3(lp2);
  plen = len;

  Rect_Ax::Rect_Update(lp1,lp2);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Line::Elem_Line(const Vec3& p1, const Vec3& p2)
  : Elem(), lp1(p1), lp2(p2)
{
  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Line::Elem_Line(const Elem_Line& cp)
 : Elem(cp), lp1(cp.lp1), lp2(cp.lp2)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Line::Elem_Line(const Elem_Line& cp, bool keep_info)
 : Elem(cp, keep_info), lp1(cp.lp1), lp2(cp.lp2)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Line& Elem_Line::operator=(const Elem_Line& src)
{
  clone_from(src);

  lp1 = src.lp1;
  lp2 = src.lp2;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::At_Par(double par, Vec3& p) const
{
  Vec3 dir(lp2); dir -= lp1; dir.unitLen3();

  p = lp1 + dir*(par - bpar);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2 Elem_Line::Start_Tangent_XY() const
{
  Vec2 dir(lp2); dir -= lp1; dir.unitLen2();

  return dir;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2 Elem_Line::End_Tangent_XY() const
{
  Vec2 dir(lp2); dir -= lp1; dir.unitLen2();

  return dir;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Line::Start_Tangent_XY(Vec2& tg) const
{
  tg = lp2 - lp1; tg.unitLen2();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Line::End_Tangent_XY(Vec2& tg) const
{
  tg = lp2 - lp1; tg.unitLen2();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Tangent_At_XY(double /*par*/, Vec2& tg) const
{
  Start_Tangent_XY(tg);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Elem_Line::Start_Tangent() const
{
  Vec3 dir(lp2); dir -= lp1; dir.unitLen3();

  return dir;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Elem_Line::End_Tangent() const
{
  Vec3 dir(lp2); dir -= lp1; dir.unitLen3();

  return dir;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3& Elem_Line::Start_Tangent(Vec3& tg) const
{
  tg = lp2 - lp1; tg.unitLen3();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3& Elem_Line::End_Tangent(Vec3& tg) const
{
  tg = lp2 - lp1; tg.unitLen3();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Tangent_At(double /*par*/, Vec3& tg) const
{
  Start_Tangent(tg);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Curve_At(double /*par*/, double& curve) const
{
  curve = 0.0;
  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Line::Area_XY_P1() const
{
  return (lp2.y - lp1.y) * (lp1.x - lp2.x) / 2.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Line::Moment_X() const
{
  double ydiff = lp2.y-lp1.y;
  
  double mom = ydiff * (lp2.x-lp1.x) * (lp2.x+2.0*lp1.x) / 6.0;

  mom += (ydiff * sqr(lp1.x) / 2.0);
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Line::Moment_Y() const
{
  double xdiff = lp1.x-lp2.x;
  
  double mom = xdiff * (lp2.y-lp1.y) * (lp2.y+2.0*lp1.y) / 6.0;

  mom += (xdiff * sqr(lp1.y) / 2.0);
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Reverse()
{
  Vec3 hold(lp1); lp1 = lp2; lp2 = hold;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Project_Pnt_XY(const Vec2& p, double tol, bool strict,
                               Vec3& pp,
                               double& parm, double &dist_xy) const
{
  bool ok = Geo_Project_P_on_Line(p,lp1,lp2,strict,tol,pp,parm,dist_xy);

  if (ok) {
    parm += bpar;
    if (!Par(parm,parm)) return false;

    Vec3 ip;
    if (!At_Par(parm,ip)) return false;
    else pp.z = ip.z;
  }
  else if (parm <= 0.0) {
    parm = bpar;
    pp   = lp1; 
    dist_xy = p.distTo2(pp);

    Vec3 nrm; Start_Tangent(nrm); nrm.rot90();
    Vec2 dp = p; dp -= lp1;

    if (dp * nrm < 0.0) dist_xy = -dist_xy;
  }
  else {
    parm = bpar + plen;
    pp   = lp2;
    dist_xy = p.distTo2(pp);

    Vec3 nrm; End_Tangent(nrm); nrm.rot90();
    Vec2 dp = p; dp -= lp2;

    if (dp * nrm < 0.0) dist_xy = -dist_xy;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Project_Pnt_Strict_XY(const Vec2& p, double tol,
                                      Vec3& pp,
                                      double& parm, double &dist_xy) const
{
  bool ok = Geo_Project_P_on_Line(p,lp1,lp2,true,tol,pp,parm,dist_xy);

  if (ok) {
    parm += bpar;
    if (!Par(parm,parm)) return false;

    Vec3 ip;
    if (!At_Par(parm,ip)) return false;
    else pp.z = ip.z;
  }

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Intersect_XY(const Elem& el, Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (!Rect_Ax::Intersects_XY(el.Rect(),Vec2::IdentDist)) return;

  if (el.Type() > Elem_Type_Line) {
    el.Intersect_XY(*this,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();

    Isect_Cursor begin(isect_lst);
    Isect_Cursor end(isect_lst.End());

    IT_Sort<Isect_Cursor,Isect_Cmp>::
       Sort(begin,end,Isect_Cmp());
  }
  else {
    Vec3 ipa;
    double pr1a, pr2a;
    int sols = Geo_Isct_Lines(lp1,lp2,el.P1(),el.P2(),true,
                              Vec2::IdentDist,ipa,pr1a,pr2a);

    if (sols > 0) {
      pr1a += bpar; pr2a += el.Begin_Par();

      if (!Par(pr1a,pr1a)) return;
      if (!Par(pr2a,pr2a)) return;

      Vec3 ip;
      if (!At_Par(pr1a,ip)) return;
      ipa.z = ip.z;

      Vec3 tg1, tg2;

      Start_Tangent(tg1);
      el.Start_Tangent(tg2);

      isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,0.0,tg2,pr2a,0.0));
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Intersect_XY(const Elem& el, bool tang_ok,
                                               Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (!Rect_Ax::Intersects_XY(el.Rect(),Vec2::IdentDist)) return;

  if (el.Type() > Elem_Type_Line) {
    el.Intersect_XY(*this,tang_ok,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();

    Isect_Cursor begin(isect_lst);
    Isect_Cursor end(isect_lst.End());

    IT_Sort<Isect_Cursor,Isect_Cmp>::
       Sort(begin,end,Isect_Cmp());
  }
  else {
    Vec3 ipa;
    double pr1a, pr2a;
    int sols = Geo_Isct_Lines(lp1,lp2,el.P1(),el.P2(),true,
                              Vec2::IdentDist,ipa,pr1a,pr2a);

    if (sols > 0) {
      pr1a += bpar; pr2a += el.Begin_Par();

      if (!Par(pr1a,pr1a)) return;
      if (!Par(pr2a,pr2a)) return;

      Vec3 ip;
      if (!At_Par(pr1a,ip)) return;
      ipa.z = ip.z;

      Vec3 tg1, tg2;

      Start_Tangent(tg1);
      el.Start_Tangent(tg2);

      isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,0.0,tg2,pr2a,0.0));
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Intersect "infinite" elements -------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Intersect_XY_Ext(const Elem& el, bool tang_ok,
                                               Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (el.Type() > Elem_Type_Line) {
    el.Intersect_XY_Ext(*this,tang_ok,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();

    Isect_Cursor begin(isect_lst);
    Isect_Cursor end(isect_lst.End());

    IT_Sort<Isect_Cursor,Isect_Cmp>::
       Sort(begin,end,Isect_Cmp());
  }
  else {
    Vec3 ipa;
    double pr1a, pr2a;
    int sols = Geo_Isct_Lines(lp1,lp2,el.P1(),el.P2(),false,
                              Vec2::IdentDist,ipa,pr1a,pr2a);

    if (sols > 0) {
      pr1a += bpar; pr2a += el.Begin_Par();

      if (!Par(pr1a,pr1a)) return;
      if (!Par(pr2a,pr2a)) return;

      Vec3 ip;
      if (!At_Par(pr1a,ip)) return;
      ipa.z = ip.z;

      Vec3 tg1, tg2;

      Start_Tangent(tg1);
      el.Start_Tangent(tg2);

      isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,0.0,tg2,pr2a,0.0));
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Offset_Into(double dist, Elem_List& offel,
                                                 bool keep_info) const
{
  offel.Delete();

  Elem_Line newel(*this, keep_info); newel.p_cnt_id = cnt_id;

  Vec2 d = lp2 - lp1; d.unitLen2(); d.rot90(); d *= dist;

  newel.lp1 += d; newel.lp2 += d;

  newel.update();
 
  offel.Push_Back(newel);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Offset_Into(double dist, double /*minlen*/,
                            Elem_List& off_lst, Vec3& strtpt, Vec3& endpt,
                                                   double& mislen) const
{
  off_lst.Delete();

  Elem_Line newel(*this, false); newel.p_cnt_id = cnt_id;

  Vec2 d = lp2 - lp1; d.unitLen2(); d.rot90(); d *= dist;

  newel.lp1 += d; newel.lp2 += d;

  newel.update();
 
  off_lst.Push_Back(newel);

  strtpt = newel.lp1;
  endpt  = newel.lp2;

  mislen = 0.0;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Line::Split(double par, Elem_List& spl_lst) const
{
  spl_lst.Delete();

  double lpar = par - bpar;
  Vec3 newp;

  if (lpar < Vec2::IdentDist || lpar > len-Vec2::IdentDist ||
                                                 !At_Par(par,newp)) {
    spl_lst.Push_Back(*this);
    return false;
  }

  Elem_Line newline1(*this);
  Elem_Line newline2(*this);

  newline1.lp2 = newp; newline2.lp1 = newp;

  newline2.bpar = par;
  
  newline1.update();
  newline2.update();

  spl_lst.Push_Back(newline1);
  spl_lst.Push_Back(newline2);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Stretch_Begin_XY(const Vec2& isp, bool check)
{
  double endpar = End_Par();

  if (check && isp.distTo2(lp1) > 10.0 * Vec2::IdentDist) 
                                       Cont_Panic(Elem_Large_Stretch);

  lp1.x = isp.x;
  lp1.y = isp.y;

  update();

  bpar = (endpar - len);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Stretch_End_XY(const Vec2& isp, bool check)
{
  if (check && isp.distTo2(lp2) > 10.0 * Vec2::IdentDist) 
                                       Cont_Panic(Elem_Large_Stretch);

  lp2.x = isp.x;
  lp2.y = isp.y;

  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem *Elem_Line::Merge_With(const Elem_Line& el) const
{
  Elem *newel = NULL;

  if (lp2 != el.lp1) return NULL;

  Vec2 dp = lp2 - lp1; dp.unitLen2();
  Vec2 ds = el.lp2 - el.lp1;

  if (ds * dp < 0.0) return NULL;

  dp.rot90();
  if (fabs(ds * dp) > Vec2::IdentDist) return NULL;

  newel = new Elem_Line(*this);

  ((Elem_Line *)newel)->lp2 = el.lp2;

  ((Elem_Line *)newel)->update();

  return newel;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Line::Transform(const Trf2& trf)
{
  lp1.transform2(trf);
  lp2.transform2(trf);

  update();
}

/* ---------------------------------------------------------------------- */

bool Elem_Line::Fillet(const Elem& el2, double filletRad,
                                                  Elem_Arc& fillet) const
{
  switch (el2.Type()) {
    case Elem_Type_Line:
      return false;

    default:
      return el2.Fillet(*this,-filletRad,fillet);
  }
}

//---------------------------------------------------------------------------
// Persistable Section

static const char fldP1x[] = "P1x";
static const char fldP1y[] = "P1y";
static const char fldP1z[] = "P1z";
static const char fldP2x[] = "P2x";
static const char fldP2y[] = "P2y";
static const char fldP2z[] = "P2z";

//---------------------------------------------------------------------------

Elem_Line::Elem_Line(PersistentReader& pi)
: Elem(pi),
  lp1(pi.readDouble(fldP1x,0.0),pi.readDouble(fldP1y,0.0),
                                                  pi.readDouble(fldP1z,0.0)),
  lp2(pi.readDouble(fldP2x,0.0),pi.readDouble(fldP2y,0.0),
                                                  pi.readDouble(fldP2z,0.0))
{
  update();
}

//---------------------------------------------------------------------------

void Elem_Line::definePersistentFields(PersistentWriter& po) const
{
  Elem::definePersistentFields(po);

  po.addField(fldP1x,typeid(double));
  po.addField(fldP1y,typeid(double));
  po.addField(fldP1z,typeid(double));
  po.addField(fldP2x,typeid(double));
  po.addField(fldP2y,typeid(double));
  po.addField(fldP2z,typeid(double));
}

//---------------------------------------------------------------------------

void Elem_Line::writePersistentObject(PersistentWriter& po) const
{
  Elem::writePersistentObject(po);

  po.writeDouble(fldP1x,lp1.x);
  po.writeDouble(fldP1y,lp1.y);
  po.writeDouble(fldP1z,lp1.z);

  po.writeDouble(fldP2x,lp2.x);
  po.writeDouble(fldP2y,lp2.y);
  po.writeDouble(fldP2z,lp2.z);
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
