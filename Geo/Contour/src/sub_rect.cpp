/* ---------------------------------------------------------------------- */
/* ---------------- Subcontour Rectangle Lists -------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "sub_rect.hi"

#include "cntpanic.hi"

#include <math.h>

namespace Ino
{

const double Sub_Rect_Project_Tol = 1.0e-11;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Sub_Rect::Sub_Rect(const Rect_Ax& rct, const Elem_Cursor& upto)
 : sub_rect(rct), upto_el(upto), Rect(sub_rect), Upto(upto_el)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Sub_Rect::Sub_Rect(const Sub_Rect& cp)
 : sub_rect(cp.sub_rect), upto_el(cp.upto_el), Rect(sub_rect), Upto(upto_el)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Sub_Rect& Sub_Rect::operator=(const Sub_Rect& src)
{
  sub_rect = src.sub_rect;
  upto_el = src.upto_el;
  return *this;
}

/* ---------------------------------------------------------------------- */
/* ------- Construct Element Rect List ---------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Rect_List::Elem_Rect_List(Elem_List& elem_list,
                                         int max_elems, double max_area)
 : Sub_Rect_List(), el_list(elem_list)
{
  if (max_elems < 1) max_elems = 1;
  if (max_area <= 0.0) max_area = 0.0;

  Delete();

  if (el_list.Length() < 1) return;

  Rect_Ax cur_rect,nxt_rect;
  int elem_cnt = 0;

  Elem_Cursor elc(el_list);

  while (elc) {
    bool go_on = true;

    if (elem_cnt >= max_elems) go_on = false;
    else {
      if (elem_cnt < 1) nxt_rect = elc->El().Rect();
      else {
        nxt_rect += elc->El().Rect();

        if (nxt_rect.Area_XY() > max_area) go_on = false;
      }
    }

    if (go_on) {
      cur_rect = nxt_rect;
      elem_cnt++;
      ++elc;
    }
    else {
      Push_Back(Sub_Rect(cur_rect,elc));
      elem_cnt = 0;
    }
  }

  if (elem_cnt > 0) Push_Back(Sub_Rect(cur_rect,elc));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Rect_List::Find_Elem_At_Par(double par, Elem_Cursor& elc) const
{
  Sub_Rect_C_Cursor rc(*this);

  elc = el_list.Begin();
  if (!elc) return false;

  for (;rc;++rc) {
    const Elem_Cursor& uptoel = rc->Upto;

    if (!uptoel || uptoel->El().Begin_Par() > par) break;

    elc = uptoel;
  }

  while (elc && elc->El().End_Par() <= par) ++elc;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Rect_List::Find_Elem_At_Par(double par, Elem_C_Cursor& elc) const
{
  Elem_Cursor nc_elc;

  bool ok = Find_Elem_At_Par(par,nc_elc);
  elc = nc_elc;

  return ok;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Rect_List::nearest_rect_to(const Vec2& p,
                                     Sub_Rect_C_Cursor& nrc,
                                     Elem_Cursor& fst_el) const
{
  nrc = End();
  fst_el = el_list.End();
  
  double mindist;
  Sub_Rect_C_Cursor crc(*this);

  if (!crc || !el_list) return false;

  nrc = crc;
  fst_el = el_list.Begin();
  mindist = crc->Rect.Dist_To_XY(p);

  while (++crc && mindist > 0.0) {
    double dist = crc->Rect.Dist_To_XY(p);
    if (dist < mindist) {
      mindist = dist;
      nrc     = crc;
      fst_el = crc.Pred()->Upto;
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Rect_List::Project_Pnt_XY(const Vec2& p, Elem_Cursor& nel,
                          Vec3& pp, double& parm, double& dist_xy) const
{
  Sub_Rect_C_Cursor nrc;
  nel = Elem_Cursor();

  Elem_Cursor cel;
  if (!nearest_rect_to(p,nrc,cel)) Cont_Panic(SubRect_No_Nearest_Rect);

  Sub_Rect_C_Cursor crc(nrc);

  bool first = true;

  do {
    if (!first && crc->Rect.Dist_To_XY(p) >
                                     fabs(dist_xy) + Vec2::IdentDist) {
      cel = crc->Upto; if (!cel) cel.To_Begin();
      ++crc;           if (!crc) crc.To_Begin();

      continue;
    }

    while (cel != crc->Upto) {
      double lpar,ldist;
      Vec3 lpp;
     
      if (cel->El().Project_Pnt_XY(p,Sub_Rect_Project_Tol,
                                                 true,lpp,lpar,ldist)) {
        if (first) {
          pp = lpp;
          parm = lpar;
          dist_xy = ldist;
          nel = cel;
          first = false;
        }
        else if (fabs(ldist) < fabs(dist_xy)) {
          pp = lpp;
          parm = lpar;
          dist_xy = ldist;
          nel = cel;
        }
      }
      else Cont_Panic(SubRect_Project_No_Elem);

      ++cel;
    }

    ++crc;
    if (!crc) crc.To_Begin();
    if (!cel) cel.To_Begin();

  } while (crc != nrc);

  if (first) nel = Elem_Cursor();

  return !first;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool par_in_range(double par, double par1, double par2)
{
  double ext_tol = 0.0;

  if (par1 <= par2) {
    if (par < par1-ext_tol || par > par2+ext_tol) return false; 
  }
  else {
    if (par < par1-ext_tol && par > par2+ext_tol) return false; 
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool range_overlap(double para1, double para2,
                          double parb1, double parb2)
{
  if (par_in_range(para1,parb1,parb2) ||
      par_in_range(para2,parb1,parb2) ||
      par_in_range(parb1,para1,para2)) return true;

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Rect_List::Project_Pnt_XY(const Vec2& p,
                                    double begin_par,
                                    double end_par,
                                    Elem_Cursor& nel,
                                    Vec3& pp, double& parm,
                                    double& dist_xy) const
{
  Sub_Rect_C_Cursor nrc;
  nel = Elem_Cursor();

  Elem_Cursor cel;
  if (!nearest_rect_to(p,nrc,cel)) Cont_Panic(SubRect_No_Nearest_Rect);

  Sub_Rect_C_Cursor crc(nrc);

  bool first = true;

  double par1 = cel->El().Begin_Par();

  do {
    Elem_Cursor upelc = crc->Upto;
    if (!upelc) upelc.To_Begin();

    double par2 = upelc->El().End_Par();

    if (!range_overlap(par1,par2,begin_par,end_par) ||
        (!first && crc->Rect.Dist_To_XY(p) >
                                     fabs(dist_xy) + Vec2::IdentDist)) {
      cel = upelc;
      ++crc; if (!crc) crc.To_Begin();

      par1 = par2;

      continue;
    }

    while (cel != crc->Upto) {
      double lpar,ldist;
      Vec3 lpp;
     
      const Elem& el = cel->El();
      
      if (!range_overlap(el.Begin_Par(),el.End_Par(),begin_par,end_par)) {
        ++cel;
        continue;
      }
      
      if (el.Project_Pnt_Strict_XY(p,Sub_Rect_Project_Tol,lpp,lpar,ldist) &&
                                    par_in_range(lpar,begin_par,end_par)) {
        if (first) {
          pp = lpp;
          parm = lpar;
          dist_xy = ldist;
          nel = cel;
          first = false;
        }
        else if (fabs(ldist) < fabs(dist_xy)) {
          pp = lpp;
          parm = lpar;
          dist_xy = ldist;
          nel = cel;
        }
      }

      ++cel;
    }

    ++crc;
    if (!crc) crc.To_Begin();
    if (!cel) cel.To_Begin();

    par1 = par2;

  } while (crc != nrc);

  if (first) nel = Elem_Cursor();

  return !first;
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
