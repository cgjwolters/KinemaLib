/* ---------------------------------------------------------------------- */
/* ---------------- 2D Element Geometry Calculations -------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1994 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "Geo.h"

#include <math.h>

#include "Trf.h"

namespace Ino
{

/* --------------------------------------------------------------------- */
/* ------------ Correct centre of arc such that both endpoints lie------ */
/* ------------ exactly on the arc (radius is taken as dist(p1<->c)) --- */
/* --------------------------------------------------------------------- */

void Geo_Correct_Arc(const Vec2& p1, const Vec2& p2, Vec2& c)
{
  Vec2 dir = p2 - p1;
  double len = dir.len2();

  if (len < Vec2::IdentDist) return;

  dir /= len;

  Trf2 to_local(p1,dir); Trf2 to_global; to_local.invertInto(to_global);

  double rad = (p1.distTo2(c) + p2.distTo2(c))/2;

  Vec2 oldc = to_local * c;

  Vec2 lc = to_local * p2; lc /= 2;

  double sqrtarg = sqr(rad) - sqr(lc.x);

  if (sqrtarg <= 0) lc.y = 0;
  else lc.y = sqrt(sqrtarg);

  if (oldc.y < 0) lc.y = -lc.y;

  c = to_global * lc;
}

/* --------------------------------------------------------------------- */
/* ------------ Normalize angle ---------------------------------------- */
/* --------------------------------------------------------------------- */

double Geo_Norm_Angle(bool acw, double a)
{
  if (a < 0.0) a = -fmod(-a,Vec2::Pi2); 
  else a = fmod(a,Vec2::Pi2);

  if (acw) {
    if (a < 0.0) a += Vec2::Pi2;
  }
  else {
    if (a > 0.0) a -= Vec2::Pi2;
  }

  return a;
}

/* --------------------------------------------------------------------- */
/* ----------- Calculate arcspan of arc in radians (acw = positive) ---- */
/* --------------------------------------------------------------------- */

double Geo_Arc_Span(const Vec2& p1, const Vec2& p2, const Vec2& centre,
                                                              bool acw)
{
  Vec2 dir1 = p1 - centre;
  Vec2 dir2 = p2 - centre;

  double ang = Geo_Norm_Angle(acw,dir1.angleTo2(dir2));

  return ang;
}

/* --------------------------------------------------------------------- */
/* ----------- Calculate arclength of arc ------------------------------ */
/* --------------------------------------------------------------------- */

double Geo_Arc_Len(const Vec2& p1, const Vec2& p2, const Vec2& centre,
                                                              bool acw)
{
  Vec2 dir1 = p1 - centre;
  Vec2 dir2 = p2 - centre;
  double r  = p1.distTo2(centre);

  double ln = Geo_Norm_Angle(acw,dir1.angleTo2(dir2)) * r;
  if (ln < 0) ln = -ln;

  return ln;
}

/* --------------------------------------------------------------------- */
/* -------- Find point on arc halfway between start- and endpoint ------ */
/* --------------------------------------------------------------------- */

Vec2 Geo_Arc_Mid(const Vec2& p1, const Vec2& p2, const Vec2& centre,
                                                              bool acw)
{
  double r = p1.distTo2(centre);

  Vec2 dp = p2 - p1;

  if (dp.len2() < r) {
    Vec2 dir1 = p1 - centre;
    Vec2 dir2 = p2 - centre;

    dp = p1 + p2; dp.unitLen2(); dp *= r;

    if (!acw == Geo_In_Between(dir1,dir2,dp)) dp.rot180();
  }
  else {
    dp.unitLen2(); dp *= r;

    if (acw) dp.rot270();
    else     dp.rot90();
  }

  dp += centre;

  return dp;
}

/* --------------------------------------------------------------------- */
/* ------------ Is c in between a and b in acw direction? -------------- */
/* --------------------------------------------------------------------- */

