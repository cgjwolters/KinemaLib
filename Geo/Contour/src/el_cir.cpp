/* ---------------------------------------------------------------------- */
/* ---------------- 3D Circle Element ----------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "El_Cir.h"

#include "El_Line.h"
#include "El_Arc.h"

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

struct store_cir
{
  store_cir *next;
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void *Elem_Circle::operator new(size_t)
{
  if (!store) return new char[sizeof(Elem_Circle)];

  void *newl = store;
  store = ((store_cir *)store)->next;

  return newl;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::operator delete(void *old_cir)
{
  ((store_cir *)old_cir)->next = (store_cir *)store;
  store = old_cir;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::CleanupStore()
{
  store_cir *fst = (store_cir *)store;

  while (fst) {
    store_cir *nxt = fst->next;

    delete[] (char *)fst;
    fst = nxt;
  }

  store = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::update()
{
  double r = R();

  len_xy = r * Vec2::Pi2;
  len = sqrt(sqr(len_xy) + sqr(lp2.z-lp1.z));
  plen = len;
  
  Vec2 edge(r,r);

  Vec3 ll(cntre,lp1.z); ll -= edge;
  Vec3 ur(cntre,lp2.z); ur += edge;

  Rect_Update(ll,ur);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle::Elem_Circle()
  : Elem(), lp1(), lp2(), cntre(), lccw(true)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle::Elem_Circle(const Vec3& p1, double z2, const Vec2& c, bool ccw)
 : Elem(), lp1(p1), lp2(p1.x,p1.y,z2), cntre(c), lccw(ccw)
{
  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle::Elem_Circle(const Vec3& p1, const Vec3& p2, const Vec2& c, bool ccw)
 : Elem(), lp1(p1), lp2(p1.x,p1.y,p2.z), cntre(c), lccw(ccw)
{
  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle::Elem_Circle(const Elem_Circle& cp)
 : Elem(cp), lp1(cp.lp1), lp2(cp.lp1.x,cp.lp1.y,cp.lp2.z),
   cntre(cp.cntre), lccw(cp.lccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle::Elem_Circle(const Elem_Circle& cp, bool keep_info)
 : Elem(cp, keep_info), lp1(cp.lp1), lp2(cp.lp1.x,cp.lp1.y,cp.lp2.z),
   cntre(cp.cntre), lccw(cp.lccw)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Circle& Elem_Circle::operator=(const Elem_Circle& src)
{
  clone_from(src);

  lp1   = src.lp1;
  lp2   = src.lp1; lp2.z = src.lp2.z;
  cntre = src.cntre;
  lccw  = src.lccw;

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

const Vec3& Elem_Circle::P2() const
{
  lp2.x = lp1.x; lp2.y = lp1.y;

  return lp2;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::At_Par(double par, Vec3& p) const
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

Vec2 Elem_Circle::Start_Tangent_XY() const
{
  Vec2 tg;
  return Start_Tangent_XY(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2 Elem_Circle::End_Tangent_XY() const
{
  Vec2 tg;
  return End_Tangent_XY(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Circle::Start_Tangent_XY(Vec2& tg) const
{
  tg = lp1; tg -= cntre; tg.unitLen2();

  if (lccw) tg.rot90();
  else      tg.rot270();
  
  return tg;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec2& Elem_Circle::End_Tangent_XY(Vec2& tg) const
{
  return Start_Tangent_XY(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Tangent_At_XY(double par, Vec2& tg) const
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

Vec3 Elem_Circle::Start_Tangent() const
{
  Vec3 tg;
  return Start_Tangent(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3 Elem_Circle::End_Tangent() const
{
  Vec3 tg;
  return End_Tangent(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Vec3& Elem_Circle::Start_Tangent(Vec3& tg) const
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

Vec3& Elem_Circle::End_Tangent(Vec3& tg) const
{
  return Start_Tangent(tg);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Tangent_At(double par, Vec3& tg) const
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

double Elem_Circle::Start_Curve() const
{
  if (lccw) return  1.0/R(); 
  else      return -1.0/R();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Circle::End_Curve() const
{
  if (lccw) return  1.0/R(); 
  else      return -1.0/R();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Curve_At(double /*par*/, double& curve) const
{
  if (lccw) curve =  1.0/R(); 
  else      curve = -1.0/R();

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Circle::Area_XY_P1() const
{
  double area = Vec2::Pi * sqr(R());

  if (lccw) return  area;
  else      return -area;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Circle::Moment_X() const
{
  double mom = Vec2::Pi * sqr(R())*cntre.x;
  
  if (!lccw) mom = -mom;
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Circle::Moment_Y() const
{
  double mom = Vec2::Pi * sqr(R())*cntre.y;
  
  if (!lccw) mom = -mom;
  
  return mom;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Circle::Span_Angle() const
{
  if (lccw) return  Vec2::Pi2;
  else      return -Vec2::Pi2;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Project_Pnt_XY(const Vec2& p, double tol, bool /*strict*/,
                               Vec3& pp,
                               double& parm, double &dist_xy) const
{
  Geo_Project_P_on_Arc(p,lp1,lp2,cntre,lccw,false,tol,pp,parm,dist_xy);

  bool ok = true;

  parm += bpar;
  if (!Par(parm,parm)) return false;

  Vec3 ip;
  if (!At_Par(parm,ip)) ok = false;
  else pp.z = ip.z;

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Project_Pnt_Strict_XY(const Vec2& p, double tol,
                                        Vec3& pp,
                                        double& parm, double &dist_xy) const
{
  Geo_Project_P_on_Arc(p,lp1,lp2,cntre,lccw,false,tol,pp,parm,dist_xy);

  bool ok = true;

  parm += bpar;
  if (!Par(parm,parm)) return false;

  Vec3 ip;
  if (!At_Par(parm,ip)) ok = false;
  else pp.z = ip.z;

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::Intersect_XY(const Elem& el, Isect_Lst& isect_lst) const
{
  isect_lst.Delete();

  if (!Rect_Ax::Intersects_XY(el.Rect(),Vec2::IdentDist)) return;

  int sols = 0;
  double pr1a=0, pr2a=0, pr1b=0, pr2b=0;
  Vec3 ipa, ipb, ip, tg1, tg2;

  double curve1 = 1/R(); if (!lccw) curve1 = -curve1;
  double curve2 = 0.0;

// Hier: Intersect moet nog tangok == true hebben

  switch (el.Type()) {
   case Elem_Type_Line:
      sols = Geo_Isct_Line_Circle(el.P1(),el.P2(),lp1,cntre,lccw,
                                  true,false,2.0*Vec2::IdentDist,
//                                  true,false,Vec2::Ident_Dist,
                                  ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Circle(ela.P1(),ela.P2(),ela.C(),ela.Ccw(),
                                 lp1,cntre,lccw,
                                 true,false,2.0*Vec2::IdentDist,
//                                 true,false,Vec2::Ident_Dist,
                                 ipa,ipb,pr2a,pr2b,pr1a,pr1b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   case Elem_Type_Circle: {
      const Elem_Circle& elc = (const Elem_Circle&) el;

      sols = Geo_Isct_Circle_Circle(lp1,cntre,lccw,
                                    elc.P1(),elc.C(),elc.Ccw(),
                                    false,2.0*Vec2::IdentDist,
//                                    false,Vec2::Ident_Dist,
                                    ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/elc.R(); if (!lccw) curve2 = -curve2;
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

void Elem_Circle::Intersect_XY(const Elem& el, bool tang_ok,
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
      sols = Geo_Isct_Line_Circle(el.P1(),el.P2(),lp1,cntre,lccw,
                                  true,tang_ok,Vec2::IdentDist,
                                  ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Circle(ela.P1(),ela.P2(),ela.C(),ela.Ccw(),
                                 lp1,cntre,lccw,
                                 true,tang_ok,Vec2::IdentDist,
                                 ipa,ipb,pr2a,pr2b,pr1a,pr1b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   case Elem_Type_Circle: {
      const Elem_Circle& elc = (const Elem_Circle&) el;

      sols = Geo_Isct_Circle_Circle(lp1,cntre,lccw,
                                    elc.P1(),elc.C(),elc.Ccw(),
                                    tang_ok,Vec2::IdentDist,
                                    ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/elc.R(); if (!lccw) curve2 = -curve2;
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
/* ------- Intersect "infinite" elements -------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::Intersect_XY_Ext(const Elem& el, bool tang_ok,
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
      sols = Geo_Isct_Line_Circle(el.P1(),el.P2(),lp1,cntre,lccw,
                                  false,tang_ok,Vec2::IdentDist,
                                  ipa,ipb,pr2a,pr2b,pr1a,pr1b);
   break;

   case Elem_Type_Arc: {
      const Elem_Arc& ela = (const Elem_Arc&) el;

      sols = Geo_Isct_Arc_Circle(ela.P1(),ela.P2(),ela.C(),ela.Ccw(),
                                 lp1,cntre,lccw,
                                 false,tang_ok,Vec2::IdentDist,
                                 ipa,ipb,pr2a,pr2b,pr1a,pr1b);

      curve2 = 1/ela.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   case Elem_Type_Circle: {
      const Elem_Circle& elc = (const Elem_Circle&) el;

      sols = Geo_Isct_Circle_Circle(lp1,cntre,lccw,
                                    elc.P1(),elc.C(),elc.Ccw(),
                                    tang_ok,Vec2::IdentDist,
                                    ipa,ipb,pr1a,pr1b,pr2a,pr2b);

      curve2 = 1/elc.R(); if (!lccw) curve2 = -curve2;
    }
   break;

   default:
    el.Intersect_XY_Ext(*this,tang_ok,isect_lst);

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

bool Elem_Circle::Offset_Into(double dist, Elem_List& offel,
                                                 bool keep_info) const
{
  offel.Delete();

  if (lccw) dist = -dist;

  if ((-dist -R()) * Vec2::Pi2 > -Vec2::IdentDist/2.0) return false;

  Vec2 d1 = lp1 - cntre; d1.unitLen2(); d1 *= dist;

  Elem_Circle newel(*this, keep_info); newel.p_cnt_id = cnt_id;

  newel.lp1 += d1; newel.lp2.x = newel.lp1.x; newel.lp2.y = newel.lp1.y;

  newel.update();

  offel.Push_Back(newel);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Offset_Into(double dist, double minlen,
                              Elem_List& off_lst, Vec3& strtpt, Vec3& endpt,
                                                   double& mislen) const
{
  minlen = fabs(minlen);

  off_lst.Delete();

  if (lccw) dist = -dist;

  double r = R();

  bool ok = (dist + r) * Vec2::Pi2 > minlen;

  Vec2 d1 = lp1 - cntre; d1.unitLen2(); d1 *= dist;

  Elem_Circle newel(*this, false); newel.p_cnt_id = cnt_id;

  newel.lp1 += d1; newel.lp2.x = newel.lp1.x; newel.lp2.y = newel.lp1.y;

  newel.update();

  strtpt = newel.lp1;
  endpt  = newel.lp2;

  if (ok) off_lst.Push_Back(newel);

  mislen = (r + dist) * Vec2::Pi2;

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Split(double par, Elem_List& spl_lst) const
{
  double lpar = par - bpar;
  Vec3 newp;

  if (lpar < Vec2::IdentDist || lpar > len-Vec2::IdentDist || 
                                                  !At_Par(par,newp)) {
    spl_lst.Push_Back(*this);
    return false;
  }


  Elem_Arc newarc1(lp1,newp,cntre,lccw);
  Elem_Arc newarc2(newp,lp2,cntre,lccw);

  newarc1.Begin_Par(bpar);
  newarc1.Id(el_id); newarc1.Cnt_Id(cnt_id); newarc1.P_Cnt_Id(p_cnt_id);
  newarc1.Cam_Inf(Cam_Inf());

  newarc2.Begin_Par(par);
  newarc2.Id(el_id); newarc2.Cnt_Id(cnt_id); newarc2.P_Cnt_Id(p_cnt_id);
  newarc2.Cam_Inf(Cam_Inf());

  const Elem_Info *info = Info();
  if (info) {
    newarc1.Add_Info(*info);
    newarc2.Add_Info(*info);
  }

  spl_lst.Push_Back(newarc1);
  spl_lst.Push_Back(newarc2);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::Stretch_Begin_XY(const Vec2& isp, bool check)
{
  double endpar = End_Par();

  if (check && isp.distTo2(lp1) > 10.0 * Vec2::IdentDist) 
                                       Cont_Panic(Elem_Large_Stretch);

  lp1.x = isp.x;
  lp1.y = isp.y;

  lp2.x = isp.x;
  lp2.y = isp.y;

  update();

  bpar = endpar - len;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::Stretch_End_XY(const Vec2& isp, bool check)
{
  if (check && isp.distTo2(lp1) > 10.0 * Vec2::IdentDist) 
                                       Cont_Panic(Elem_Large_Stretch);

  lp1.x = isp.x;
  lp1.y = isp.y;

  lp2.x = isp.x;
  lp2.y = isp.y;

  update();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Circle::Limit_Span_180(Elem_Cursor& newpair) const
{
  if (!newpair.Container()) return false;

  Vec3 midp;
  double midpar = bpar + plen/2.0;

  if (!At_Par(midpar,midp)) return false;

  Elem_Arc newarc2(midp,lp2,cntre,lccw);
  newarc2.Id(el_id); newarc2.Cnt_Id(cnt_id); newarc2.P_Cnt_Id(p_cnt_id);
  newarc2.Cam_Inf(Cam_Inf());
  newarc2.Begin_Par(midpar);
  if (Info()) newarc2.Add_Info(*Info());

  newpair.Insert(newarc2);

  Elem_Arc newarc1(lp1,midp,cntre,lccw);
  newarc1.Id(el_id); newarc1.Cnt_Id(cnt_id); newarc1.P_Cnt_Id(p_cnt_id);
  newarc1.Cam_Inf(Cam_Inf());
  newarc1.Begin_Par(bpar);
  if (Info()) newarc1.Add_Info(*Info());

  newpair.Insert(newarc1);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Circle::Transform(const Trf2& trf)
{
  lp1.transform2(trf);
  lp2.transform2(trf); lp2.x = lp1.x; lp2.y = lp1.y;
  cntre.transform2(trf);

  if (trf.mirrors()) lccw = !lccw;

  update();
}

/* ---------------------------------------------------------------------- */

bool Elem_Circle::Fillet(const Elem& /*el2*/, double /*filletRad*/,
                                                Elem_Arc& /*fillet*/) const
{
  return false; // Not implemented
}

//---------------------------------------------------------------------------
// Persistable Section

static const char fldP1x[] = "P1x";
static const char fldP1y[] = "P1y";
static const char fldcx[]  = "cx";
static const char fldcy[]  = "cy";
static const char fldccw[] = "ccw";

//---------------------------------------------------------------------------

Elem_Circle::Elem_Circle(PersistentReader& pi)
: Elem(pi),
  lp1(pi.readDouble(fldP1x),pi.readDouble(fldP1y)),
  lp2(lp1.x,lp1.y),
  cntre(pi.readDouble(fldcx),pi.readDouble(fldcy)),
  lccw(pi.readBool(fldccw))
{
  update();
}

//---------------------------------------------------------------------------

void Elem_Circle::definePersistentFields(PersistentWriter& po) const
{
  Elem::definePersistentFields(po);

  po.addField(fldP1x,typeid(double));
  po.addField(fldP1y,typeid(double));
  po.addField(fldcx,typeid(double));
  po.addField(fldcy,typeid(double));
  po.addField(fldccw,typeid(bool));
}

//---------------------------------------------------------------------------

void Elem_Circle::writePersistentObject(PersistentWriter& po) const
{
  Elem::writePersistentObject(po);

  po.writeDouble(fldP1x,lp1.x);
  po.writeDouble(fldP1y,lp1.y);
  po.writeDouble(fldcx,cntre.x);
  po.writeDouble(fldcy,cntre.y);
  po.writeBool(fldccw,lccw);
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
