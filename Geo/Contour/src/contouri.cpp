/* ---------------------------------------------------------------------- */
/* ---------------- Element Lists & Closed/Open Contours ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------- Internal Support Functions -------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "contouri.hi"
#include "cntpanic.hi"
#include <math.h>

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Closed Contours ---------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Pt_Inside(const Vec2& p, const Cont_Clsd& cnt, bool& on_cnt)
{
  on_cnt = false;

  if (cnt.Rect().Point_Inside(p,Vec2::IdentDist)) {
    Cont_Pnt pnt;
    double   dist;

    cnt.Project_Pnt_XY(p,pnt,dist);

    bool inside = (dist > 0.0) == cnt.Ccw();

    if (fabs(dist) <= Vec2::IdentDist) on_cnt = true;

    return inside;
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ------- Is contour inside other contour? ----------------------------- */
/* ------- (Contours must be closed && not intersecting !!!) ------------ */
/* ---------------------------------------------------------------------- */

bool Cnt_Inside(const Contour& cnt1, const Cont_Clsd& cnt2,
                                            const Contour* &colinear)
{
  colinear = NULL;

  Elem_C_Cursor elc1(cnt1);

  if (!elc1 || cnt2.Empty()) return false;

  while (elc1) {
    bool on_cnt = true;
    Vec2 p1 = elc1->El().P1();

    bool inside = Pt_Inside(p1,cnt2,on_cnt);

    if (!on_cnt) return inside;

    ++elc1;
  }

  // Now check the reverse

  Elem_C_Cursor elc2(cnt2);

  while (elc2) {
    Vec2 p1 = elc2->El().P1();

    Cont_Pnt pnt; double dist;
    if (!cnt1.Project_Pnt_XY(p1,pnt,dist))
                                 Cont_Panic(Conti_Cant_Project);

    if (fabs(dist) > Vec2::IdentDist) {
      bool on_cnt = true;
      bool inside = Pt_Inside(pnt.P(),cnt2,on_cnt);

      if (!on_cnt) return inside;
    }

    ++elc2;
  }

  // Check midpoints of cnt1

  elc1.To_Begin();

  while (elc1) {
    const Elem& el = elc1->El();
    Vec3 pm;
    
    if (!el.At_Par(el.Begin_Par()+el.Par_Len()/2.0,pm))
                                       Cont_Panic(Conti_No_Midpoint);
    bool on_cnt = true;
    bool inside = Pt_Inside(pm,cnt2,on_cnt);

    if (!on_cnt) return inside;

    ++elc1;
  }

  // Check midpoints of cnt2

  elc2.To_Begin();

  while (elc2) {
    const Elem& el = elc2->El();
    Vec3 pm;
    
    if (!el.At_Par(el.Begin_Par()+el.Par_Len()/2.0,pm))
                                       Cont_Panic(Conti_No_Midpoint);
    Cont_Pnt pnt; double dist;
    if (!cnt1.Project_Pnt_XY(pm,pnt,dist))
                                 Cont_Panic(Conti_Cant_Project);


    if (fabs(dist) > Vec2::IdentDist) {
      bool on_cnt = true;
      bool inside = Pt_Inside(pnt.P(),cnt2,on_cnt);

      if (!on_cnt) return inside;
    }

    ++elc2;
  }

  // As a last resort

  double len1 = cnt1.Len_XY();
  double len2 = cnt2.Len_XY();

  // Still no resolution, use area as a check

  double mean_dist  = (fabs(cnt1.Area_XY()) - fabs(cnt2.Area_XY()));
         mean_dist /= ((len1 + len2)/2.0);

  if (fabs(mean_dist) <= Vec2::IdentDist) {
    colinear = &(const Contour &)cnt2;
    return false;
  }

  return mean_dist < 0.0;  // This may not always give the right answer!
                           // Needs further elaboration (especially if
                           // elements other than lines, arcs and circles 
                           // are present!)
}