bool Geo_In_Between(const Vec2& a, const Vec2& b, const Vec2& c)
{
  bool ab = a.x*b.y - a.y*b.x >= 0.0;
  bool ac = a.x*c.y - a.y*c.x >= 0.0;
  bool cb = c.x*b.y - c.y*b.x >= 0.0;

  if (ab) return (ac && cb);
  else    return (ac || cb);
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

static void correct_parm(double& pr, double circum, double tol)
{
  double pr2 = pr - circum;

  if (pr2 >= -tol) pr = pr2;
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

static bool check_strict(double len1, double par1,
                         double len2, double par2, double tol)
{
  if (par1 < -tol || par1 > len1 + tol) return false;
  if (par2 < -tol || par2 > len2 + tol) return false;

  return true;
}

/* --------------------------------------------------------------------- */
/* ------------ Project Point on a Line -------------------------------- */
/* --------------------------------------------------------------------- */

bool Geo_Project_P_on_Line(const Vec2& p, const Vec2& s, const Vec2& e,
                           bool  strict, double tol,
                           Vec2& pp, double& pr, double& dist)
{
  Vec2 dir = e - s;
  double len = dir.len2();

  if (len < tol) {                 // (Almost) Zero length line
    pp = (s + e) / 2.0;            // No accurate direction

    pr = 0.5 * len;

    Vec2 dp = p - pp;
    dist = dp.len2();
 
    dir.rot90();
    if (dp * dir < 0) dist = -dist;

    return false;
  }
  else {
    dir /= len;

    // Determine transform such that line is along x-axis starting in origin

    Trf2 to_local(s,dir); Trf2 to_global; to_local.invertInto(to_global);

    // Transform to local coordinate system

    pp = to_local * p; pr = pp.x; dist = pp.y; pp.y = 0;

    // Transform back to global
    pp = to_global * pp;

    if (strict && (pr < -tol || pr > len+tol)) return false;

    return true;
  }
}

/* --------------------------------------------------------------------- */
/* ------------ Project Point on an Arc -------------------------------- */
/* --------------------------------------------------------------------- */

bool Geo_Project_P_on_Arc(const Vec2& p, const Vec2& s, const Vec2& e,
                          const Vec2& c, bool acw,
                          bool  strict, double tol,
                          Vec2& pp, double& pr, double& dist)
{
  double rad = s.distTo2(c);
  
  double ex_dist = p.distTo2(c);

  if (ex_dist < Vec2::IdentDist) {
    pp = s;
    pr = 0.0;
    dist = ex_dist - rad; if (acw) dist = -dist;
    return false;
  }
  
  pp = p - c; pp *= (rad/ex_dist); pp += c;

  dist = ex_dist - rad; if (acw) dist = -dist;

  pr = Geo_Arc_Len(s,pp,c,acw);

  if (strict) {
    double arclen = Geo_Arc_Len(s,e,c,acw);

    double circum = Vec2::Pi2 * rad;
    correct_parm(pr,circum,tol);

    if (pr < -tol || pr > arclen + tol) return false;
  }

  return true;
}

/* --------------------------------------------------------------------- */
/* ------------ Intersect two lines (help routine) --------------------- */
/* --------------------------------------------------------------------- */

static int Isct_Line(const Vec2& b, const Vec2& dir, double len,
                     const Vec2& s, const Vec2& e, double selen,
                     bool strict, double tol,
                     Vec2& ip, double& pr1, double& pr2)
{
  if (tol < NumAccuracy) tol = NumAccuracy;

  Trf2 to_local(b,dir);
  Trf2 to_global; to_local.invertInto(to_global);

  Vec2 ls = to_local * s; Vec2 le = to_local * e;

  Vec2 d = le - ls;

  // Assume a relative accurary on the input data (const Num_Accuracy)
  // tangent conditions are reached when the accurary in ip
  // exceeds tol, this is the case when ...

 // double abssum = fabs(d.x) + fabs(d.y);
  if (fabs(d.y) < 1e-12) return 0;

  // if (fabs(d.y) < Vec2::Ident_Dist ||
  //              sqr(d.y) <= 4.0 * Num_Accuracy * abssum/tol) return 0;

  // Not tangent, so determine intersection

  ip.x = ls.x - ls.y * d.x / d.y; ip.y = 0;

  pr1 = ip.x;
  Vec2 dp = ip - ls; pr2 = dp * d/selen;

  ip = to_global * ip;

  if (strict && !check_strict(len,pr1,selen,pr2,tol)) return 0;

  return 1;
}
/* --------------------------------------------------------------------- */
/* ------------ Intersect two lines ------------------------------------ */
/* --------------------------------------------------------------------- */

int Geo_Isct_Lines(const Vec2& s1, const Vec2& e1,
                    const Vec2& s2, const Vec2& e2,
                    bool strict, double tol,
                    Vec2& ip, double& pr1, double& pr2)
{
  Vec2 dir1 = e1 - s1; double len1 = dir1.len2(); dir1 /= len1;
  Vec2 dir2 = e2 - s2; double len2 = dir2.len2(); dir2 /= len2;

  if (len1 > len2)
       return Isct_Line(s1,dir1,len1,s2,e2,len2,strict,tol,ip,pr1,pr2);
  else return Isct_Line(s2,dir2,len2,s1,e1,len1,strict,tol,ip,pr2,pr1);
}

/* --------------------------------------------------------------------- */
/* ------------ Intersect a line with an arc --------------------------- */
/* --------------------------------------------------------------------- */

int Geo_Isct_Line_Arc(const Vec2& s1, const Vec2& e1,
                       const Vec2& s2, const Vec2& e2,
                       const Vec2& c2, bool acw,
                       bool strict, bool tangok, double tol,
                       Vec2& ipa, Vec2& ipb,
                       double& pr1a, double& pr1b,
                       double& pr2a, double& pr2b)
{
  int sols = 0;

  Vec2 dir1 = e1 - s1; double len1 = dir1.len2();

  if (len1 < tol) return 0;

  dir1 /= len1;

  double rad = s2.distTo2(c2);
  double len2 = Geo_Arc_Len(s2, e2, c2, acw);

  Trf2 to_local(s1,dir1); Trf2 to_global; to_local.invertInto(to_global);

  Vec2 ls2, le2, lc2;

  ls2 = to_local * s2; le2 = to_local * e2; lc2 = to_local * c2;

  double c2_dist = fabs(lc2.y);

  if (c2_dist > rad) {        // Tangent solution or else none
    if (!tangok || c2_dist > rad+2.0*tol) return 0;

    double dist = (c2_dist - rad)/2.0;
    
    ipa.y = (c2.y < 0 ? -dist : dist);
    ipa.x = lc2.x;
    ipb = ipa;

    sols = 1;
  }
  else {
    double dist;

    double sqrtarg = sqr(rad) - sqr(lc2.y);

    if (sqrtarg <= 0) dist = 0;
    else              dist = sqrt(sqrtarg);

    ipa.y = 0; ipb.y = 0;

    if (dist < tol) {
      ipa.x = ipb.x = lc2.x;
      sols = 1;
      if (!tangok) sols = 0;
    }
    else {
      ipa.x = lc2.x - dist; ipb.x = lc2.x + dist;
      sols = 2;
    }
  }

  pr1a = ipa.x; pr1b = ipb.x;

  pr2a = Geo_Arc_Len(ls2,ipa,lc2,acw);
  pr2b = Geo_Arc_Len(ls2,ipb,lc2,acw);

  ipa = to_global * ipa;
  ipb = to_global * ipb;

  if (pr1a > pr1b) {
    double hold;
    hold = pr1a; pr1a = pr1b; pr1b = hold;
    hold = pr2a; pr2a = pr2b; pr2b = hold;

    Vec2 vhold = ipa; ipa = ipb; ipb = vhold;
  }

  if (strict) {
    double circum = Vec2::Pi2 * rad;

    correct_parm(pr2a,circum,tol);
    correct_parm(pr2b,circum,tol);

    if (sols > 1 && !check_strict(len1,pr1b,len2,pr2b,tol)) sols--;
    if (sols > 0 && !check_strict(len1,pr1a,len2,pr2a,tol)) {
      sols--;
      if (sols > 0) {
        pr1a = pr1b; pr2a = pr2b; ipa = ipb;
      }
    }
  }

  return sols;
}

/* --------------------------------------------------------------------- */
/* ---------- Intersect an arc with an arc ----------------------------- */
/* --------------------------------------------------------------------- */

int Geo_Isct_Arc_Arc(const Vec2& s1, const Vec2& e1,
                      const Vec2& c1, bool acw1,
                      const Vec2& s2, const Vec2& e2,
                      const Vec2& c2, bool acw2,
                      bool strict, bool tangok, double tol,
                      Vec2& ipa, Vec2& ipb,
                      double& pr1a, double& pr1b,
                      double& pr2a, double& pr2b)
{
  int sols = 0;

  Vec2 dir = c2 - c1;
  
  double len = dir.len2();
  if (len < tol) return 0;

  dir /= len;

  Trf2 to_local(c1,dir); Trf2 to_global; to_local.invertInto(to_global);

  Vec2 ls1, le1, ls2, le2, lc2;

  ls1 = to_local * s1; le1 = to_local * e1;
  ls2 = to_local * s2; le2 = to_local * e2;

  lc2 = Vec2(len,0);

  double rad1 = ls1.len2(); double rad2 = ls2.distTo2(lc2);

  if (len < rad1) {  // c2 inside circle 1
    double dr = rad1 - rad2;

    if (dr >= len || dr <= -len) {
      if (!tangok || dr > len + tol || dr < -len - tol) return 0;

      if (dr > 0) ipa.x =  (rad1 + rad2 + len)/2.0;
      else        ipa.x = -(rad1 + rad2 - len)/2.0;
      ipa.y = 0;

      ipb = ipa;

      sols = 1;
    }
  }
  else {                      // c2 outside circle 1
    double dr = rad1 - rad2;
    double sr = rad1 + rad2;

    if (sr <= len || dr <= -len) {
     if (!tangok || sr < len - tol || dr < -len - tol) return 0;

     if (rad2 > len) ipa.x = -(rad1 + rad2 - len)/2.0;
     else            ipa.x =  (rad1 + len - rad2)/2.0;

     ipa.y = 0;

     ipb = ipa;

     sols = 1;
    }
  }

  if (sols < 1) {              // Solution is apparently not tangent, so
    ipa.x = ((sqr(rad1) - sqr(rad2))/len + len) / 2.0;
    ipb.x = ipa.x;

    double dist;

    double sqrtarg = sqr(rad1) - sqr(ipa.x);
    if (sqrtarg <= 0) dist = 0.0;
    else              dist = sqrt(sqrtarg);

    if (dist < tol) {
      ipa.y = 0; ipb.y = 0;
      sols = 1;
      if (!tangok) sols = 0;
    }
    else {
      ipa.y = dist; ipb.y = -dist;
      sols = 2;
    }
  }

  ipa = to_global * ipa;
  ipb = to_global * ipb;

  pr1a = Geo_Arc_Len(s1,ipa,c1,acw1);
  pr1b = Geo_Arc_Len(s1,ipb,c1,acw1);

  pr2a = Geo_Arc_Len(s2,ipa,c2,acw2);
  pr2b = Geo_Arc_Len(s2,ipb,c2,acw2);

  if (pr1a > pr1b) {
    double hold;
    hold = pr1a; pr1a = pr1b; pr1b = hold;
    hold = pr2a; pr2a = pr2b; pr2b = hold;

    Vec2 vhold = ipa; ipa = ipb; ipb = vhold;
  }

  if (strict) {
    double len1 = Geo_Arc_Len(s1,e1,c1,acw1);
    double len2 = Geo_Arc_Len(s2,e2,c2,acw2);

    double circum1 = Vec2::Pi2 * rad1;
    double circum2 = Vec2::Pi2 * rad2;

    correct_parm(pr1a,circum1,tol);
    correct_parm(pr1b,circum1,tol);

    correct_parm(pr2a,circum2,tol);
    correct_parm(pr2b,circum2,tol);

    if (sols > 1 && !check_strict(len1,pr1b,len2,pr2b,tol)) sols--;
    if (sols > 0 && !check_strict(len1,pr1a,len2,pr2a,tol)) {
      sols--;
      if (sols > 0) {
        pr1a = pr1b; pr2a = pr2b; ipa = ipb;
      }
    }
  }
  return sols;
}

/* --------------------------------------------------------------------- */
/* ------------ Intersect a line with a circle ------------------------- */
/* --------------------------------------------------------------------- */

int Geo_Isct_Line_Circle(const Vec2& s1, const Vec2& e1,
                         const Vec2& s2, const Vec2& c2, bool acw2,
                         bool strict, bool tangok, double tol,
                         Vec2& ipa, Vec2& ipb,
                         double& pr1a, double& pr1b,
                         double& pr2a, double& pr2b)
{
  Vec2 mp = c2 * 2; mp -= s2;

  int sols = Geo_Isct_Line_Arc(s1,e1,s2,mp,c2,acw2,false,tangok,tol,
                                           ipa,ipb,pr1a,pr1b,pr2a,pr2b);

  if (strict) {
    double len1 = s1.distTo2(e1);
    double len2 = s2.distTo2(c2) * Vec2::Pi2;

    if (sols > 1 && !check_strict(len1,pr1b,len2,pr2b,tol)) sols--;
    if (sols > 0 && !check_strict(len1,pr1a,len2,pr2a,tol)) {
      sols--;
      if (sols > 0) {
        pr1a = pr1b; pr2a = pr2b; ipa = ipb;
      }
    }
  }

  return sols;
}

/* --------------------------------------------------------------------- */
/* ---------- Intersect an arc with a circle --------------------------- */
/* --------------------------------------------------------------------- */

int Geo_Isct_Arc_Circle(const Vec2& s1, const Vec2& e1,
                        const Vec2& c1, bool acw1,
                        const Vec2& s2, const Vec2& c2, bool acw2,
                        bool strict, bool tangok, double tol,
                        Vec2& ipa, Vec2& ipb,
                        double& pr1a, double& pr1b,
                        double& pr2a, double& pr2b)
{
  Vec2 mp = c2 * 2; mp -= s2;

  int sols = Geo_Isct_Arc_Arc(s1,e1,c1,acw1,s2,mp,c2,acw2,false,tangok,tol,
                                              ipa,ipb,pr1a,pr1b,pr2a,pr2b);

  if (strict) {
    double len1 = Geo_Arc_Len(s1,e1,c1,acw1);
    double len2 = s2.distTo2(c2) * Vec2::Pi2;

    if (sols > 1 && !check_strict(len1,pr1b,len2,pr2b,tol)) sols--;
    if (sols > 0 && !check_strict(len1,pr1a,len2,pr2a,tol)) {
      sols--;
      if (sols > 0) {
        pr1a = pr1b; pr2a = pr2b; ipa = ipb;
      }
    }
  }
  return sols;
}

/* --------------------------------------------------------------------- */
/* ---------- Intersect two circles ------------------------------------ */
/* --------------------------------------------------------------------- */

int Geo_Isct_Circle_Circle(const Vec2& s1, const Vec2& c1, bool acw1,
                           const Vec2& s2, const Vec2& c2, bool acw2,
                           bool tangok, double tol,
                           Vec2& ipa, Vec2& ipb,
                           double& pr1a, double& pr1b,
                           double& pr2a, double& pr2b)
{
  Vec2 mp1 = c1 * 2; mp1 -= s1;
  Vec2 mp2 = c2 * 2; mp2 -= s2;

  int sols = Geo_Isct_Arc_Arc(s1,mp1,c1,acw1,s2,mp2,c2,acw2,false,tangok,tol,
                                              ipa,ipb,pr1a,pr1b,pr2a,pr2b);

  return sols;
}

} // namespace Ino

/* --------------------------------------------------------------------- */
/* ------------------- End of module ----------------------------------- */
/* --------------------------------------------------------------------- */
