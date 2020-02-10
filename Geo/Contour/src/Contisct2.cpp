/* ---------------------------------------------------------------------- */
/* ---------------- Intersection Processing for Contours ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1997 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "contisct.hi"

#include "contouri.hi"
#include "El_Arc.h"

#include "it_gen.h"
#include "sub_rect.hi"

#include "cntpanic.hi"

#include <math.h>
#include <stdio.h>

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
#include <stdio.h>

static void poutput(const Cont_Isect_D_Cursor& iscc)
{
  Cont_Isect_D_Cursor isc(iscc);

  FILE *fd = fopen("/tmp/clemens","w");

  char Y[] = "Yes";
  char N[] = "No";

  for (;isc;++isc) {
    char *a;

    if (isc->To_Left) a = Y; else a = N;

    fprintf(fd,"Par: %.4f, P (%.4f,%.4f) Left: %s\n",
    isc->Pnt.Par(),isc->Pnt.P().x,isc->Pnt.P().y,a);
  }

  fclose(fd);
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void number_isects(Cont_Isect_D_List& ilist)
{
  Cont_Isect_D_Cursor isc(ilist);

  int id = 0;
  
  for (;isc;++isc) isc->id_no = id++;  
}

/* ---------------------------------------------------------------------- */
/* ------- Cont2_Isect_List --------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Advance(Cont_Isect_D_Cursor& isc)
{
  const Contour* cont = isc->Pnt.Parent_Contour();

  ++isc; if (!isc) isc.To_Begin(); if (!isc) return;

  while (isc->Pnt.Parent_Contour() != cont) {
    ++isc; if (!isc) isc.To_Begin();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Backup(Cont_Isect_D_Cursor& isc)
{
  const Contour* cont = isc->Pnt.Parent_Contour();

  --isc; if (!isc) isc.To_Last(); if (!isc) return;

  while (isc->Pnt.Parent_Contour() != cont) {
    --isc; if (!isc) isc.To_Last();
  }
}

/* ---------------------------------------------------------------------- */
/* ------------------------ --------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Advance_Both(Cont_Isect_D_Cursor& isc)
{
  const Contour* cont     = isc->Pnt.Parent_Contour();
  const Contour* oth_cont = isc->Other->Pnt.Parent_Contour();

  ++isc; if (!isc) isc.To_Begin(); if (!isc) return;

  while (isc->Pnt.Parent_Contour() != cont ||
         isc->Other->Pnt.Parent_Contour() != oth_cont) {
    ++isc; if (!isc) isc.To_Begin();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Backup_Both(Cont_Isect_D_Cursor& isc)
{
  const Contour* cont     = isc->Pnt.Parent_Contour();
  const Contour* oth_cont = isc->Other->Pnt.Parent_Contour();

  --isc; if (!isc) isc.To_Last(); if (!isc) return;

  while (isc->Pnt.Parent_Contour() != cont ||
         isc->Other->Pnt.Parent_Contour() != oth_cont) {
    --isc; if (!isc) isc.To_Last();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
static int index2_of(const Cont_Isect_D_Cursor& isc)
{
  if (!isc) return -1;

  Cont_Isect_D_Cursor lisc(isc);
  lisc.To_Begin();

  int idx = 0;

  while (lisc && lisc != isc) {
    ++idx;
    ++lisc;
  }

  return idx;
}

static void do2_make_a_print(FILE *fd, Cont_Isect_D_List& ilist)
{
  if (!fd) return;

   Cont_Isect_D_Cursor isc(ilist);

   fprintf(fd," Nr Par         RelPar        X            Y         Nr  Prv Nxt a d\n");
   fprintf(fd,"-------------------------------------------------------------------\n");
   for (;isc;++isc) {
     char atg,dtg,ma=' ',md=' ';
     
     switch (isc->arrive) {
       case Cont_Isect::tang: atg = 't';
       break;
       case Cont_Isect::a_tang: atg = 'a';
       break;
       case Cont_Isect::left: atg = 'l';
       break;
       case Cont_Isect::right: atg = 'r';
       break;
       default: atg = '?';
     }
     
     switch (isc->depart) {
       case Cont_Isect::tang: dtg = 't';
       break;
       case Cont_Isect::a_tang: dtg = 'a';
       break;
       case Cont_Isect::left: dtg = 'l';
       break;
       case Cont_Isect::right: dtg = 'r';
       break;
       default: dtg = '?';
     }

     if (isc->Mark_Arrive) ma='m';
     if (isc->Mark_Depart) md='m';
          
     Cont_Isect_D_Cursor predc(isc->Other);
     Cont2_Isect_List::Backup_Both(predc);
     Cont_Isect& oth_prv_is = *predc->Other;

     Cont_Isect_D_Cursor nxtc(isc->Other);
     Cont2_Isect_List::Advance_Both(nxtc);
     Cont_Isect& oth_nxt_is = *nxtc->Other;

     fprintf(fd,"%3d  %7.2f %12.5f %12.5f %12.5f %3d %3d %3d  %c %c ,(%c,%c)\n",
                 isc->id_no,isc->Pnt.Par(),isc->Pnt.Rel_Par(),
                         isc->Pnt.P().x,isc->Pnt.P().y,
                                       isc->Other->id_no,oth_prv_is.id_no,
                                       oth_nxt_is.id_no,atg,dtg,ma,md);
//      idx++;
   }
}
#endif

void print2_isect(FILE *fd, int il, const Cont_Isect_D_Cursor& isc)
{
   if (!fd) return;
   
     char atg,dtg,ma=' ',md=' ';
     
     switch (isc->arrive) {
       case Cont_Isect::tang: atg = 't';
       break;
       case Cont_Isect::a_tang: atg = 'a';
       break;
       case Cont_Isect::left: atg = 'l';
       break;
       case Cont_Isect::right: atg = 'r';
       break;
       default: atg = '?';
     }
     
     switch (isc->depart) {
       case Cont_Isect::tang: dtg = 't';
       break;
       case Cont_Isect::a_tang: dtg = 'a';
       break;
       case Cont_Isect::left: dtg = 'l';
       break;
       case Cont_Isect::right: dtg = 'r';
       break;
       default: dtg = '?';
     }
     
     if (isc->Mark_Arrive) ma='m';
     if (isc->Mark_Depart) md='m';
          
     fprintf(fd,"%1d,%3d %7.2f %12.5f %12.5f %12.5f %3d  %c %c,(%c,%c)\n",
                il,isc->id_no,isc->Pnt.Par(),isc->Pnt.Rel_Par(),
                     isc->Pnt.P().x,isc->Pnt.P().y,isc->Other->id_no,
                     atg,dtg,ma,md);
}

/* ---------------------------------------------------------------------- */
/* --------Set boolean To_Left in the Cont_Isects ----------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::set_to_left()
{
  unmark();
  
  Cont_Isect_D_Cursor isbc1(ilist1);
  
  for (;isbc1;++isbc1) isbc1->Set_To_Left();

  unmark();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::remove_non_crossing()
{
  Cont_Isect_D_Cursor isbc1(ilist1);
  
  // Remove all intersections that are tangent/antitangent
  // on both arrive and depart (because they dont cross the contour)
  
  while (isbc1) {
    Cont_Isect& is = *isbc1;
    
    if ((is.arrive == Cont_Isect::tang ||
                                 is.arrive == Cont_Isect::a_tang) &&
        (is.depart == Cont_Isect::tang ||
                                 is.depart == Cont_Isect::a_tang)) {
      isbc1->Other.Delete();
      isbc1.Delete();
    }
    else ++isbc1;
  }
  
  // Now also delete other non_crossing intersections
  // (but not one-side tangent/anti-tangent intersections!)

  isbc1.To_Begin();
  
  while (isbc1) {
    if (isbc1->arrive == isbc1->depart) {
      isbc1->Other.Delete();
      isbc1.Delete();
    }
    else ++isbc1;
  }

  // Now look at pairs of intersections, that have coincident (tangent)
  // stretches between them.
  // If the pair ultimately crosses the contour, then remove one of them
  // and make the other a true crossing intersection

  isbc1.To_Begin();

  while (isbc1) {
    Cont_Isect_D_Cursor predc(isbc1); Backup_Both(predc);

    if (predc == isbc1) {
      ++isbc1;
      continue;
    }

    Cont_Isect& is     = *isbc1;
    Cont_Isect& prv_is = *predc;

    if (prv_is.depart == Cont_Isect::tang &&
                               is.arrive == prv_is.depart) {

      // If the pair crosses, then remove one

      if (prv_is.arrive != is.depart) {
        prv_is.depart = is.depart;

        prv_is.Other->depart = is.Other->depart;

        isbc1->Other.Delete();
        isbc1.Delete();

        continue;
      }
    }

    if (prv_is.arrive == Cont_Isect::a_tang &&
                               is.depart == prv_is.arrive) {

      // If the pair crosses, then remove one

      if (prv_is.depart != is.arrive) {
        prv_is.arrive = is.arrive;
        
        prv_is.Other->depart = is.Other->depart;

        isbc1->Other.Delete();
        isbc1.Delete();

        continue;
      }
    }
    
    ++isbc1;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void delete2_identical(Cont_Isect_D_List& ilist)
{
  // Delete identical intersections,

  // Tol slightly more than the maximum possible difference
  double tol = 4.2 * Vec2::IdentDist;

  bool modified = true;
  
  while (modified) {
    modified = false;

    Cont_Isect_D_Cursor isbc(ilist);

    while (isbc) {
      Cont_Isect_D_Cursor predc(isbc); Cont2_Isect_List::Backup(predc);

      if (predc == isbc) {
        // Only one intersection left on this contour
        // This should never occur off course, but the error
        // will be trapped later on (odd intersection count)
        
        ++isbc;
        continue;
      }
      
      Cont_Isect& is     = *isbc;
      Cont_Isect& oth_is = *is.Other;
      
      Cont_Isect& prv_is     = *predc;
      Cont_Isect& oth_prv_is = *prv_is.Other;
      
      // Companion intersections must be on the same contour
      
      if (oth_is.Cont != oth_prv_is.Cont) {
        ++isbc;
        continue;
      }

      double pdist1, pdist2;
      prv_is.Pnt.Par_Dist_To(is.Pnt,pdist1);
      oth_prv_is.Pnt.Min_Par_Dist_To(oth_is.Pnt,pdist2);

      if (fabs(pdist1) <= tol && fabs(pdist2) <= tol) {
        if (is.In_Range()) {
          prv_is.Other.Delete();
          predc.Delete();
        }
        else {
          is.Other.Delete();
          isbc.Delete();
        }

        modified = true;
        continue;
      }
        
      ++isbc;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void delete2_close(Cont_Isect_D_List& ilist)
{
  // Delete intersection pairs that are very close
  // where at least one is out of range

  // Tol slightly more than the maximum possible difference
  double tol = 4.2 * Vec2::IdentDist;

  bool modified = true;
  
  while (modified) {
    modified = false;

    Cont_Isect_D_Cursor isbc(ilist);

    while (isbc) {
      Cont_Isect_D_Cursor predc(isbc); Cont2_Isect_List::Backup(predc);

      if (predc == isbc) {
        // Only one intersection left on this contour
        // This should never occur off course, but the error
        // will be trapped later on (odd intersection count)
        
        ++isbc;
        continue;
      }
      
      Cont_Isect& is = *isbc;
      Cont_Isect& oth_is = *is.Other;
      
      Cont_Isect& prv_is = *predc;
      Cont_Isect& oth_prv_is = *prv_is.Other;
      
      // Companion intersections must be on the same contour
      
      if (prv_is.Cont != oth_prv_is.Cont) {
        ++isbc;
        continue;
      }

      double pdist1, pdist2;
      prv_is.Pnt.Par_Dist_To(is.Pnt,pdist1);
      oth_prv_is.Pnt.Min_Par_Dist_To(oth_is.Pnt,pdist2);

      if (fabs(pdist1) <= tol && fabs(pdist2) <= tol) {
        prv_is.Other.Delete();
        predc.Delete();

        is.Other.Delete();
        isbc.Delete();

        modified = true;
        continue;
      }

      ++isbc;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void mark2_for_tangent_analysis(const Cont_Isect_D_Cursor& isc,
                                                            double minlen)
{
  if (!isc) return;
  
  // Analyze if:
  // a. arrive or depart == tangent/anti_tangent
  // b. the intersection is near start- or endpoint of element
  //    (possibly out-of-range)
  // c. The arriving/departing stretch is very short
  
  Cont_Isect& is = *isc;
  Cont_Isect& is_oth = *is.Other;

  if (is.arrive == Cont_Isect::tang ||
           is.arrive == Cont_Isect::a_tang) is.Mark_Arrive = true;
  
  if (is.depart == Cont_Isect::tang ||
           is.depart == Cont_Isect::a_tang) is.Mark_Depart = true;

  if (is.Mark_Arrive && is.Mark_Depart) return;

  if (is.Near_Elem_Start() || is.Near_Elem_End()) {
    is.Mark_Arrive = true;
    is.Mark_Depart = true;

    is_oth.Mark_Arrive = true;
    is_oth.Mark_Depart = true;
    return;
  }
  
  // Determine if arriving stretch is very short
  
  Cont_Isect_D_Cursor predc(isc);  Cont1_Isect_List::Backup(predc);

  Cont_Isect& prv_is = *predc;
  
  double pdist;
  prv_is.Pnt.Par_Dist_To(is.Pnt, pdist);

  if (fabs(pdist) < minlen) {
    is.Mark_Arrive=true;
    is_oth.Mark_Arrive = true;
    is_oth.Mark_Depart = true;

    prv_is.Mark_Depart = true;
    prv_is.Other->Mark_Arrive = true;
    prv_is.Other->Mark_Depart = true;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::project_range(const Cont_PPair& r, bool ccw,
                                     const Cont_Clsd& cont,
                                     const Cont_Pnt& p1, const Cont_Pnt& p2,
                                     bool full_range,
                                     double tol, Cont_Isect::Rel_Dir& dir)
{
  // project points on range p2 onto cont

  Elem_C_Cursor elc(r.one);
  
  bool spanning = false;
  if (elc == r.two && r.one.Par() > r.two.Par()) {
    // This is a (rare) special case, where element elc spans
    // from the END to the START of the range!
    
    spanning = true;
  }

  // Project all mid- and endpoints within the range
  
  for (;;) {
    const Elem& el = elc->El();
    
    double bpar,epar;
    
    bool in_range = false;
    
    if (spanning) in_range = r.Range_Inside_2(el, bpar, epar);
    else          in_range = r.Range_Inside_1(el, bpar, epar);

    if (in_range) {

      double dist_xy, midpar = (bpar+epar)/2.0;
      Vec3 mp;
      Cont_Pnt pnt;
      el.At_Par(midpar,mp);

      if (cont.Project_Pnt_XY(mp,pnt,dist_xy)) {
        if (fabs(dist_xy) > tol) {
          if (dist_xy > 0.0) dir = Cont_Isect::left;
          else               dir = Cont_Isect::right;

          return;
        }
      }

      el.At_Par(epar,mp);

      if (cont.Project_Pnt_XY(mp,pnt,dist_xy)) {
        if (fabs(dist_xy) > tol) {
          if (dist_xy > 0.0) dir = Cont_Isect::left;
          else               dir = Cont_Isect::right;

          return;
        }
      }
    }
    
    if (elc.Container()->Length() < 2) spanning = false;
 
    if (!spanning && elc == r.one) break;

    spanning = false;
    
    ++elc; if (!elc) elc.To_Begin();
  }

  // We get here if the range r is close to contour cont on all points
  // Now dir is either tang or a_tang
  // We must determine which of both it actually is.

  if (full_range) {
    if (ccw == cont.Ccw()) dir = Cont_Isect::tang;
    else                   dir = Cont_Isect::a_tang;
    
    return;
  }

  dir = Cont_Isect::tang;
  
  Cont_Pnt midpnt(r.Mid());
  
  if (!midpnt) return;  // Serious error!
  
  double dist_xy;
  Cont_Pnt pnt;

  if (!cont.Project_Pnt_XY(midpnt.P(),pnt,dist_xy)) return; // Error!!
  
  if (Cont_Par_Between(p1,p2,pnt.Par())) dir = Cont_Isect::tang;
  else                                   dir = Cont_Isect::a_tang;

  return;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::analyze_arrive(const Cont_Isect_D_Cursor& isc,
                                                              double tol)
{
   if (!isc) return;
   
   Cont_Isect_D_Cursor predc(isc->Other); Backup_Both(predc);
   
   Cont_Isect&     is = *isc;
   Cont_Isect& oth_is = *isc->Other;
   Cont_Isect& prv_oth_is = *predc;

   Cont_Isect& prv_is = *prv_oth_is.Other;

   Cont_PPair range(prv_oth_is.Pnt,oth_is.Pnt);
   if (predc == isc->Other) range.Is_Full(true);
   
   const Cont_Clsd& cont     = (const Cont_Clsd&)(is.Cont->Cont);
   const Cont_Clsd& oth_cont = (const Cont_Clsd&)(oth_is.Cont->Cont);
   
   Cont_Isect::Rel_Dir dir;
   
   project_range(range,oth_cont.Ccw(),cont,
                             prv_is.Pnt,is.Pnt,range.Is_Full(),tol,dir);
   
   is.arrive      = dir; is.Mark_Arrive     = false;
   prv_is.depart  = dir; prv_is.Mark_Depart = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::analyze_depart(const Cont_Isect_D_Cursor& isc,
                                                              double tol)
{
   if (!isc) return;
   
   Cont_Isect_D_Cursor nxtc(isc->Other); Advance_Both(nxtc);
   
   Cont_Isect&     is = *isc;
   Cont_Isect& oth_is = *isc->Other;
   Cont_Isect& nxt_oth_is = *nxtc;

   Cont_Isect& nxt_is = *nxt_oth_is.Other;

   Cont_PPair range(oth_is.Pnt,nxt_oth_is.Pnt);
   if (nxtc == isc->Other) range.Is_Full(true);
   
   const Cont_Clsd& cont     = (const Cont_Clsd&)(is.Cont->Cont);
   const Cont_Clsd& oth_cont = (const Cont_Clsd&)(oth_is.Cont->Cont);
   
   Cont_Isect::Rel_Dir dir;
   
   project_range(range,oth_cont.Ccw(),cont,
                          is.Pnt,nxt_is.Pnt,range.Is_Full(),tol,dir);
   
   is.depart      = dir; is.Mark_Depart     = false;
   nxt_is.arrive  = dir; nxt_is.Mark_Arrive = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::analyze_tangent(const Cont_Isect_D_Cursor& isc)
{
  if (!isc) return;
  
  Cont_Isect&     is = *isc;
  Cont_Isect& oth_is = *isc->Other;

  if (is.arrive == Cont_Isect::tang) {
    if (oth_is.arrive == Cont_Isect::left)       is.arrive = Cont_Isect::right;
    else if (oth_is.arrive == Cont_Isect::right) is.arrive = Cont_Isect::left;
  }
  else if (is.arrive == Cont_Isect::a_tang) {
    if (oth_is.depart == Cont_Isect::left)       is.arrive = Cont_Isect::left;
    else if (oth_is.depart == Cont_Isect::right) is.arrive = Cont_Isect::right;
  }

  if (is.depart == Cont_Isect::tang) {
    if (oth_is.depart == Cont_Isect::left)       is.depart = Cont_Isect::right;
    else if (oth_is.depart == Cont_Isect::right) is.depart = Cont_Isect::left;
  }
  else if (is.depart == Cont_Isect::a_tang) {
    if (oth_is.arrive == Cont_Isect::left)       is.depart = Cont_Isect::left;
    else if (oth_is.arrive == Cont_Isect::right) is.depart = Cont_Isect::right;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::analyze_tangent_stretches(FILE* /*fd*/, double minlen)
{
  // Analyze tangency of stretches:
  // a. that are deemed tangent on either end so far
  // b. that start/end in an intersection that is out of range
  // c. if the stretch is very short ( < minlen)
  
  // The analysis is done by finding the max. distance between
  // the stretches of contour on either side.

  unmark_arrive_depart();

  Cont_Isect_D_Cursor isbc1(ilist1);
  for (;isbc1;++isbc1) mark2_for_tangent_analysis(isbc1,minlen);

  Cont_Isect_D_Cursor isbc2(ilist2);
  for (;isbc2;++isbc2) mark2_for_tangent_analysis(isbc2,minlen);

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Mark Analyze_Tangent: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc1.To_Begin();
  for (;isbc1;++isbc1) {
    if (isbc1->Mark_Arrive) analyze_arrive(isbc1,minlen);
  }

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Analyze Arrive 1: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc1.To_Begin();
  for (;isbc1;++isbc1) {
    if (isbc1->Mark_Depart) analyze_depart(isbc1,minlen);
  }

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Analyze Depart 1: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc2.To_Begin();
  for (;isbc2;++isbc2) {
    if (isbc2->Mark_Arrive) analyze_arrive(isbc2,minlen);
  }

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Analyze Arrive 2: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc2.To_Begin();
  for (;isbc2;++isbc2) {
    if (isbc2->Mark_Depart) analyze_depart(isbc2,minlen);
  }
  
  // Now analyze tangent/antitangent situations
  
#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Analyze Depart 2: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc1.To_Begin();
  for (;isbc1;++isbc1) analyze_tangent(isbc1);

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Analyze Tangent 1: \n");
   do2_make_a_print(fd,ilist1);
   do2_make_a_print(fd,ilist2);
#endif

  isbc2.To_Begin();
  for (;isbc2;++isbc2) analyze_tangent(isbc2);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::cleanup()
{
#ifdef DEBUG_PRINT
  FILE *fd = fopen("/tmp/clemens","w");
  fprintf(fd,"Initieel: \n\n");
  do2_make_a_print(fd,ilist1);
  do2_make_a_print(fd,ilist2);

#else
  FILE *fd = NULL;
#endif


  // Identical intersections can occur if an intersection is located
  // almost exactly at the begin/end of an element

  delete2_identical(ilist1);
  delete2_identical(ilist2);

  // Now delete all pairs of intersections that are very close
  // in parametric space

  delete2_close(ilist1);
  
  Cont_Isect_D_Cursor isbc1(ilist1);
  if (!isbc1) return;

  for (;isbc1;++isbc1) {
    isbc1->Eval2_Sides();
    isbc1->Other->Eval2_Sides();
  }
  
  unmark();
  
#ifdef DEBUG_PRINT
  fprintf(fd,"\nNa Eval_Sides: \n");
#endif

  // Now if there are any tangent intersections, analyze the adjacent
  // contour stretch, this is also done for out-of-range intersections
  // and very short stretches between intersections

  analyze_tangent_stretches(fd,2.0*Vec2::IdentDist);


#ifdef DEBUG_PRINT
  fprintf(fd,"\nNa Analyze_Tangent: \n");
  do2_make_a_print(fd,ilist1);
  do2_make_a_print(fd,ilist2);
#endif

  // Now remove intersections that dont crossover and also one of each
  // pair that encloses a tangent stretch, but where the pair does
  // effectively crossover.

  remove_non_crossing();
  

#ifdef DEBUG_PRINT
  fprintf(fd,"\nNa remove non crossing: \n");
  do2_make_a_print(fd,ilist1);
  do2_make_a_print(fd,ilist2);

  fclose(fd);
#endif

  // At this point the intersections should be consistent in tangency


  // Set flag To_Left in each intersection
  set_to_left();
  
  // Reverse if necessary (not yet implemented correctly)

  if (rev1) {
    isbc1.To_Begin();

    for (;isbc1;++isbc1) isbc1->Pnt.Reverse(false);

    ilist1.Reverse();
  }

  if (rev2) {
    Cont_Isect_D_Cursor isbc2(ilist2);

    for (;isbc2;++isbc2) isbc2->Pnt.Reverse(false);

    ilist2.Reverse();
  }


  unmark();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::
            intersect_el(Cont_Ref& cntref1, Cont_Ref& cntref2,
                         const Elem_Cursor& elc1,
                         const Elem_Cursor& elc2)
{
  Isect_Lst pair_list;

  Elem &el1 = elc1->El();
  Elem &el2 = elc2->El();


  el1.Intersect_XY(el2,true,pair_list);

  if (!pair_list) return;

  // Insert results into list

  Isect_Cursor plc(pair_list);

  Cont_Isect_D_Cursor isc1(ilist1);
  Cont_Isect_D_Cursor isc2(ilist2);


  plc.To_Begin();

  for (;plc;++plc) {
    Vec3 isp(plc->P);

    double rpar = plc->First.Par - el1.Begin_Par();

    Cont_Pnt pnt1(cntref1.Cont,elc1,rpar,isp);
    pnt1.calc_point_attr();

    rpar = plc->Last.Par - el2.Begin_Par();
 
    Cont_Pnt pnt2(cntref2.Cont,elc2,rpar,isp);
    pnt2.calc_point_attr();

    isc1.To_End(); isc1.Insert(Cont_Isect(&cntref1,pnt1));
    isc2.To_End(); isc2.Insert(Cont_Isect(&cntref2,pnt2));

    isc1->Other = isc2; isc2->Other = isc1;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::intersect_xy(Cont_Ref& cntref1,
                                     Cont_Ref& cntref2, bool one_only)
{
  Contour &cnt1 = (Contour &)(cntref1.Cont);
  Contour &cnt2 = (Contour &)(cntref2.Cont);

  if (!cnt1.el_rect_list) cnt1.build_rect_list();
  if (!cnt2.el_rect_list) cnt2.build_rect_list();

  Elem_Cursor el1(cnt1.el_list);
  Elem_Cursor el2(cnt2.el_list);

  if (!el1 || !el2 ||
      !cnt1.Rect_Ax::Intersects_XY(cnt2,Vec2::IdentDist)) return;

  Sub_Rect_C_Cursor rc1(cnt1.el_rect_list->Begin());
  Sub_Rect_C_Cursor rc2(cnt2.el_rect_list->Begin());

  for (;rc1;++rc1) {
    const Rect_Ax& rct1 = rc1->Rect;

    if (&cnt1 == &cnt2) rc2 = rc1;
    else                rc2.To_Begin();

    el2.To_Begin();

    for (;rc2;++rc2) {
      const Rect_Ax& rct2 = rc2->Rect;

      if (rct1.Intersects_XY(rct2,Vec2::IdentDist)) {

        Elem_Cursor curel1(el1);

        for (;curel1 != rc1->Upto;++curel1) {
          if (!curel1) Cont_Panic(ContIsect_Nill_Cursor);

          const Elem &locel1 = curel1->El();

          if (locel1.Len_XY() >= Vec2::IdentDist &&
              locel1.Rect().Intersects_XY(rct2,Vec2::IdentDist)) {

            Elem_Cursor curel2(el2);

            if (rc1 == rc2) {
              curel2 = curel1; ++curel2;
            }

            for (;curel2 != rc2->Upto;++curel2) {
              if (!curel2) Cont_Panic(ContIsect_Nill_Cursor);

              const Elem &locel2 = curel2->El();

              if (locel2.Len_XY() >= Vec2::IdentDist) {
                intersect_el(cntref1,cntref2,curel1,curel2);

                if (one_only && ilist1) return;
              }
            }
          }
        }
      }

      el2 = rc2->Upto;
    }

    el1 = rc1->Upto;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::unmark()
{
  Cont_Isect_D_Cursor isc(ilist1);

  for (;isc;++isc) isc->Mark = false;

  isc = ilist2.Begin();

  for (;isc;++isc) isc->Mark = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::delete_marked()
{
  Cont_Isect_D_Cursor isc(ilist1);

  while (isc) {
    if (isc->Mark) {
      isc->Other.Delete();
      isc.Delete();
    }
    else ++isc;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::unmark_arrive_depart()
{
  Cont_Isect_D_Cursor isc(ilist1);

  for (;isc;++isc) {
    Cont_Isect& is = *isc;
    
    is.Mark_Arrive = false;
    is.Mark_Depart = false;

    is.Full_Range_Arrive = false;
    is.Full_Range_Depart = false;
  }

  isc = ilist2.Begin();

  for (;isc;++isc) {
    Cont_Isect& is = *isc;
    
    is.Mark_Arrive = false;
    is.Mark_Depart = false;

    is.Full_Range_Arrive = false;
    is.Full_Range_Depart = false;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::
           collect_non_intersecting(Cont_Clsd_D_List& cnt_list,
                                    Cont_D_List *rest1,
                                    Cont_D_List *rest2) const
{
  rlist1->Determine_Pos_In_List(*rlist2);
  rlist2->Determine_Pos_In_List(*rlist1);

  Cont_Clsd_Cursor cc(cnt_list);

  Cont_Ref_Cursor rc(*rlist1);

  for (;rc;++rc) {
    if (rc->Isect_Count) continue;

    bool use_it = rc->To_Left == left; if (rev1) use_it = !use_it;

    if (rc->Colinear) use_it = false;

    if (use_it) {
      cc.To_End(); cc.Insert(Cont_Clsd());

      cc->cont = rc->Cont; cc->lccw = rc->Ccw;

      if (rev1) cc->Reverse();
    }
    else if (rest1) {
      Cont_Cursor occ(*rest1); occ.To_End();

      occ.Insert(rc->Cont);

      if (rev1) occ->Reverse();
    }
  }

  rc = rlist2->Begin();

  for (;rc;++rc) {
    if (rc->Isect_Count) continue;

    bool use_it;

    if (rc->Colinear) {
      use_it = rc->Ccw == rc->Colinear->Ccw;
      if (rev1) use_it = !use_it;
    }
    else use_it = rc->To_Left == left;

    if (rev2) use_it = !use_it;

    if (use_it) {
      cc.To_End(); cc.Insert(Cont_Clsd());

      cc->cont = rc->Cont; cc->lccw = rc->Ccw;

      if (rev2) cc->Reverse();
    }
    else if (rest2) {
      Cont_Cursor occ(*rest2); occ.To_End();

      occ.Insert(rc->Cont);

      if (rev2) occ->Reverse();
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Copy intersect lists ----------------------------------------- */
/* ---------------------------------------------------------------------- */

static void copy_intersect_lists(const Cont_Isect_D_List& src,
                          Cont_Isect_D_List &lst1, Cont_Isect_D_List &lst2)
{
  lst1.Delete(); lst2.Delete();

  Cont_Isect_D_C_Cursor sc; sc  = src.Begin();
  Cont_Isect_Cursor ic1;    ic1 = lst1.Begin();
  Cont_Isect_Cursor ic2;    ic2 = lst2.Begin();

  for (;sc;++sc) {
    ic1.To_End(); ic1.Insert(*sc);
    ic2.To_End(); ic2.Insert(*(sc->Other));

    ic1->Other = ic2;
    ic2->Other = ic1;
  }

  ic1 = lst2.Begin();
  ic2 = ic1; ic2.To_End();

  IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>::Sort(ic1,ic2,
                                                        Cont_Isect_Cmp());
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::extract_combine(Cont_Clsd_D_List& cnt_list)
{
  unmark();

  Cont_Clsd_Cursor cc(cnt_list);

  while (ilist1) {
    bool on_list1 = true;

    Contour newc;
    Elem_Cursor elc(newc.el_list);

    unmark();

    Cont_Isect_Cursor isc; isc = ilist1.Begin();
    Cont_Isect_Cursor prvisc(isc);

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) {
  
        isc = isc->Other;
        on_list1 = !on_list1;
      }

      prvisc = isc;
      Advance(isc);

    } while (!isc->Mark);

    unmark();

    bool atang = true;

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) {
        if (prvisc->arrive != Cont_Isect::a_tang) atang = false;

        bool rev = (on_list1 ? rev1 : rev2);

        Contour newpiece;
 
        if (rev) {
          isc->Pnt.Extract_Upto(prvisc->Pnt, newpiece);
          newpiece.Reverse();
        }
        else prvisc->Pnt.Extract_Upto(isc->Pnt, newpiece);
          
        elc.To_Last();

        if (!newpiece.Empty()) {
          newpiece.el_list.Append_To(newc.el_list);

          if (elc && elc.Succ()) elc->El().Join_To_XY(elc.Succ()->El());
        }

        isc = isc->Other;
        on_list1 = !on_list1;
      }

      prvisc = isc;
      Advance(isc);
    } while (!isc->Mark);

    elc.To_Last();
    if (elc && elc.Succ()) elc->El().Join_To_XY(elc.Succ()->El());


    // Add a check here for full tangential (anti_tangential) solution!!!


    if (!atang && !newc.Empty()) {

      Remove_Short_Elems(newc.el_list, 5.0*Vec2::IdentDist,true);

      newc.calc_invar();

      cc.To_End();
      cc.Insert(Cont_Clsd());

      newc.Move_To(cc->cont);

      cc->calc_invar();
        
      // If area is very small, just delete the contour
      if (fabs(cc->Area_XY()) < Vec2::IdentDist/100.0/Vec2::Pi)
                                                          cc.Delete(); 
    }

    delete_marked();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::extract_comb_rest(Cont_D_List *cnt_list1,
                                          Cont_D_List *cnt_list2)
{
  unmark();

  while (ilist1) {
    bool on_list1 = true;

    Cont_Isect_Cursor isc; isc = ilist1.Begin();
    Cont_Isect_Cursor prvisc(isc);

    unmark();

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left != left) {
  
        isc = isc->Other;
        on_list1 = !on_list1;
      }

      prvisc = isc;
      Advance(isc);

    } while (!isc->Mark);

    unmark();

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left != left) {
        bool extract = (on_list1 && cnt_list1) || (!on_list1 && cnt_list2);

        if (extract) {
          bool rev = (on_list1 ? rev1 : rev2);

          Contour newpiece;
 
          if (rev) {
            isc->Pnt.Extract_Upto(prvisc->Pnt, newpiece);
            newpiece.Reverse();
          }
          else prvisc->Pnt.Extract_Upto(isc->Pnt, newpiece);
        
          newpiece.calc_invar();

          if (!newpiece.Empty()) {
            Remove_Short_Elems(newpiece.el_list,
                                   5.0*Vec2::IdentDist,newpiece.Closed());
            if (on_list1) {
              if (cnt_list1) cnt_list1->End().Insert(newpiece);
            }
            else if (cnt_list2) cnt_list2->End().Insert(newpiece);
          }
        }

        isc = isc->Other;
        on_list1 = !on_list1;
      }

      prvisc = isc;
      Advance(isc);
    } while (!isc->Mark);

    delete_marked();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont2_Isect_List::Cont2_Isect_List(Cont_Ref_List& lst1, bool rev_1,
                                       Cont_Ref_List& lst2, bool rev_2,
                                       bool go_left, bool must_cleanup)
  : ilist1(), ilist2(), left(go_left), rev1(rev_1), rev2(rev_2),
    offsetref(NULL), rlist1(&lst1), rlist2(&lst2)
{
  if (rlist1 == rlist2) return;

  Cont_Ref_Cursor rc1(lst1);

  for (;rc1;++rc1) {
    Cont_Ref_Cursor rc2(lst2);

    for (;rc2;++rc2) intersect_xy(*rc1,*rc2);
  }

  // Sort the intersections by parametric

  Cont_Isect_D_Cursor isbc1(ilist1);
  if (!isbc1) return;

  // First sort the intersections by parametric distance

  Cont_Isect_D_Cursor isec1(ilist1); isec1.To_End();

  IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>::Sort(isbc1,isec1,
                                                        Cont_Isect_Cmp());

  // Companion list needs cleaning as well
  Cont_Isect_D_Cursor isbc2(ilist2);
  Cont_Isect_D_Cursor isec2(ilist2); isec2.To_End();

  IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>::Sort(isbc2,isec2,
                                                        Cont_Isect_Cmp());

  number_isects(ilist1);
  number_isects(ilist2);
  
  if (must_cleanup) cleanup();

  // Set intersection count for each contour in the ref_list

  lst1.Isect_Count_All(0);
  lst2.Isect_Count_All(0);

  Cont_Isect_D_Cursor ic(ilist1);

  for (;ic;++ic) ++(ic->Cont->Isect_Count);

  ic = ilist2.Begin();

  for (;ic;++ic) ++(ic->Cont->Isect_Count);


  if (!must_cleanup) return;

  if (lst1.Closed() && lst2.Closed()) {

    bool odd = false;
    
    Cont_Ref_C_Cursor rfc(lst1);
    for (;rfc;++rfc) {
      if (rfc->Isect_Count & 1) {
        odd = true;
        break;
      }
    }
    
    if (!odd) {
      rfc = lst2.Begin();
      for (;rfc;++rfc) {
        if (rfc->Isect_Count & 1) {
          odd = true;
          break;
        }
      }
    }
    
    if (odd) {    
      Cont_Panic(ContIsect_Odd_Intersect);
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Intersections_Into(Cont_PPair_List& pplist)
{
  pplist.Delete();

  unmark();

  Cont_PPair pair;

  Cont_Isect_D_Cursor ilc(ilist1);

  for (;ilc;++ilc) {
    if (!ilc->Mark) {
      pair.one = ilc->Pnt;
      pair.two = ilc->Other->Pnt;

      pplist.Push_Back(pair);

      ilc->Mark = true; ilc->Other->Mark = true;
    }
  }

  unmark();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont2_Isect_List::Extract_Combine(Cont_Clsd_D_List& cnt_list,
                                         Cont_D_List *rest1,
                                         Cont_D_List *rest2)
{
  cnt_list.Delete();

  if (rest1) rest1->Delete();
  if (rest2) rest2->Delete();

  // Hunt for intersected parts

  Cont_Isect_D_List clist1;
  Cont_Isect_D_List clist2;

  if (rest1 || rest2) copy_intersect_lists(ilist1, clist1, clist2);

  Cont_Clsd_D_List int_cnt_list;

  extract_combine(int_cnt_list);

  int_cnt_list.Append_To(cnt_list);

  if (rest1 || rest2) {
    copy_intersect_lists(clist1, ilist1, ilist2);
    extract_comb_rest(rest1, rest2);
  }

  collect_non_intersecting(cnt_list, rest1, rest2);
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