/* ---------------------------------------------------------------------- */
/* ------- Nested Contours ---------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Pt_Inside(const Vec2& p, const Cont_Nest& nest,
                                bool& on_cnt, Cont_Clsd_C_Cursor& closest)
{
  closest = Cont_Clsd_C_Cursor();
  on_cnt = false;

  Cont_Clsd_C_Cursor curcnt(nest);

  if (!curcnt) return false;

  bool inside = Pt_Inside(p,*curcnt,on_cnt);

  if (on_cnt) closest = curcnt;
    
  if (!inside) return false;

  while (++curcnt) {
    bool cur_on_cnt = false;

    inside = Pt_Inside(p,*curcnt,cur_on_cnt);

    if (cur_on_cnt && !on_cnt) {
      on_cnt = true;
      closest = curcnt;
    }

    if (inside) return false;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ------- Is contour inside nest --------------------------------------- */
/* ------- (Contour must be closed && not intersect nest!!!) ------------ */
/* ---------------------------------------------------------------------- */

bool Cnt_Inside(const Contour& cnt, const Cont_Nest& nest,
                                              const Contour* &colinear)
{
  colinear = NULL;

  Cont_Clsd_C_Cursor cc(nest);

  if (!cc || cnt.Empty()) return false;

  if (!Cnt_Inside(cnt,*cc,colinear) || colinear) return false;

  while (++cc) {
    if (Cnt_Inside(cnt,*cc,colinear) || colinear) return false;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ------- Contour Areas ------------------------------------------------ */
/* ---------------------------------------------------------------------- */

bool Pt_Inside(const Vec2& p, const Cont_Area& ar,
                                bool& on_cnt, Cont_Clsd_C_Cursor& closest)
{
  Cont_Nest_C_Cursor curnest(ar);

  closest = Cont_Clsd_C_Cursor();
  on_cnt = false;

  if (!curnest) return false;

  Cont_Clsd_C_Cursor curcnt;

  for (;curnest;++curnest) {
    bool cur_on_cnt;

    bool inside = Pt_Inside(p,*curnest,cur_on_cnt,curcnt);

    if (cur_on_cnt && !on_cnt) {
      on_cnt = true;
      closest = curcnt;
    }

    if (inside) return true;
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ------- Is contour inside area --------------------------------------- */
/* ------- (Contour must be closed && not intersect area!!!) ------------ */
/* ---------------------------------------------------------------------- */

bool Cnt_Inside(const Contour& cnt, const Cont_Area& ar,
                                              const Contour* &colinear)
{
  colinear = NULL;

  Cont_Nest_C_Cursor nc(ar);

  if (!nc || cnt.Empty()) return false;

  for (;nc;++nc) {
    if (Cnt_Inside(cnt,*nc,colinear) || colinear) return true;
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ------- Remove short elements (short in 2D) -------------------------- */
/* ---------------------------------------------------------------------- */

void Remove_Short_Elems(Elem_List& ellst, double tol, bool closed)
{
  Elem_Cursor elc(ellst);
  if (!elc) return;

  bool join_first = false, join = false, first = true;

  Vec3 fstp(elc->El().P1());
  Vec3 lstp(elc.Pred()->El().P2());

  double len = 0.0;

  while (elc) {
   len += elc->El().Len_XY();

   if (len < tol) {
     elc.Delete();
     join = true;
   }
   else {
     if (join) {
       if (first) {
         if (closed) join_first = true;
         else elc->El().Stretch_Begin_XY(fstp,false);
       }
       else {
         Elem &prvel = elc.Pred()->El();
         prvel.Join_To_XY(elc->El(),false);
       }

       join = false;
     }
     else {
       ++elc;
       first = false;
     }

     len = 0.0;
   }
  }

  if (ellst && (join || join_first)) {
    elc.To_Last();

    if (closed) elc->El().Join_To_XY(elc.Succ()->El(),false);
    else        elc->El().Stretch_End_XY(lstp,false);
  }
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
