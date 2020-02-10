/* ---------------------------------------------------------------------- */
/* ---------------- 3D Arc Element -------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "El_Arc.h"

#include "El_Line.h"
#include "El_Cir.h"

#include "Trf.h"
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

struct store_arc
{
  store_arc *next;
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void *Elem_Arc::operator new(size_t)
{
  if (!store) return new char[sizeof(Elem_Arc)];

  void *newl = store;
  store = ((store_arc *)store)->next;

  return newl;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::operator delete(void *old_arc)
{
  ((store_arc *)old_arc)->next = (store_arc *)store;
  store = old_arc;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::CleanupStore()
{
  store_arc *fst = (store_arc *)store;

  while (fst) {
    store_arc *nxt = fst->next;

    delete[] (char *)fst;
    fst = nxt;
  }

  store = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::update()
{
  len_xy = Geo_Arc_Len(lp1,lp2,cntre,lccw);

  len = sqrt(sqr(len_xy) + sqr(lp2.z-lp1.z));

  plen = len;

  Rect_Ax::Around_Arc(lp1,lp2,cntre,lccw);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Arc::Elem_Arc(const Vec3& p1, const Vec3& p2, const Vec2& c, bool ccw)
 : Elem(), lp1(p1), lp2(p2), cntre(c), lccw(ccw)
{
  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Arc::Elem_Arc(const Elem_Arc& cp)
 : Elem(cp), lp1(cp.lp1), lp2(cp.lp2), cntre(cp.cntre), lccw(cp.lccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Arc::Elem_Arc(const Elem_Arc& cp, bool keep_info)
 : Elem(cp, keep_info), lp1(cp.lp1), lp2(cp.lp2),
   cntre(cp.cntre), lccw(cp.lccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Arc& Elem_Arc::operator=(const Elem_Arc& src)
{
  clone_from(src);

  lp1   = src.lp1;
  lp2   = src.lp2;
  cntre = src.cntre;
  lccw  = src.lccw;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::At_Par(double par, Vec3& p) const
{
  double par_xy;
  if (!Par_XY(par,par_xy)) return false;

  par_xy -= bpar;

  double r = R();
  if (r <= NumAccuracy * fabs(par_xy)) return false;

  double angle = par_xy/r;

  if (!lccw) angle = -angle;

  angle = fmod(angle,Vec2::Pi2);

  Vec2 dp1 = lp1 - cntre; dp1.unitLen2();

  Vec2 dp(cos(angle)*r,sin(angle)*r);

  p.x = dp1.x * dp.x - dp1.y * dp.y + cntre.x;
  p.y = dp1.y * dp.x + dp1.x * dp.y + cntre.y;

  p.z = lp1.z + (par - bpar)/len*(lp2.z-lp1.z);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2 Elem_Arc::Start_Tangent_XY() const
{
  Vec2 tg;
  return Start_Tangent_XY(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2 Elem_Arc::End_Tangent_XY() const
{
  Vec2 tg;

  return End_Tangent_XY(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Arc::Start_Tangent_XY(Vec2& tg) const
{
  tg = lp1; tg -= cntre; tg.unitLen2();

  if (lccw) tg.rot90();
  else      tg.rot270();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Arc::End_Tangent_XY(Vec2& tg) const
{
  tg = lp2; tg -= cntre; tg.unitLen2();

  if (lccw) tg.rot90();
  else      tg.rot270();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Tangent_At_XY(double par, Vec2& tg) const
{
  Vec3 p;
  bool ok = At_Par(par,p);

  tg = p; tg -= cntre; tg.unitLen2();

  if (lccw) tg.rot90();
  else      tg.rot270();
  
  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Elem_Arc::Start_Tangent() const
{
  Vec3 tg;
  return Start_Tangent(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Elem_Arc::End_Tangent() const
{
  Vec3 tg;

  return End_Tangent(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3& Elem_Arc::Start_Tangent(Vec3& tg) const
{
  tg = lp1; tg -= cntre; 

  if (lccw) tg.rot90();
  else      tg.rot270();

  tg.unitLen2();
  tg *= len_xy;

  tg.z = lp2.z - lp1.z; tg.unitLen3();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3& Elem_Arc::End_Tangent(Vec3& tg) const
{
  tg = lp2; tg -= cntre; 

  if (lccw) tg.rot90();
  else      tg.rot270();

  tg.unitLen2();
  tg *= len_xy;

  tg.z = lp2.z - lp1.z; tg.unitLen3();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Tangent_At(double par, Vec3& tg) const
{
  bool ok = At_Par(par,tg);

  tg -= cntre;

  if (lccw) tg.rot90();
  else      tg.rot270();
  
  tg.unitLen2();
  tg *= len_xy;

  tg.z = lp2.z - lp1.z; tg.unitLen3();
  
  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::Start_Curve() const
{
  if (lccw) return  1.0/R(); 
  else      return -1.0/R();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::End_Curve() const
{
  if (lccw) return  1.0/R(); 
  else      return -1.0/R();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Curve_At(double /*par*/, double& curve) const
{
  if (lccw) curve =  1.0/R(); 
  else      curve = -1.0/R();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::Area_XY_P1() const
{
  Vec2 dp(lp2);   dp -= lp1;
  Vec2 dc(cntre); dc -= lp1; dc.rot90 ();

  return (Span_Angle() * sqr(R()) + dp * dc - dp.x * dp.y)/2.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static double line_moment_x(const Vec2& p1, const Vec2& p2)
{
  double ydiff = p2.y-p1.y;
  
  double mom = ydiff * (p2.x-p1.x) * (p2.x+2.0*p1.x) / 6.0;

  mom += (ydiff * sqr(p1.x) / 2.0);
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::Moment_X() const
{
  double ydiff = lp2.y - lp1.y;

  double integral = sqr(R())*(3.0*cntre.x*Span_Angle()+2.0*ydiff)/6.0;
  
  integral += line_moment_x(lp1,cntre);
  integral += line_moment_x(cntre,lp2);
  
  return integral;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static double line_moment_y(const Vec2& p1, const Vec2& p2)
{
  double xdiff = p1.x-p2.x;
  
  double mom = xdiff * (p2.y-p1.y) * (p2.y+2.0*p1.y) / 6.0;

  mom += (xdiff * sqr(p1.y) / 2.0);
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::Moment_Y() const
{
  double xdiff = lp1.x - lp2.x;

  double integral = sqr(R())*(3.0*cntre.y*Span_Angle()+2.0*xdiff)/6.0;
  
  integral += line_moment_y(lp1,cntre);
  integral += line_moment_y(cntre,lp2);
  
  return integral;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Arc::Span_Angle() const
{
  return Geo_Norm_Angle(lccw,Start_Tangent().angleTo2(End_Tangent()));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Reverse()
{
  Vec3 hold(lp1); lp1 = lp2; lp2 = hold;

  lccw = !lccw;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Project_Pnt_XY(const Vec2& p, double tol, bool strict,
                               Vec3& pp,
                               double& parm, double &dist_xy) const
{
  bool ok = Geo_Project_P_on_Arc(p,lp1,lp2,cntre,lccw,strict,tol,
                                                   pp,parm,dist_xy);

  if (ok) {
    parm += bpar;
    if (!Par(parm,parm)) return false;

    Vec3 ip;
    if (!At_Par(parm,ip)) return false;
    else pp.z = ip.z;
  }
  else {
    double dist1 = p.distTo2(lp1);
    double dist2 = p.distTo2(lp2);

    if (dist1 < dist2) {
      parm = bpar;
      pp   = lp1; 
      dist_xy = dist1;
  
      Vec3 nrm; Start_Tangent(nrm); nrm.rot90();
      Vec2 dp = p; dp -= lp1;
  
      if (dp * nrm < 0.0) dist_xy = -dist_xy;
    }
    else {
      parm = bpar + plen;
      pp   = lp2;
      dist_xy = dist2;
  
      Vec3 nrm; End_Tangent(nrm); nrm.rot90();
      Vec2 dp = p; dp -= lp2;
  
      if (dp * nrm < 0.0) dist_xy = -dist_xy;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Project_Pnt_Strict_XY(const Vec2& p, double tol,
                                     Vec3& pp,
                                     double& parm, double &dist_xy) const
{
  bool ok = Geo_Project_P_on_Arc(p,lp1,lp2,cntre,lccw,true,tol,
                                                   pp,parm,dist_xy);

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

void Elem_Arc::Intersect_XY(const Elem& el, Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (!Rect_Ax::Intersects_XY(el.Rect(),Vec2::IdentDist)) return;

  int sols = 0;
  double pr1a=0, pr2a=0, pr1b=0, pr2b=0;
  Vec3 ipa, ipb, ip, tg1, tg2;

  double curve1 = 1/R(); if (!lccw) curve1 = -curve1;
  double curve2 = 0.0;

  switch (el.Type()) {
   case Elem_Type_Line:
      sols = Geo_Isct_Line_Arc(el.P1(),el.P2(),lp1,lp2,cntre,lccw,
                               true,false,Vec2::IdentDist,
                               ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Arc(lp1,lp2,cntre,lccw,
                               ela.lp1,ela.lp2,ela.cntre,ela.lccw,
                               true,false,Vec2::IdentDist,
                               ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   default:
    el.Intersect_XY(*this,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();
  }

  while (sols > 0) {
    pr1a += bpar; pr2a += el.Begin_Par();

    if (!Par(pr1a,pr1a)) return; // Make it 3D Parameter
    if (!Par(pr2a,pr2a)) return;

    if (!At_Par(pr1a,ip)) return; ipa.z = ip.z;

    Tangent_At(pr1a,tg1);
    el.Tangent_At(pr2a,tg2);
 
    isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,curve1,tg2,pr2a,curve2));

    sols--;
    ipa = ipb; pr1a = pr1b; pr2a = pr2b;
  }

  Isect_Cursor begin(isect_lst);
  Isect_Cursor end(isect_lst.End());

  IT_Sort<Isect_Cursor,Isect_Cmp>::
     Sort(begin,end,Isect_Cmp());
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Intersect_XY(const Elem& el, bool tang_ok,
                                              Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (!Rect_Ax::Intersects_XY(el.Rect(),Vec2::IdentDist)) return;

  int sols = 0;
  double pr1a=0, pr2a=0, pr1b=0, pr2b=0;
  Vec3 ipa, ipb, ip, tg1, tg2;

  double curve1 = 1/R(); if (!lccw) curve1 = -curve1;
  double curve2 = 0.0;

  switch (el.Type()) {
   case Elem_Type_Line:
      sols = Geo_Isct_Line_Arc(el.P1(),el.P2(),lp1,lp2,cntre,lccw,
                               true,tang_ok,Vec2::IdentDist,
                               ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Arc(lp1,lp2,cntre,lccw,
                               ela.lp1,ela.lp2,ela.cntre,ela.lccw,
                               true,tang_ok,Vec2::IdentDist,
                               ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   default:
    el.Intersect_XY(*this,tang_ok,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();
  }

  while (sols > 0) {
    pr1a += bpar; pr2a += el.Begin_Par();

    if (!Par(pr1a,pr1a)) return; // Make it 3D Parameter
    if (!Par(pr2a,pr2a)) return;

    if (!At_Par(pr1a,ip)) return; ipa.z = ip.z;

    Tangent_At(pr1a,tg1);
    el.Tangent_At(pr2a,tg2);
 
    isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,curve1,tg2,pr2a,curve2));

    sols--;
    ipa = ipb; pr1a = pr1b; pr2a = pr2b;
  }

  Isect_Cursor begin(isect_lst);
  Isect_Cursor end(isect_lst.End());

  IT_Sort<Isect_Cursor,Isect_Cmp>::
     Sort(begin,end,Isect_Cmp());
}

/* ---------------------------------------------------------------------- */
/* ------- Normalize parameter to lie near closest endpoint ------------- */
/* ---------------------------------------------------------------------- */

static void norm_ext(const Elem_Arc& arc, double& relpar)
{
  double circum = Vec2::Pi2 * arc.R();

  if (relpar < 0.0) {
    double maxpar = relpar + circum;

    if (maxpar - arc.Len() < -relpar) relpar = maxpar;
  }
  else if (relpar > arc.Len()) {
    double minpar = relpar - circum;

    if (relpar - arc.Len() > -minpar) relpar = minpar;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Intersect "infinite" elements -------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Intersect_XY_Ext(const Elem& el, bool tang_ok,
                                              Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  int sols = 0;
  double pr1a=0, pr2a=0, pr1b=0, pr2b=0;
  Vec3 ipa, ipb, ip, tg1, tg2;

  double curve1 = 1/R(); if (!lccw) curve1 = -curve1;
  double curve2 = 0.0;

  switch (el.Type()) {
   case Elem_Type_Line:
      sols = Geo_Isct_Line_Arc(el.P1(),el.P2(),lp1,lp2,cntre,lccw,
                               false,tang_ok,Vec2::IdentDist,
                               ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Arc(lp1,lp2,cntre,lccw,
                               ela.lp1,ela.lp2,ela.cntre,ela.lccw,
                               false,tang_ok,Vec2::IdentDist,
                               ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;

      if (sols > 1) norm_ext(ela,pr2b);
      if (sols > 0) norm_ext(ela,pr2a);
    }
   break;

   default:
    el.Intersect_XY_Ext(*this,tang_ok,isect_lst);

    Isect_Cursor c(isect_lst);

    for (;c;++c) c->Swap();

    // Here sols == 0
  }

  if (sols > 1) norm_ext(*this,pr1b);
  if (sols > 0) norm_ext(*this,pr1a);

  while (sols > 0) {

    pr1a += bpar; pr2a += el.Begin_Par();

    if (!Par(pr1a,pr1a)) return; // Make it 3D Parameter
    if (!Par(pr2a,pr2a)) return;

    if (!At_Par(pr1a,ip)) return; ipa.z = ip.z;

    Tangent_At(pr1a,tg1);
    el.Tangent_At(pr2a,tg2);
 
    isect_lst.Push_Back(Isect_Pair(ipa,tg1,pr1a,curve1,tg2,pr2a,curve2));

    sols--;
    ipa = ipb; pr1a = pr1b; pr2a = pr2b;
  }

  Isect_Cursor begin(isect_lst);
  Isect_Cursor end(isect_lst.End());

  IT_Sort<Isect_Cursor,Isect_Cmp>::Sort(begin,end,Isect_Cmp());
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Offset_Into(double dist, Elem_List& offel,
                                                  bool keep_info) const
{
  offel.Delete();

  if (lccw) dist = -dist;

  double r = R();

  if (r < NumAccuracy * len_xy) return false;

  if ((-dist -r) *len_xy/r > -Vec2::IdentDist/2.0) return false;

  Vec2 d1 = lp1 - cntre; d1.unitLen2(); d1 *= dist;
  Vec2 d2 = lp2 - cntre; d2.unitLen2(); d2 *= dist;

  Elem_Arc newel(*this, keep_info); newel.p_cnt_id = cnt_id;

  newel.lp1 += d1; newel.lp2 += d2;

  newel.update();

  offel.Push_Back(newel);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Offset_Into(double dist, double minlen, Elem_List& off_lst,
                                             Vec3& strtpt, Vec3& endpt,
                                                   double& mislen) const
{
  minlen = fabs(minlen);

  off_lst.Delete();

  if (lccw) dist = -dist;

  double r = R();

  bool r_ok = r > NumAccuracy * len_xy;

  bool ok = false;

  if (r_ok) ok = (dist + r) *len_xy/r > minlen;

  Vec2 d1 = lp1 - cntre; d1.unitLen2(); d1 *= dist;
  Vec2 d2 = lp2 - cntre; d2.unitLen2(); d2 *= dist;

  Elem_Arc newel(*this,false); newel.p_cnt_id = cnt_id;

  newel.lp1 += d1; newel.lp2 += d2;

  newel.update();

  strtpt = newel.lp1;
  endpt  = newel.lp2;

  if (ok) off_lst.Push_Back(newel);

  if (r_ok) mislen = (r + dist) / r * len_xy;
  else mislen = 0.0;

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Split(double par, Elem_List& spl_lst) const
{
  spl_lst.Delete();

  double lpar = par - Begin_Par();
  Vec3 newp;

  if (lpar < Vec2::IdentDist || lpar > Len()-Vec2::IdentDist ||
                                                   !At_Par(par,newp)) {
    spl_lst.Push_Back(*this);
    return false;
  }

  Elem_Arc newarc1(*this);
  Elem_Arc newarc2(*this);

  newarc1.lp2 = newp; newarc2.lp1 = newp;

  newarc2.bpar = par;
  
  newarc1.update();
  newarc2.update();

  spl_lst.Push_Back(newarc1);
  spl_lst.Push_Back(newarc2);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Stretch_Begin_XY(const Vec2& isp, bool check)
{
  double endpar = End_Par();

  if (check && isp.distTo2(lp1) > 10.0 * Vec2::IdentDist) 
                                       Cont_Panic(Elem_Large_Stretch);

  lp1.x = isp.x;
  lp1.y = isp.y;

  Geo_Correct_Arc(lp1,lp2,cntre);

  update();

  bpar = endpar - len;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Stretch_End_XY(const Vec2& isp, bool check)
{
  if (check) {
    double dst = isp.distTo2(lp2);
    if (dst > 10.0 * Vec2::IdentDist) Cont_Panic(Elem_Large_Stretch);
  }

  lp2.x = isp.x;
  lp2.y = isp.y;

  Geo_Correct_Arc(lp1,lp2,cntre);

  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem *Elem_Arc::Merge_With(const Elem_Arc& el) const
{
  Elem *newel = NULL;

  if (lp2 != el.lp1 || lccw != el.lccw ||
                               cntre != el.cntre) return NULL;

  if (lp1 == el.lp2) {   // Insert Circle

   newel = new Elem_Circle((lp1 + el.lp2)/2.0, el.lp2,
                                      (cntre + el.cntre)/2.0, lccw);

   Elem_Circle *newcir = (Elem_Circle *)newel;

   if (Info()) newcir->Add_Info(*Info());
  }
  else { // Insert Arc

    newel = new Elem_Arc(*this); Elem_Arc *newarc = (Elem_Arc *)newel;

    newarc->lp2 = el.lp2;

    Geo_Correct_Arc(newarc->lp1, newarc->lp2, newarc->cntre);

    newarc->update();
  }

  return newel;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::Limit_Span_180(Elem_Cursor& newpair) const
{
  if (!newpair.Container()) return false;

  double span = len_xy / R();

  if (span < Vec2::Pi) return false;

  Vec3 midp;
  double midpar = bpar + plen/2.0;

  if (!At_Par(midpar,midp)) return false;

  Elem_Arc newarc2(*this);
  newarc2.lp1 = midp; newarc2.bpar = midpar; newarc2.update();
  newpair.Insert(newarc2);

  Elem_Arc newarc1(*this); newarc1.lp2 = midp; newarc1.update();
  newpair.Insert(newarc1);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Arc::Transform(const Trf2& trf)
{
  lp1.transform2(trf);
  lp2.transform2(trf);
  cntre.transform2(trf);

  if (trf.mirrors()) lccw = !lccw;

  update();
}

/* ---------------------------------------------------------------------- */

bool Elem_Arc::Fillet(const Elem& el2, double filletRad,
                                               Elem_Arc& /*fillet*/) const
{
  switch (el2.Type()) {
    case Elem_Type_Line: {
      if (filletRad < 0.0)
           return false;
      else return false;
    }

    case Elem_Type_Arc: {
      // const Elem_Arc& arc = (const Elem_Arc&) el2;
      return false;
    }

    case Elem_Type_Circle:
    default:
      return false; // Not implemented
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Arc::From_3pt(const Vec2& p1, const Vec2& p2,
                                        const Vec2& p3, double max_rad)
{
  max_rad = fabs(max_rad);

  Vec2 org(p1); org += p2; org /= 2.0;
  Vec2 dir(p2); dir -= p1;

  double dirlen = dir.len2();
  
  if (dirlen < Vec2::IdentDist) return false;

  dir /= dirlen; dirlen /= 2.0;
    
  Trf2 to_local(org,dir);
  
  Vec2 hp2(p2); hp2.transform2(to_local);
  Vec2 hp3(p3); hp3.transform2(to_local);

  Vec2 hpm(hp2); hpm += hp3; hpm /= 2.0;
  
  Vec2 norm(hp3); norm -= hp2;
  
  if (norm.len2() < Vec2::IdentDist) return false;
  
  norm.rot90();
  
  if (fabs(norm.x) < NumAccuracy * fabs(hpm.x)) return false;
  
  double a = -hpm.x/norm.x;
  double hy = hpm.y + a*norm.y;
  
  if (sqr(hy) + sqr(dirlen) > sqr(max_rad)) return false;
  
  lccw = a >= 0.0;
  
  lp1 = p1; lp2 = p3;
  
  cntre.x = 0.0; cntre.y = hy;

  Trf2 to_global;
  to_local.invertInto(to_global);
  
  cntre.transform2(to_global);
  
  bpar = 0.0;
  
  update();
  
  return true;
}

//---------------------------------------------------------------------------
// Persistable Section

static const char fldP1x[] = "P1x";
static const char fldP1y[] = "P1y";
static const char fldP2x[] = "P2x";
static const char fldP2y[] = "P2y";
static const char fldcx[]  = "cx";
static const char fldcy[]  = "cy";
static const char fldccw[] = "ccw";

//---------------------------------------------------------------------------

Elem_Arc::Elem_Arc(PersistentReader& pi)
: Elem(pi),
  lp1(pi.readDouble(fldP1x),pi.readDouble(fldP1y)),
  lp2(pi.readDouble(fldP2x),pi.readDouble(fldP2y)),
  cntre(pi.readDouble(fldcx),pi.readDouble(fldcy)),
  lccw(pi.readBool(fldccw))
{
  update();
}

//---------------------------------------------------------------------------

void Elem_Arc::definePersistentFields(PersistentWriter& po) const
{
  Elem::definePersistentFields(po);

  po.addField(fldP1x,typeid(double));
  po.addField(fldP1y,typeid(double));
  po.addField(fldP2x,typeid(double));
  po.addField(fldP2y,typeid(double));
  po.addField(fldcx,typeid(double));
  po.addField(fldcy,typeid(double));
  po.addField(fldccw,typeid(bool));
}

//---------------------------------------------------------------------------

void Elem_Arc::writePersistentObject(PersistentWriter& po) const
{
  Elem::writePersistentObject(po);

  po.writeDouble(fldP1x,lp1.x);
  po.writeDouble(fldP1y,lp1.y);
  po.writeDouble(fldP2x,lp2.x);
  po.writeDouble(fldP2y,lp2.y);
  po.writeDouble(fldcx,cntre.x);
  po.writeDouble(fldcy,cntre.y);
  po.writeBool(fldccw,lccw);
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
