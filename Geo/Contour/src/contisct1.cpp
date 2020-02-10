/* ---------------------------------------------------------------------- */
/* ---------------- Intersection Processing for Contours ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1997 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "contisct.hi"

#include "contouri.hi"
#include "El_Line.h"
#include "El_Arc.h"

#include "it_gen.h"
#include "sub_rect.hi"

#include "cntpanic.hi"

#include <math.h>
#include <stdio.h>

namespace Ino
{

typedef IT_D_C_Cursor<Cont_Isect,
                                Cont_Isect_Alloc> Cont_Isect_D_C_Cursor;

/* ---------------------------------------------------------------------- */
/* ------- Contour Reference -------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref::Cont_Ref(const Contour& cnt, bool ccw)
  : Cont(cnt), Isect_Count(0), To_Left(true), Ccw(ccw), Colinear(NULL)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref::Cont_Ref(const Cont_Ref& cp)
: Cont(cp.Cont), Isect_Count(cp.Isect_Count), To_Left(cp.To_Left),
  Ccw(cp.Ccw), Colinear(cp.Colinear)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref& Cont_Ref::operator=(const Cont_Ref& /*src*/)
{
  return *this;  // Should really throw an exception!!
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref_List::Cont_Ref_List(const Contour& cnt)
 : Cont_Ref_D_List(), contclsd(NULL), contnest(NULL),
   contarea(NULL), kind(ref_contour)
{
  Delete();
  Push_Back(Cont_Ref(cnt,false));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref_List::Cont_Ref_List(const Cont_Clsd& cnt)
 : Cont_Ref_D_List(), contclsd(&cnt), contnest(NULL),
   contarea(NULL), kind(ref_contclsd)
{
  Delete();
  Push_Back(Cont_Ref(cnt,cnt.Ccw()));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref_List::Cont_Ref_List(const Cont_Nest& nst)
 : Cont_Ref_D_List(), contclsd(NULL), contnest(&nst),
   contarea(NULL), kind(ref_contnest)
{
  Delete();

  Cont_Clsd_C_Cursor cc(nst);

  for (;cc;++cc) Push_Back(Cont_Ref(*cc,cc->Ccw()));
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref_List::Cont_Ref_List(const Cont_Area& ar)
 : Cont_Ref_D_List(), contclsd(NULL), contnest(NULL),
   contarea(&ar), kind(ref_contarea)
{
  Delete();

  Cont_Nest_C_Cursor nsc(ar);

  for (;nsc;++nsc) {
    Cont_Clsd_C_Cursor cc(*nsc);

    for (;cc;++cc) Push_Back(Cont_Ref(*cc,cc->Ccw()));
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Ref_List::Isect_Count_All(int val)
{
  Cont_Ref_Cursor rc(*this);

  for (;rc;++rc) rc->Isect_Count = val;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Ref_List::Closed() const
{
  if (kind == ref_contour) {
    Cont_Ref_C_Cursor rc(*this);
    if (!rc) return false;
    else     return rc->Cont.Closed();
  }
  else return true;
}

/* ---------------------------------------------------------------------- */
/* --- Determine contour pos (inside/outside) in other list ------------- */
/* --- but only if Flag value is zero (i.e. having no intersections) ---- */
/* --- and only for closed contours ------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Ref_List::Determine_Pos_In_List(Cont_Ref_List& otherlist)
{
  Cont_Ref_Cursor rc(*this);

  for (;rc;++rc) {
    if (rc->Isect_Count == 0) {
      const Contour *colcont = NULL;

      switch (otherlist.kind) {
       case ref_contclsd: {
         const Cont_Clsd &othcont = *otherlist.contclsd;
         rc->To_Left = Cnt_Inside(rc->Cont,othcont,colcont) == othcont.Ccw();
        }
       break;

       case ref_contnest: {
         const Cont_Nest &othnest = *otherlist.contnest;
         rc->To_Left = Cnt_Inside(rc->Cont,othnest,colcont) == othnest.Ccw();
        }
       break;

       case ref_contarea: {
         const Cont_Area &othar = *otherlist.contarea;
         rc->To_Left = Cnt_Inside(rc->Cont, othar, colcont) == othar.Ccw();
        }
       break;

       default:
         break;
      }

      if (colcont) {
        Cont_Ref_Cursor refc(otherlist);

        for (;refc;++refc) {
          if (&(refc->Cont) == colcont) {
            rc->Colinear = &(*refc);
            refc->Colinear = &(*rc);
            break;
          }
        }
      }
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Contour Intersection Points ---------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Isect::Cont_Isect(Cont_Ref *cntref, const Cont_Pnt& p)
  : id_no(0), To_Left(false), Pnt(p), Cont(cntref),
    arrive(left), depart(left),
    St_Pt(),End_Pt(),
    Other(*(new Cont_Isect_Cursor())), Mark(false),
    Mark_Arrive(false), Mark_Depart(false),
    Full_Range_Arrive(false), Full_Range_Depart(false)
{
}

/* ---------------------------------------------------------------------- */
/* ------- Private Copy Constructor ------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Isect::Cont_Isect(const Cont_Isect& cp)
  : id_no(0), To_Left(cp.To_Left), Pnt(cp.Pnt), Cont(cp.Cont),
    arrive(cp.arrive), depart(cp.depart),
    St_Pt(cp.St_Pt), End_Pt(cp.End_Pt),
    Other(*(new Cont_Isect_Cursor())), Mark(false),
    Mark_Arrive(cp.Mark_Arrive), Mark_Depart(cp.Mark_Depart),
    Full_Range_Arrive(cp.Full_Range_Arrive),
    Full_Range_Depart(cp.Full_Range_Depart)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Isect::~Cont_Isect()
{
  delete &Other;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Isect::In_Range() const
{
  if (!Pnt.Cursor()) return false;

  const Elem& curel = Pnt.Cursor()->El();

  if (Pnt.Rel_Par() < 0.0 || Pnt.Rel_Par() > curel.Par_Len()) return false;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Isect::Near_Elem_Start() const
{
  if (!Pnt.Cursor()) return false;

  if (Pnt.Rel_Par() <= Vec2::IdentDist) return true;

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont_Isect::Near_Elem_End() const
{
  if (!Pnt.Cursor()) return false;

  const Elem& curel = Pnt.Cursor()->El();

  if (Pnt.Rel_Par() >= curel.Par_Len() - Vec2::IdentDist) return true;

  return false;
}

/* ---------------------------------------------------------------------- */
/* -------- Set value of boolean To_Left -------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Isect::Set_To_Left()
{
  if (Mark) return; // Already set

       if (arrive == left)  To_Left = false;
  else if (arrive == right) To_Left = true;
  else if (depart == left)  To_Left = true;
  else                      To_Left = false;

  Other->To_Left = !To_Left;
  
  Mark = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool arr_dep_tab_set = false;
static Cont_Isect::Rel_Dir arrive_tab[4][4], depart_tab[4][4];

static void set_arr_dep_tabs()
{
  arr_dep_tab_set = true;
  
  arrive_tab[Cont_Isect::tang  ][Cont_Isect::tang  ] = Cont_Isect::tang;
  arrive_tab[Cont_Isect::tang  ][Cont_Isect::a_tang] = Cont_Isect::tang;
  arrive_tab[Cont_Isect::tang  ][Cont_Isect::left  ] = Cont_Isect::tang;
  arrive_tab[Cont_Isect::tang  ][Cont_Isect::right ] = Cont_Isect::tang;

  arrive_tab[Cont_Isect::a_tang][Cont_Isect::tang  ] = Cont_Isect::tang;
  arrive_tab[Cont_Isect::a_tang][Cont_Isect::a_tang] = Cont_Isect::a_tang;
  arrive_tab[Cont_Isect::a_tang][Cont_Isect::left  ] = Cont_Isect::left;
  arrive_tab[Cont_Isect::a_tang][Cont_Isect::right ] = Cont_Isect::right;

  arrive_tab[Cont_Isect::left  ][Cont_Isect::tang  ] = Cont_Isect::right;
  arrive_tab[Cont_Isect::left  ][Cont_Isect::a_tang] = Cont_Isect::a_tang;
  arrive_tab[Cont_Isect::left  ][Cont_Isect::left  ] = Cont_Isect::right;
  arrive_tab[Cont_Isect::left  ][Cont_Isect::right ] = Cont_Isect::right;

  arrive_tab[Cont_Isect::right ][Cont_Isect::tang  ] = Cont_Isect::left; 
  arrive_tab[Cont_Isect::right ][Cont_Isect::a_tang] = Cont_Isect::a_tang;
  arrive_tab[Cont_Isect::right ][Cont_Isect::left  ] = Cont_Isect::left; 
  arrive_tab[Cont_Isect::right ][Cont_Isect::right ] = Cont_Isect::left; 

  depart_tab[Cont_Isect::tang  ][Cont_Isect::tang  ] = Cont_Isect::tang;
  depart_tab[Cont_Isect::tang  ][Cont_Isect::a_tang] = Cont_Isect::tang;
  depart_tab[Cont_Isect::tang  ][Cont_Isect::left  ] = Cont_Isect::right;
  depart_tab[Cont_Isect::tang  ][Cont_Isect::right ] = Cont_Isect::left;

  depart_tab[Cont_Isect::a_tang][Cont_Isect::tang  ] = Cont_Isect::a_tang;
  depart_tab[Cont_Isect::a_tang][Cont_Isect::a_tang] = Cont_Isect::a_tang;
  depart_tab[Cont_Isect::a_tang][Cont_Isect::left  ] = Cont_Isect::a_tang;
  depart_tab[Cont_Isect::a_tang][Cont_Isect::right ] = Cont_Isect::a_tang;

  depart_tab[Cont_Isect::left  ][Cont_Isect::tang  ] = Cont_Isect::tang;
  depart_tab[Cont_Isect::left  ][Cont_Isect::a_tang] = Cont_Isect::left;
  depart_tab[Cont_Isect::left  ][Cont_Isect::left  ] = Cont_Isect::right;
  depart_tab[Cont_Isect::left  ][Cont_Isect::right ] = Cont_Isect::left;

  depart_tab[Cont_Isect::right ][Cont_Isect::tang  ] = Cont_Isect::tang; 
  depart_tab[Cont_Isect::right ][Cont_Isect::a_tang] = Cont_Isect::right;
  depart_tab[Cont_Isect::right ][Cont_Isect::left  ] = Cont_Isect::right;
  depart_tab[Cont_Isect::right ][Cont_Isect::right ] = Cont_Isect::left; 
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void oth_arr_dep(Cont_Isect::Rel_Dir arr,
                        Cont_Isect::Rel_Dir dep,
                        Cont_Isect::Rel_Dir& o_arr,
                        Cont_Isect::Rel_Dir& o_dep)
{
  if (!arr_dep_tab_set) set_arr_dep_tabs();
  
  o_arr = arrive_tab[arr][dep];
  o_dep = depart_tab[arr][dep];
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Isect::Eval_Sides()
{
  if (Mark) return;
  
  const Vec2& tg_bef = Pnt.Tang_Before_XY();
  const Vec2& tg_aft = Pnt.Tang_After_XY ();

  Vec2 o_tg_bef(Other->Pnt.Tang_Before_XY());
  Vec2 o_tg_aft(Other->Pnt.Tang_After_XY ());

  Vec2 nrm_bef(tg_bef); nrm_bef.rot90();
  Vec2 nrm_aft(tg_aft); nrm_aft.rot90();

  double inprod_bef = nrm_bef * o_tg_bef;
  double inprod_aft = nrm_aft * o_tg_bef;

  // Determine arrival side

  if (fabs(inprod_bef) < Vec2::IdentDir &&
                         (tg_bef * o_tg_bef) > 0.0) arrive = tang;

  else if (fabs(inprod_aft) < Vec2::IdentDir &&
                         (tg_aft * o_tg_bef) < 0.0) arrive = a_tang;

  else if (nrm_bef * tg_aft >= 0.0) { // Transition to the left
    if (inprod_bef < 0.0 && inprod_aft < 0.0) arrive = left;
    else                                      arrive = right;
  }
  else {  // Transition to the right
    if (inprod_bef > 0.0 && inprod_aft > 0.0) arrive = right;
    else                                      arrive = left;
  }


  // Determine departure side

  inprod_bef = nrm_bef * o_tg_aft;
  inprod_aft = nrm_aft * o_tg_aft;

  if (fabs(inprod_bef) < Vec2::IdentDir &&
                       (tg_bef * o_tg_aft) < 0.0) depart = a_tang;
  else if (fabs(inprod_aft) < Vec2::IdentDir &&
                       (tg_aft * o_tg_bef) > 0.0) depart = tang;

  else if (nrm_bef * tg_aft >= 0.0) { // Transition to the left
    if (inprod_bef < 0.0 && inprod_aft < 0.0) depart = right;
    else                                      depart = left;
  }
  else {  // Transition to the right
    if (inprod_bef > 0.0 && inprod_aft > 0.0) depart = left;
    else                                      depart = right;
  }


  // Fill in the data for "Other"
  // See logic tables above

  oth_arr_dep(arrive,depart,Other->arrive,Other->depart);

  Mark = true;
  Other->Mark = true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Isect::Eval2_Sides()
{
  const Vec2& tg_bef = Pnt.Tang_Before_XY();
  const Vec2& tg_aft = Pnt.Tang_After_XY ();

  Vec2 o_tg_bef(Other->Pnt.Tang_Before_XY());
  Vec2 o_tg_aft(Other->Pnt.Tang_After_XY ());

  Vec2 nrm_bef(tg_bef); nrm_bef.rot90();
  Vec2 nrm_aft(tg_aft); nrm_aft.rot90();

  double inprod_bef = nrm_bef * o_tg_bef;
  double inprod_aft = nrm_aft * o_tg_bef;

  // Determine arrival side

  if (fabs(inprod_bef) < Vec2::IdentDir &&
                         (tg_bef * o_tg_bef) > 0.0) arrive = tang;

  else if (fabs(inprod_aft) < Vec2::IdentDir &&
                         (tg_aft * o_tg_bef) < 0.0) arrive = a_tang;

  else if (nrm_bef * tg_aft >= 0.0) { // Transition to the left
    if (inprod_bef < 0.0 && inprod_aft < 0.0) arrive = left;
    else                                      arrive = right;
  }
  else {  // Transition to the right
    if (inprod_bef > 0.0 && inprod_aft > 0.0) arrive = right;
    else                                      arrive = left;
  }


  // Determine departure side

  inprod_bef = nrm_bef * o_tg_aft;
  inprod_aft = nrm_aft * o_tg_aft;

  if (fabs(inprod_bef) < Vec2::IdentDir &&
                       (tg_bef * o_tg_aft) < 0.0) depart = a_tang;
  else if (fabs(inprod_aft) < Vec2::IdentDir &&
                       (tg_aft * o_tg_bef) > 0.0) depart = tang;

  else if (nrm_bef * tg_aft >= 0.0) { // Transition to the left
    if (inprod_bef < 0.0 && inprod_aft < 0.0) depart = right;
    else                                      depart = left;
  }
  else {  // Transition to the right
    if (inprod_bef > 0.0 && inprod_aft > 0.0) depart = left;
    else                                      depart = right;
  }
}

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
/* ------- Allocation for Isect_New_Cursor ------------------------------ */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> > Isect_Cursor_Alloc;

void *Cont_Isect_Cursor::operator new(size_t /*sz*/)
{
  return Isect_Cursor_Alloc::New();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont_Isect_Cursor::operator delete(void *oc)
{
  Isect_Cursor_Alloc::Free((IT_D_Item<Cont_Isect_Cursor> *)oc);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

int Cont_Isect_Cmp::Compare(const Cont_Isect& i1, const Cont_Isect& i2)
{
  if   (i1.Pnt.Par() < i2.Pnt.Par()) return -1;
  else                               return  1;
}

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
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void remove_inconsistencies(bool go_left, bool closed,
                                   Cont_Isect_D_List& ilist)
{
  // Once offset, if two adjacent intersections have the same
  // transition sense, one must be deleted.
  // This is required allthough principally not enough to guarantee
  // that correct contours will be found

  Cont_Isect_D_Cursor isc(ilist);

  if (!closed) ++isc;

  while (isc) {
    Cont_Isect *pred = isc.Pred();

    if (pred == &(*isc)) break;

    if (isc->To_Left == pred->To_Left) {
      if (isc->To_Left == go_left) {
        isc->Other.Delete();
        isc.Delete();
      }
      else { // Throw away predecessor
        --isc; if (!isc) isc.To_Last();

        isc->Other.Delete();
        isc.Delete();

        if (!isc) isc.To_Begin();

        ++isc;
      }
    }
    else ++isc;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Cont_I_Isect_List ---------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::Advance(Cont_Isect_D_Cursor& isc)
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

void Cont1_Isect_List::Backup(Cont_Isect_D_Cursor& isc)
{
  const Contour* cont = isc->Pnt.Parent_Contour();

  --isc; if (!isc) isc.To_Last(); if (!isc) return;

  while (isc->Pnt.Parent_Contour() != cont) {
    --isc; if (!isc) isc.To_Last();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
static int index_of(const Cont_Isect_D_Cursor& isc)
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


static void do_make_a_print(FILE *fd, Cont_Isect_D_List& ilist)
{
  if (!fd) return;

   Cont_Isect_D_Cursor isc(ilist);

   fprintf(fd," Nr Par         RelPar        X            Y         -->Nr  a d\n");
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
          
     fprintf(fd,"%3d  %7.2f %12.5f %12.5f %12.5f %3d  %c %c ,(%c,%c)\n",
                 isc->id_no,isc->Pnt.Par(),isc->Pnt.Rel_Par(),
                         isc->Pnt.P().x,isc->Pnt.P().y,
                                       isc->Other->id_no,atg,dtg,ma,md);
//      idx++;
   }
}
#endif

void print_isect(FILE *fd, int il, const Cont_Isect_D_Cursor& isc)
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
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::delete_phantom_intersections()
{
  Cont_Isect_D_Cursor isbc1(ilist);
  
  while (isbc1) {
    Cont_Isect& is = *isbc1;
    
    if (!is.In_Range() && !is.Other->In_Range()) {
      double pdist;
      is.Pnt.Min_Par_Dist_To(is.Other->Pnt,pdist);

      if (fabs(pdist) < 2.1*Vec2::IdentDist) {
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

static void mark_for_tangent_analysis(const Cont_Isect_D_Cursor& isc,
                                                            double minlen)
{
  if (!isc) return;
  
  // Analyze if:
  // a. arrive or depart == tangent/anti_tangent
  // b. the intersection is near start- or endpoint of element
  //    (possibly out-of-range)
  // c. The arriving/departing stretch is very short
  
  Cont_Isect& is     = *isc;
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

bool Cont1_Isect_List::set_analysis_range_before(
                                            const Cont_Isect_D_Cursor& isc)
{
  if (!isc)       return false;
  if (isc->St_Pt) return true; /* Already defined */ 
  
  Cont_Isect_D_Cursor predc(isc); Backup(predc);

  Cont_Isect& is     = *isc;
  Cont_Isect& prv_is = *predc;
  
  // Determine the range upto the intersection
  
  is.St_Pt  = prv_is.Pnt;
  is.Full_Range_Arrive = true;
  
  if (predc == isc->Other) {
    // Then this is an offset operation and the intersection is its own
    // predecessor, we split the range in half

    is.St_Pt.Become_Midpoint_Of(is.St_Pt,is.Pnt);
    is.St_Pt.Become_Midpoint_Of(is.St_Pt,is.Pnt);
    is.Full_Range_Arrive = false;
  }

  // Now determine the longest contiguous range ending at the intersection
  // but only for offset operations, combines are always contiguous, so:
  
  Elem_C_Cursor curc(is.Pnt);

  double tol = sqr(Vec2::IdentDist);

  while (curc != is.St_Pt) {
    Elem_C_Cursor prvc(curc); --prvc; if (!prvc) prvc.To_Last();

    if (prvc->El().P2().sqDistTo2(curc->El().P1()) > tol) {
      is.St_Pt.elemc = curc;
      is.St_Pt.rel_par = 0.0;
      is.St_Pt.pt = curc->El().P1();
      is.St_Pt.calc_point_attr();

      is.Full_Range_Arrive = false;

      break;
    }

    curc = prvc;
  }
  
  bool has_length = true;
  
  if (curc == is.Pnt && is.Pnt.Rel_Par() < Vec2::IdentDist) has_length=false;

  if (!prv_is.End_Pt && is.Full_Range_Arrive) {
    prv_is.End_Pt = is.Pnt;
    prv_is.Full_Range_Depart = true;
  }

  return has_length;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::set_analysis_range_after(
                                            const Cont_Isect_D_Cursor& isc)
{
  if (!isc)        return false;
  if (isc->End_Pt) return true; /* Already defined */
  
  Cont_Isect_D_Cursor nextc(isc); Advance(nextc);

  Cont_Isect& is     = *isc; 
  Cont_Isect& nxt_is = *nextc;
  
  // Determine the range starting at the intersection
  
  is.End_Pt = nxt_is.Pnt;
  is.Full_Range_Depart = true;

  if (nextc == isc->Other) {
    // Then this is an offset operation and the intersection is its own
    // predecessor, we split the range in half
    is.End_Pt.Become_Midpoint_Of(is.Pnt,is.End_Pt);
    is.End_Pt.Become_Midpoint_Of(is.Pnt,is.End_Pt);
    is.Full_Range_Depart = false;
  }
  
  // Now determine the longest contiguous range starting at the intersection
  // but only for offset operations, combines are always contiguous, so:
  
  Elem_C_Cursor curc(is.Pnt);

  double tol = sqr(Vec2::IdentDist);

  while (curc != is.End_Pt) {
    Elem_C_Cursor nxtc(curc); ++nxtc; if (!nxtc) nxtc.To_Begin();

    if (nxtc->El().P1().sqDistTo2(curc->El().P2()) > tol) {
      is.End_Pt.elemc = curc;
      is.End_Pt.rel_par = curc->El().Par_Len();
      is.End_Pt.pt = curc->El().P2();
      is.End_Pt.calc_point_attr();

      is.Full_Range_Depart = false;

      break;
    }

    curc = nxtc;
  }
  
  bool has_length = true;
  if (curc == is.Pnt &&
      is.Pnt.Rel_Par() >= curc->El().Par_Len()-Vec2::IdentDist)
                                                      has_length = false;
  
  if (!nxt_is.St_Pt && is.Full_Range_Depart) {
    nxt_is.St_Pt = is.Pnt;
    nxt_is.Full_Range_Arrive = true;
  }
  
  return has_length;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::find_rel_pos_arrive(const Cont_PPair& p1,
                                            double /*isp_par*/,
                                            const Cont_PPair& p2,
                                            double tol,
                                            Cont_Isect::Rel_Dir& dir)
{
  // project points on range p2 onto range p1

  Cont_Pnt pnt, last_pnt;
  double dist_xy, last_par=0.0;
  bool last_on_mid = false;

  Elem_C_Cursor last_elc;
  Elem_C_Cursor elc(p2.two);
  
  bool spanning = false;
  if (elc == p2.one && p2.one.Par() > p2.two.Par()) {
    // This is a (rare) special case, where element elc spans
    // from the END to the START of the range!
    
    spanning = true;
  }

  // Project all start- and midpoints within the range
  
  for (;;) {
    const Elem& el = elc->El();
    
    double bpar,epar;
    
    bool in_range = false;
    
    if (spanning) in_range = p2.Range_Inside_2(el, bpar, epar);
    else          in_range = p2.Range_Inside_1(el, bpar, epar);

    if (in_range) {

      double midpar = (bpar+epar)/2.0;
      Vec3 mp;
      el.At_Par(midpar,mp);

      if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
        last_elc = elc; last_par = midpar; last_on_mid = true;
        last_pnt = pnt;

        if (fabs(dist_xy) > tol) {
          if (dist_xy > 0.0) dir = Cont_Isect::left;
          else               dir = Cont_Isect::right;

          return true;
        }
      }
      else break;

      el.At_Par(bpar,mp);

      if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
          last_elc = elc; last_par = bpar; last_on_mid = false;
          last_pnt = pnt;

          if (fabs(dist_xy) > tol) {
            if (dist_xy > 0.0) dir = Cont_Isect::left;
            else               dir = Cont_Isect::right;

            return true;
          }
        }
      }
      else break;

    
    if (elc.Container()->Length() < 2) spanning = false;
 
    if (!spanning && elc == p2.one) break;

    spanning = false;
    
    --elc; if (!elc) elc.To_Last();
  }


  // Lets try and find the ultimate point that projects ok
  // then that point defines the result
  
  double lwb_par, upb_par;
  Elem_C_Cursor relc(last_elc);
  
  if (last_elc) {
    if (last_on_mid) {
      upb_par = last_par;
      lwb_par = relc->El().Begin_Par();
    }
    else {
      --relc; if (!relc) relc.To_Last();
      upb_par = relc->El().End_Par();
      lwb_par = relc->El().Mid_Par();
    }
  }
  else {
    upb_par = p2.two.Par();
    relc    = p2.two;
    if (p2.two.Rel_Par() < Vec2::IdentDist) {
      --relc; if (!relc) relc.To_Last();
      upb_par = relc->El().End_Par();
    }
    lwb_par  = relc->El().Begin_Par();
  }

  if (lwb_par < p2.one.Par()) lwb_par = p2.one.Par();

  // Now the ultimate point must lie on element relc and between
  // lwbpar and upbpar, use binary search to find it

  const Elem& el= relc->El();

  while (lwb_par < upb_par - Vec2::IdentDist) {
    double mpar = (lwb_par + upb_par)/2.0;

    Vec3 mp;
    el.At_Par(mpar,mp);
    
    if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
      last_elc = elc; last_par = mpar;
      last_pnt = pnt;

      if (fabs(dist_xy) > tol) {
        if (dist_xy > 0.0) dir = Cont_Isect::left;
        else               dir = Cont_Isect::right;

        return true;
      }
      
      upb_par = mpar;
    }
    else lwb_par = mpar;
  }

  if (!last_elc) return false; // Cant find it, leave dir unmodified!

  // Ok, we have a tang/anti_tang solution, leave solution as is
  
#ifdef NEVER

  // The solution below may give the wrong answer if lpar is close
  // to p1.one.Par() and p1.Is_Full() == true!!!
  
  double lpar = last_pnt.Par();

  if (p1.one.Par() < isp_par) {
    if (p1.one.Par() <= lpar && lpar <= isp_par) dir = Cont_Isect::tang;
    else                                         dir = Cont_Isect::a_tang;
  }
  else {
    if (p1.one.Par() <= lpar || lpar <= isp_par) dir = Cont_Isect::tang;
    else                                         dir = Cont_Isect::a_tang;
  }
#endif
  
  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::find_rel_pos_depart(const Cont_PPair& p1,
                                            double /*isp_par*/,
                                            const Cont_PPair& p2,
                                            double tol,
                                            Cont_Isect::Rel_Dir& dir)
{
  // project points on range p2 onto range p1

  Cont_Pnt pnt, last_pnt;
  double dist_xy, last_par=0.0;
  bool last_on_mid = false;

  Elem_C_Cursor last_elc;
  Elem_C_Cursor elc(p2.one);
  
  bool spanning = false;
  if (elc == p2.two && p2.one.Par() > p2.two.Par()) {
    // This is a (rare) special case, where element elc spans
    // from the END to the START of the range!
    
    spanning = true;
  }

  // Project all start- and midpoints within the range
  
  for (;;) {
    const Elem& el = elc->El();
    
    double bpar,epar;
    
    bool in_range = false;
    
    if (spanning) in_range = p2.Range_Inside_1(el, bpar, epar);
    else          in_range = p2.Range_Inside_2(el, bpar, epar);

    if (in_range) {

      double midpar = (bpar+epar)/2.0;
      Vec3 mp;
      el.At_Par(midpar,mp);

      if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
        last_elc = elc; last_par = midpar; last_on_mid = true;
        last_pnt = pnt;

        if (fabs(dist_xy) > tol) {
          if (dist_xy > 0.0) dir = Cont_Isect::left;
          else               dir = Cont_Isect::right;

          return true;
        }
      }
      else break;


      el.At_Par(epar,mp);
      if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
        last_elc = elc; last_par = epar; last_on_mid = false;
        last_pnt = pnt;

        if (fabs(dist_xy) > tol) {
          if (dist_xy > 0.0) dir = Cont_Isect::left;
          else               dir = Cont_Isect::right;

          return true;
        }
      }
      else break;
    }
    
    if (elc.Container()->Length() < 2) spanning = false;
 
    if (!spanning && elc == p2.two) break;

    spanning = false;

    ++elc; if (!elc) elc.To_Begin();
  }


  // Lets try and find the ultimate point that projects ok
  // then that point defines the result
  
  double lwb_par, upb_par;
  Elem_C_Cursor relc(last_elc);
  
  if (last_elc) {
    if (last_on_mid) {
      lwb_par = last_par;
      upb_par = relc->El().End_Par();
    }
    else {
      ++relc; if (!relc) relc.To_Begin();
      lwb_par = relc->El().Begin_Par();
      upb_par = relc->El().Mid_Par();
    }
  }
  else {
    lwb_par = p2.one.Par();
    relc    = p2.one;
    if (p2.one.Rel_Par() > relc->El().Par_Len()-Vec2::IdentDist) {
      ++relc; if (!relc) relc.To_Begin();
      lwb_par = relc->El().Begin_Par();
    }
    upb_par  = relc->El().End_Par();
  }

  if (upb_par > p2.two.Par()) upb_par = p2.two.Par();

  // Now the ultimate point must lie on element relc and between
  // lwbpar and upbpar, use binary search to find it

  const Elem& el = relc->El();

  while (lwb_par < upb_par - Vec2::IdentDist) {
    double mpar = (lwb_par + upb_par)/2.0;

    Vec3 mp;
    el.At_Par(mpar,mp);
    
    if (p1.Project_Pnt_XY(mp,pnt,dist_xy)) {
      last_elc = elc; last_par = mpar;
      last_pnt = pnt;

      if (fabs(dist_xy) > tol) {
        if (dist_xy > 0.0) dir = Cont_Isect::left;
        else               dir = Cont_Isect::right;

        return true;
      }
      
      lwb_par = mpar;
    }
    else upb_par = mpar;
  }

  if (!last_elc) return false; // Cant find it, leave dir unmodified!

  // Ok, we have a tang/anti_tang solution, leave solution as is
  
#ifdef NEVER

  // The solution below may give the wrong answer if lpar is close
  // to p1.one.Par() and p1.Is_Full() == true!!!
  
  double lpar = last_pnt.Par();

  if (p1.one.Par() < isp_par) {
    if (p1.one.Par() <= lpar && lpar <= isp_par) dir = Cont_Isect::a_tang;
    else                                         dir = Cont_Isect::tang;
  }
  else {
    if (p1.one.Par() <= lpar || lpar <= isp_par) dir = Cont_Isect::a_tang;
    else                                         dir = Cont_Isect::tang;
  }
#endif
  
  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::analyze_arrive(FILE * /*fd*/,int /*il*/,
                                       const Cont_Isect_D_Cursor& isc)
{
  if (!isc) return;

  Cont_Isect& is     = *isc;
  Cont_Isect& oth_is = *isc->Other;

  Cont_Isect_D_Cursor oth_predc(is.Other); Backup(oth_predc);
  
  if (!set_analysis_range_before(is.Other)) {

#ifdef DEBUG_PRINT
    fprintf(fd,"Geen range before: \n");
    print_isect(fd,il,isc);
#endif

    return;
  }

  // Now determine left/right/coincident situation

  Cont_PPair my_range(is.St_Pt,is.End_Pt);
  Cont_PPair oth_range(oth_is.St_Pt,oth_is.Pnt);

  // my_range may be the entire contour
  if (is.Full_Range_Arrive && is.Full_Range_Depart) {
    Cont_Isect_D_Cursor prvc(isc); Backup(prvc);
    Cont_Isect_D_Cursor nxtc(isc); Advance(nxtc);

    if (prvc == nxtc) my_range.Is_Full(true);
  }
  
  if (!find_rel_pos_arrive(my_range, isc->Pnt.Par(), oth_range,
                                Vec2::IdentDist, is.arrive)) {

#ifdef DEBUG_PRINT
    fprintf(fd,"Arrive: Na find_rel_pos(false): \n");
    print_isect(fd,il,isc);
#endif

    return;
  }
                                          
  is.Mark_Arrive = false;
  
#ifdef DEBUG_PRINT
   fprintf(fd,"Arrive: Na find_rel_pos(true): \n");
   print_isect(fd,il,isc);
#endif
 
  // Ok, set, now try and save some analysis work:

  if (!oth_is.Full_Range_Arrive) return;

  Cont_Isect_D_Cursor predc(isc); Backup(predc);
  if (predc == isc) return;

  if (is.Full_Range_Arrive && predc->Other == oth_predc &&
                                                   predc->Mark_Depart) {

#ifdef DEBUG_PRINT
    fprintf(fd,"Setting predc->depart (%d,%d)\n",
                                    predc->Other->id_no,oth_predc->id_no);
#endif

    predc->depart = is.arrive;
    predc->Mark_Depart = false;
  }

  Cont_Isect_D_Cursor nxtc(isc); Advance(nxtc);
  if (nxtc == isc) return;

  if (is.Full_Range_Depart && nxtc->Other == oth_predc &&
                                                   nxtc->Mark_Depart) {

#ifdef DEBUG_PRINT
    fprintf(fd,"Setting nxtc->depart (%d,%d)\n",
                                    nxtc->Other->id_no,oth_predc->id_no);
#endif

    nxtc->depart = is.arrive;
    nxtc->Mark_Depart = false;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::analyze_depart(FILE * /*fd*/,int /*il*/,
                                       const Cont_Isect_D_Cursor& isc)
{
  if (!isc) return;
  
  // Now determine left/right/coincident situation

  Cont_Isect& is     = *isc;
  Cont_Isect& oth_is = *isc->Other;

  Cont_Isect_D_Cursor oth_nxtc(is.Other); Advance(oth_nxtc);
  
  if (!set_analysis_range_after(is.Other)) {
#ifdef DEBUG_PRINT
    fprintf(fd,"Geen range after: \n");
    print_isect(fd,il,isc);
#endif

    return;
  }

  // Now determine left/right/coincident situation

  Cont_PPair my_range(is.St_Pt,is.End_Pt);
  Cont_PPair oth_range(oth_is.Pnt,oth_is.End_Pt);
  
  // my_range may be the entire contour
  if (is.Full_Range_Arrive && is.Full_Range_Depart) {
    Cont_Isect_D_Cursor prvc(isc); Backup(prvc);
    Cont_Isect_D_Cursor nxtc(isc); Advance(nxtc);

    if (prvc == nxtc) my_range.Is_Full(true);
  }
  
  if (!find_rel_pos_depart(my_range, isc->Pnt.Par(), oth_range,
                                Vec2::IdentDist, is.depart)) {

#ifdef DEBUG_PRINT
    fprintf(fd,"depart, Na find_rel_pos (false): \n");
    print_isect(fd,il,isc);
#endif

    return;
  }

  is.Mark_Depart = false;

#ifdef DEBUG_PRINT
  fprintf(fd,"depart, Na find_rel_pos(true): \n");
  print_isect(fd,il,isc);
#endif

 
  // Ok, set, now try and save some analysis work:

  if (!oth_is.Full_Range_Depart) return;

  Cont_Isect_D_Cursor nxtc(isc); Advance(nxtc);

  if (is.Full_Range_Depart && nxtc->Other == oth_nxtc &&
                                                   nxtc->Mark_Arrive) {
#ifdef DEBUG_PRINT
    fprintf(fd,"Setting nxtc->arrive (%d,%d)\n",
                              nxtc->Other->id_no,oth_nxtc->id_no);
#endif

    nxtc->arrive = is.depart;
    nxtc->Mark_Arrive = false;
  }

  Cont_Isect_D_Cursor predc(isc); Backup(predc);

  if (is.Full_Range_Arrive && predc->Other == oth_nxtc &&
                                                  predc->Mark_Arrive) {
#ifdef DEBUG_PRINT
    fprintf(fd,"Setting predc->arrive (%d,%d)\n",
                              predc->Other->id_no,oth_nxtc->id_no);
#endif

    predc->arrive = is.depart;
    predc->Mark_Arrive = false;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::analyze_tangent_stretches(FILE* fd,double minlen)
{
  // Analyze tangency of stretches:
  // a. that are deemed tangent on either end so far
  // b. that start/end in an intersection that is out of range
  // c. if the stretch is very short ( < minlen)
  
  // The analysis is done by finding the max. distance between
  // the stretches of contour on either side.

  unmark_arrive_depart();

  Cont_Isect_D_Cursor isbc1(ilist);
  for (;isbc1;++isbc1) mark_for_tangent_analysis(isbc1,minlen);

#ifdef DEBUG_PRINT
   do_make_a_print(fd,ilist);
#endif

  for (isbc1.To_Begin();isbc1;++isbc1) {
    Cont_Isect& is = *isbc1;
    
    if (is.Mark_Arrive || is.Mark_Depart) {
      set_analysis_range_before(isbc1);
      set_analysis_range_after(isbc1);
    }
    
    if (is.Mark_Arrive) analyze_arrive(fd,1,isbc1);
    if (is.Mark_Depart) analyze_depart(fd,1,isbc1);
    
    // These calls may reset the mark on "Other" to avoid
    // duplicate analysis
  }

  // Now check and correct inconsistencies in tangency

  for (isbc1.To_Begin();isbc1;++isbc1) {
    Cont_Isect& is     = *isbc1;
    Cont_Isect& is_oth = *is.Other;
    
    Cont_Isect::Rel_Dir arr, dep;
    oth_arr_dep(is_oth.arrive,is_oth.depart,arr,dep);

    if (is.Mark_Arrive && (is.arrive == Cont_Isect::tang ||
                           is.arrive == Cont_Isect::a_tang)) {
      if (arr == Cont_Isect::left ||
          arr == Cont_Isect::right) is.arrive = arr;
    }

    if (is.Mark_Depart && (is.depart == Cont_Isect::tang ||
                           is.depart == Cont_Isect::a_tang)) {
      if (dep == Cont_Isect::left ||
          dep == Cont_Isect::right) is.depart = dep;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::sort_offset_coincident()
{
  // A pair of intersection may coicide (almost) in parametric
  // space when offsetting.
  // In that case the sort order of the pair in the intersection
  // list must be checked.

  // The idea is to sort them such that the jump to the "other"
  // isect is minimal.

  Cont_Isect_D_Cursor isc(ilist);
  
  while (isc) {
    Cont_Isect_D_Cursor predc(isc); Backup(predc);

    Cont_Isect& is     = *isc;
    Cont_Isect& prv_is = *predc;
        
    if (is.Other != predc) {
    
      double dist;
      if (is.Pnt.Min_Par_Dist_To(prv_is.Pnt,dist) &&
                                        fabs(dist) < Vec2::IdentDist) {

        double dist1, dist2;

        if (is.Pnt.Par_Dist_To(is.Other->Pnt,dist1) &&
            prv_is.Pnt.Par_Dist_To(prv_is.Other->Pnt,dist2) &&
                                                     dist1 > dist2) {

          // Ok, reverse isc with predc
          predc.Re_Insert(isc);   
          
          continue;
        }
      }
    }

    ++isc;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::special_offset_analyze()
{
  bool modified = false;

  Cont_Isect_D_Cursor isc(ilist);

  if (!isc) return;

  Contour *cnt = (Contour *)(isc->Pnt.Parent_Contour());

  while (isc) {
    Cont_Isect_D_Cursor predc(isc); Backup(predc);
    
    if (predc != isc->Other) {
      ++isc;
      continue;
    }
    
    Cont_Isect& is    = *isc;
    Cont_Isect& prvis = *predc;
    
    if (is.arrive == Cont_Isect::tang && is.depart == Cont_Isect::tang &&
        prvis.arrive == Cont_Isect::tang &&
        prvis.depart == Cont_Isect::tang &&
        !is.Full_Range_Arrive && !prvis.Full_Range_Depart) {
        
      modified = true;

      Elem_Cursor prvelc(cnt->el_list);
      while (prvelc && prvelc != prvis.Pnt) ++prvelc;
       
      Elem_Cursor elc(cnt->el_list);
      while (elc && elc != is.Pnt) ++elc;
      
      // Delete intermediate elements

      Elem_Cursor celc(prvelc);
      
      if (celc != elc) {
        ++celc; if (!celc) celc.To_Begin();
      }
      
      while (celc != elc) {
        celc.Delete();
        if (!celc) celc.To_Begin();
      }
      

      // Now clip the elements
      
      if (prvis.Pnt.Rel_Par() < 3.0 * Vec2::IdentDist) {
        celc = prvelc;
        --prvelc; if (!prvelc) prvelc.To_Last();
        
        Cont_Isect_D_Cursor lisc(ilist);
        while (lisc) {
          if (lisc != predc && lisc != isc && celc == lisc->Pnt) {
            lisc->Other.Delete();
            lisc.Delete();
          }
          else ++lisc;
        }

        Vec3 p1(celc->El().P1());
        
        celc.Delete();
        if (p1 == prvelc->El().P2())
                 prvelc->El().Stretch_End_XY(prvis.Pnt.P(),false);
      }
      else prvelc->El().Stretch_End_XY(is.Pnt.P(),false);
      
      if (is.Pnt.Rel_Par() > elc->El().Par_Len() - 3.0 * Vec2::IdentDist) {

        Cont_Isect_D_Cursor lisc(ilist);
        while (lisc) {
          if (lisc != predc && lisc != isc && elc == lisc->Pnt) {
            lisc->Other.Delete();
            lisc.Delete();
          }
          else ++lisc;
        }

        Vec3 p2(elc->El().P2());

        elc.Delete();
        if (!elc) elc.To_Begin();
        
        if (p2 == elc->El().P1())
                    elc->El().Stretch_Begin_XY(is.Pnt.P(),false);
      }
      else {
        Elem& el = elc->El();
        
        double old_bpar = el.Begin_Par();
        el.Stretch_Begin_XY(is.Pnt.P(),false);

        double bpar_diff = old_bpar - el.Begin_Par();
        
        Cont_Isect_D_Cursor lisc(ilist);
        while (lisc) {
          if (lisc != predc && lisc != isc && elc == lisc->Pnt) {
            lisc->Pnt.rel_par += bpar_diff;
          }

          ++lisc;
        }
        
      }
      
      is.Other.Delete();
      isc.Delete();
    }
    else ++isc;
  }

  if (modified) {
    cnt->inval_rects();
    cnt->calc_invar();
    
    // DO NOT reparametrize!!
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::cleanup_self(bool closed, bool make_consistent)
{
  Cont_Isect_D_Cursor isbc1(ilist);
  if (!isbc1) return;

  Cont_Isect_D_Cursor isec1(ilist); isec1.To_End();

  IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>::Sort(isbc1,isec1,
                                                        Cont_Isect_Cmp());

  // Delete phantom intersections, that is where both "sides" of the
  // intersection are out of range and where the parametric distance
  // between the sides is very small
  
  // Tijdelijk

    number_isects(ilist);

#ifdef DEBUG_PRINT
    FILE *fd = fopen("/tmp/clemens","w");

    fprintf(fd,"Initieel:\n\n");
    do_make_a_print(fd,ilist);
#else
      FILE *fd = NULL;
#endif


  if (make_consistent) delete_phantom_intersections();


  // Delete identical intersections
  
  delete_identical(2.0*Vec2::IdentDist);

  // Define sort order of coincident intersections
  sort_offset_coincident();

  unmark();

  isbc1.To_Begin(); // Evaluate intersections

  for (;isbc1;++isbc1) isbc1->Eval_Sides();
  
  // Now if there are any tangent intersections, analyze the adjacent
  // contour stretch, this is also done for out-of-range intersections
  // and very short stretches between intersections
  // stretches may be discontinuous, in that case limit the check
  // to the continuous stretches adjacent to the intersections

  // The result is that intersections only arrive or depart tangent
  // if the entire adjacent stretches are truely coincident.

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Eval_Sides:\n\n");
#endif

  analyze_tangent_stretches(fd,2.0*Vec2::IdentDist);

#ifdef DEBUG_PRINT
     fprintf(fd,"\nNa Analyze:\n\n");
     do_make_a_print(fd,ilist);
#endif

  // Special temporary measure:
  
//  special_offset_analyze();

  // Now remove intersections that dont crossover and also one of each
  // pair that encloses a tangent stretch, but where the pair does
  // effectively crossover.

  if (make_consistent) remove_non_crossing();
  
#ifdef DEBUG_PRINT
     fprintf(fd,"\nNa Remove_Non_Crossing:\n\n");
     do_make_a_print(fd,ilist);
#endif

  // At this point the intersections should be consistent in tangency

  
  set_to_left();
  
  if (make_consistent) remove_inconsistencies(left,closed,ilist);

#ifdef DEBUG_PRINT
   fprintf(fd,"\nNa Remove_Inconsistencies:\n\n");
  do_make_a_print(fd,ilist);
  fclose(fd);
#endif
}

/* ---------------------------------------------------------------------- */
/* --------Set boolean To_Left in the Cont_Isects ----------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::set_to_left()
{
  unmark();
  
  Cont_Isect_D_Cursor isbc1(ilist);
  
  for (;isbc1;++isbc1) isbc1->Set_To_Left();

  unmark();
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::remove_non_crossing()
{
  Cont_Isect_D_Cursor isbc1(ilist);
  
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

  // One side tangent intersections are set to crossover

  isbc1.To_Begin();

  while (isbc1) {
    Cont_Isect& is = *isbc1;

    if ((is.arrive == Cont_Isect::tang ||
                                is.arrive == Cont_Isect::a_tang)) {

        if (is.depart == Cont_Isect::left) is.arrive=Cont_Isect::right;
        else                               is.arrive=Cont_Isect::left;
    }

    ++isbc1;
  }

  isbc1.To_Begin();

  while (isbc1) {
    Cont_Isect& is = *isbc1;

    if ((is.depart == Cont_Isect::tang ||
                                is.depart == Cont_Isect::a_tang)) {

        if (is.arrive == Cont_Isect::left) is.depart=Cont_Isect::right;
        else                               is.depart=Cont_Isect::left;
    }

    ++isbc1;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::delete_identical(double tol)
{
  // Delete identical intersections,
  // where at least one is out of range

  tol = 2.1 * tol;  // Slightly more than the maximum possible difference

  Cont_Isect_D_Cursor isbc1(ilist);

  while (isbc1) {
    if (ilist.Length() < 2) break;

    bool modified = false;

    while (isbc1) {
      Cont_Isect_D_Cursor predc(isbc1); Backup(predc);

      if (predc == isbc1 || predc == isbc1->Other) {
        ++isbc1;
        continue;
      }
      
      Cont_Isect& is     = *isbc1;
      Cont_Isect& oth_is = *is.Other;
      
      Cont_Isect& prv_is     = *predc;
      Cont_Isect& oth_prv_is = *prv_is.Other;
      
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
          isbc1.Delete();
        }

        modified = true;
        continue;
      }

        
      ++isbc1;
    }

    if (modified) isbc1.To_Begin();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::
            intersect_el(Cont_Ref& cntref,
                         const Elem_Cursor& elc1,
                         const Elem_Cursor& elc2, double /*parlen*/)
{
  Isect_Lst pair_list;

  Elem &el1 = elc1->El();
  Elem &el2 = elc2->El();

  bool adjacent = false;

  if (elc1.Succ() == elc2.Self() ||
      elc1.Pred() == elc2.Self()) adjacent = true;

  // if (adjacent) el1.Intersect_XY(el2,true,pair_list); // Tangent Ok
  // else          el1.Intersect_XY(el2,pair_list);

  el1.Intersect_XY(el2,true,pair_list);

  if (!pair_list) return;

  // Insert results into list

  Isect_Cursor plc(pair_list);

  Cont_Isect_D_Cursor isc1(ilist);
  Cont_Isect_D_Cursor isc2(ilist);

  if (adjacent) {
    plc.To_Begin();
    if (plc->First.Par < el1.Begin_Par() + Vec2::IdentDist &&
        plc->Last.Par  > el2.End_Par()   - Vec2::IdentDist)
    {
      el1.Stretch_Begin_XY(plc->P);
      el2.Stretch_End_XY(plc->P);

      plc.Delete();
    }

    plc.To_Last(); if (!plc) return;

    if (plc->First.Par > el1.End_Par()   - Vec2::IdentDist &&
        plc->Last.Par  < el2.Begin_Par() + Vec2::IdentDist)
    {
      el1.Stretch_End_XY(plc->P);
      el2.Stretch_Begin_XY(plc->P);

      plc.Delete();
    }
  }

  plc.To_Begin();

  for (;plc;++plc) {
    Vec3 isp(plc->P);

    double rpar = plc->First.Par - el1.Begin_Par();

    Cont_Pnt pnt1(cntref.Cont,elc1,rpar,isp);
    pnt1.calc_point_attr();

    rpar = plc->Last.Par - el2.Begin_Par();
 
    Cont_Pnt pnt2(cntref.Cont,elc2,rpar,isp);
    pnt2.calc_point_attr();

    isc1.To_End(); isc1.Insert(Cont_Isect(&cntref,pnt1));
    isc2.To_End(); isc2.Insert(Cont_Isect(&cntref,pnt2));

    isc1->Other = isc2; isc2->Other = isc1;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::intersect_xy(Cont_Ref& cntref, bool one_only)
{
  Contour &cnt = (Contour &)(cntref.Cont);

  if (!cnt.el_rect_list) cnt.build_rect_list();

  Elem_Cursor el1(cnt.el_list);
  Elem_Cursor el2(cnt.el_list);

  if (!el1) return;

  double parlen = cnt.End_Par() - cnt.Begin_Par();

  Sub_Rect_C_Cursor rc1(cnt.el_rect_list->Begin());
  Sub_Rect_C_Cursor rc2(rc1);

  for (;rc1;++rc1) {
    const Rect_Ax& rct1 = rc1->Rect;

    rc2 = rc1;

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
                intersect_el(cntref,curel1,curel2,parlen);

                if (one_only && ilist) return;
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

void Cont1_Isect_List::unmark()
{
  Cont_Isect_D_Cursor isc(ilist);

  for (;isc;++isc) isc->Mark = false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::delete_marked()
{
  Cont_Isect_D_Cursor isc(ilist);

  while (isc) {
    if (isc->Mark) isc.Delete();
    else ++isc;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::unmark_arrive_depart()
{
  Cont_Isect_D_Cursor isc(ilist);

  for (;isc;++isc) {
    Cont_Isect& is = *isc;
    
    is.Mark_Arrive = false;
    is.Mark_Depart = false;

    is.Full_Range_Arrive = false;
    is.Full_Range_Depart = false;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Find just one intersection ----------------------------------- */
/* ---------------------------------------------------------------------- */

Cont1_Isect_List::Cont1_Isect_List(Cont_Ref& cntref, bool one_only)
  : ilist(), left(true), offsetref(&cntref), rlist(NULL)
{
  if (cntref.Cont.Empty()) return;

  intersect_xy(cntref, one_only);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont1_Isect_List::Cont1_Isect_List(Cont_Ref& cntref, bool closed,
                                       bool go_left, bool make_consistent)
  : ilist(), left(go_left), offsetref(&cntref), rlist(NULL)
{
  if (cntref.Cont.Empty()) return;

  intersect_xy(cntref);

  cleanup_self(closed,make_consistent);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::Intersections_Into(Cont_PPair_List& pplist)
{
  pplist.Delete();

  unmark();

  Cont_PPair pair;

  Cont_Isect_D_Cursor ilc(ilist);

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

static Vec2 last_gap;

const Vec2& Last_Gap()
{
  return last_gap;
}


static bool check_contiguous(const Elem_List& ellst, bool closed)
{
  Elem_C_Cursor elc(ellst);

  if (!closed) ++elc;
  if (!elc) return false;

  for (;elc;++elc) {
    const Elem& prvel = elc.Pred()->El();
    const Elem& curel = elc->El();

    if (prvel.P2() != curel.P1())
//    if (prvel.P2().Dist_To_2(curel.P1()) > 2.0*Vec2::Ident_Dist)
// Tijdelijk
     {
       last_gap = prvel.P2();

     //  double dist = prvel.P2().distTo2(curel.P1());

         return false;
     }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::Check_Against_Cont(const Cont_Clsd& org_cont,
                                                         double offdist)
{
  if (fabs(offdist) < Vec2::IdentDist) return;
  
  Cont_Isect_Cursor isc; isc = ilist.Begin();
  
  while (isc) {
    Cont_Pnt pnt; double dist;
    
    if (!org_cont.Project_Pnt_XY(isc->Pnt.P(),pnt,dist))
                                     Cont_Panic(ContIsect_Cant_Project);

    if (fabs(dist - offdist) > 2.0 * Vec2::IdentDist) {
      // Remove this intersection
      isc->Other.Delete();
      isc.Delete();
    }
    else ++isc;
  }
}
 
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::Extract_Offset_Open(Cont_D_List& /*cnt_list*/)
{
  // Hier verder
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool repair_forward(Elem_Cursor& lelc, Elem_Cursor& felc)
{
  if (!lelc || !felc) return false;
  
  const Vec3& endp = lelc->El().P2();
  
  while (felc) {
    Elem& el = felc->El();

    Vec3 pp;
    double parm, dist_xy;
    
    if (el.Project_Pnt_XY(endp, Vec2::IdentDist, true, pp, parm, dist_xy) &&
                                  fabs(dist_xy) < 2.0 * Vec2::IdentDist) {

      if (parm > el.End_Par()-Vec2::IdentDist) {
        felc.Delete();
        if (!felc) lelc->El().Stretch_End_XY(el.P2(),true);
        else {
          lelc->El().Join_To_XY(felc->El(),true);
          lelc.To_End();
          lelc.Re_Insert(felc);
        }
      }
      else {
        switch (el.Type()) {
          case Elem_Type_Line: {
               Elem_Line el_line((Elem_Line&)el);
               el_line.Stretch_Begin_XY(pp,false);
               lelc->El().Join_To_XY(el_line,true);

               lelc.To_End(); lelc.Insert(el_line);
            }
          break;

          case Elem_Type_Arc: {
               Elem_Arc el_arc((Elem_Arc&)el);
               el_arc.Stretch_Begin_XY(pp,false);
               lelc->El().Join_To_XY(el_arc,true);

               lelc.To_End(); lelc.Insert(el_arc);
            }
          break;

          default:
          return false;
        }

        felc.Delete();
      }
      
      return true;
    }
    else felc.Delete();
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool repair_backward(Elem_Cursor& lelc, Elem_Cursor& felc)
{
  if (!lelc || !felc) return false;
  
  const Vec3& begp = felc->El().P1();
  
  while (lelc) {
    Elem& el = lelc->El();

    Vec3 pp;
    double parm, dist_xy;
    
    if (el.Project_Pnt_XY(begp, Vec2::IdentDist, true, pp, parm, dist_xy) &&
                                  fabs(dist_xy) < 2.0 * Vec2::IdentDist) {

      if (parm < el.Begin_Par()+Vec2::IdentDist) {
        lelc.Delete(); lelc.To_Last();

        if (!lelc) felc->El().Stretch_Begin_XY(el.P1(),true);
        else {
          lelc->El().Join_To_XY(felc->El(),true);
          lelc.To_End();
          lelc.Re_Insert(felc);
        }
      }
      else {
        switch (el.Type()) {
          case Elem_Type_Line: {
               Elem_Line el_line((Elem_Line&)el);
               el_line.Stretch_End_XY(pp,false);
               el_line.Join_To_XY(felc->El(),true);

               lelc.Delete(); lelc.Insert(el_line);
            }
          break;

          case Elem_Type_Arc: {
               Elem_Arc el_arc((Elem_Arc&)el);
               el_arc.Stretch_End_XY(pp,false);
               el_arc.Join_To_XY(felc->El(),true);

               lelc.Delete(); lelc.Insert(el_arc);
            }
          break;

          default:
          return false;
        }

        lelc.Re_Insert(felc);
      }
      
      return true;
    }
    else ++felc;
  }

  return false;

}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::analyze_contiguous(const Cont_Clsd& org,
                                          double offdist,
                                          Cont_Isect_Cursor& isc,
                                          Contour& newpiece)
{
  newpiece.Delete();

  if (!isc) return false;
  
  const Cont_Pnt& pnt1 = isc.Pred()->Pnt;
  const Cont_Pnt& pnt2 = isc->Pnt;

  pnt1.Extract_Upto(pnt2,newpiece);
  if (newpiece.Empty()) return true;
  
  Elem_List ellst;
  newpiece.Move_To(ellst);
  
  Elem_Cursor felc(ellst);
  Elem_Cursor lelc(newpiece.el_list);
  
  bool repaired = false;
  
  while (felc) {
    if (lelc) {
      Elem& el1 = lelc->El();
      Elem& el2 = felc->El();
      
      if (el1.P2().distTo2(el2.P1()) > Vec2::IdentDist) {

        Cont_Pnt pp;
        double dist_xy;

        if (org.Project_Pnt_XY(el1.P2(),pp,dist_xy) &&
                    fabs(dist_xy-offdist) < 2.0*Vec2::IdentDist) {

          if (repair_forward(lelc,felc)) {
            repaired = true;
            continue;
          }
        }
        else if (org.Project_Pnt_XY(el2.P1(),pp,dist_xy) &&
                    fabs(dist_xy-offdist) < 2.0*Vec2::IdentDist) {

          if (repair_backward(lelc,felc)) {
            repaired = true;
            continue;
          }
        }

        // Cant repair, so quit
        newpiece.Delete();

        return false;
      }
    }
    
    lelc.To_End();
    lelc.Re_Insert(felc);
  }

  if (lelc->El().P2().distTo2(pnt2.P()) > Vec2::IdentDist) {
    newpiece.Delete();

    return false;
  }
  
  if (repaired) { // Do a full size check!
  }

  newpiece.calc_invar();
  newpiece.Begin_Par(pnt1.Par());

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Cont1_Isect_List::Extract_Closed(Cont_Clsd_D_List& cnt_list)
{
  cnt_list.Delete();

  // Hunt for intersected parts

  unmark();

  Cont_Clsd_Cursor cc(cnt_list);

  if (!ilist && offsetref) {
    cc.Insert(Cont_Clsd());

    cc->cont = offsetref->Cont;
    cc->lccw = offsetref->Ccw;

    return;
  }

  while (ilist) {
    Cont_Isect_Cursor isc; isc = ilist.Begin();

    // Hunt for closed loop

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) isc = isc->Other;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    unmark();

    Contour newc;
    Elem_Cursor elc(newc.el_list);

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) {
        Contour newpiece;

        isc.Pred()->Pnt.Extract_Upto(isc->Pnt, newpiece);
        
        elc.To_Last();

        if (!newpiece.Empty()) {
           newpiece.el_list.Append_To(newc.el_list);

           if (elc && elc.Succ()) elc->El().Join_To_XY(elc.Succ()->El());
        }
      }

      isc = isc->Other;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    elc.To_Last();
    if (elc) elc->El().Join_To_XY(elc.Succ()->El());

    if (!newc.Empty()) {
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

void Cont1_Isect_List::Extract_Offset_Closed(const Cont_Clsd& org,
                                              double offdist,
                                               Cont_Clsd_D_List& cnt_list)
{
  cnt_list.Delete();

  // Hunt for intersected parts

  unmark();

  Cont_Clsd_Cursor cc(cnt_list);

  if (!ilist && offsetref && check_contiguous(offsetref->Cont,true)) {
    cc.Insert(Cont_Clsd());

    cc->cont = offsetref->Cont;
    cc->lccw = offsetref->Ccw;

    return;
  }

  while (ilist) {
    Cont_Isect_Cursor isc; isc = ilist.Begin();

    // Hunt for closed loop

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) isc = isc->Other;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    unmark();

    Contour newc;
    Elem_Cursor elc(newc.el_list);

    bool contiguous = true;

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) {
        if (contiguous) {

          Contour newpiece;

          if (org.Empty()) {
            if (!isc.Pred()->Pnt.Contiguous_Upto(isc->Pnt)) {
              contiguous = false;
              break;
            }
          }
          else {
            if (!analyze_contiguous(org,offdist,isc,newpiece)) {
              contiguous = false;
              break;
            }
          }

          isc.Pred()->Pnt.Extract_Upto(isc->Pnt, newpiece);
          
          elc.To_Last();

          if (!newpiece.Empty()) {
             newpiece.el_list.Append_To(newc.el_list);

             if (elc && elc.Succ()) elc->El().Join_To_XY(elc.Succ()->El());
          }
        }

        isc = isc->Other;
      }
      else contiguous = false;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    if (contiguous) {
      elc.To_Last();
      if (elc) elc->El().Join_To_XY(elc.Succ()->El());

      if (!newc.Empty() && check_contiguous(newc.el_list,true)) {
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
    }

    delete_marked();
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static bool repair_gap(Elem_Cursor& elc, double tol)
{
  Elem& curel = elc->El();
  Elem& prvel = elc.Pred()->El();

  Vec3 pp;
  double parm, dist_xy;

  if (prvel.Project_Pnt_XY(curel.P1(),Vec2::IdentDist,true,
                                   pp,parm,dist_xy) && fabs(dist_xy) < tol) {

    if (parm < prvel.Begin_Par()+2.0*Vec2::IdentDist) {
      Vec3 begp = prvel.P1();

      Elem_Cursor pelc(elc); --pelc; pelc.Delete();
      curel.Stretch_Begin_XY(begp,false);
    }
    else {
      prvel.Stretch_End_XY(pp,false);
      curel.Stretch_Begin_XY(pp,false);
    }
  }
  else if (curel.Project_Pnt_XY(prvel.P2(),Vec2::IdentDist,true,
                                   pp,parm,dist_xy) && fabs(dist_xy) < tol) {

    if (parm > curel.End_Par()-2.0*Vec2::IdentDist) {
      Vec3 endp = curel.P2();

      Elem_Cursor pelc(elc); --pelc; elc.Delete(); elc = pelc;

      prvel.Stretch_End_XY(endp,false);
    }
    else {
      prvel.Stretch_End_XY(pp,false);
      curel.Stretch_Begin_XY(pp,false);
    }
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::compile_closed_cont(double tol, Cont_Clsd& cnt)
{
  cnt.cont.el_list = offsetref->Cont;
  cnt.Begin_Par(0.0);

  Elem_Cursor elc(cnt.cont.el_list);

  bool contiguous = true;

  while (elc) {
    Elem& prvel = elc.Pred()->El();
    Elem& curel = elc->El();

    double dist = prvel.P2().distTo2(curel.P1());

    if (dist < 2.0 * Vec2::IdentDist) prvel.Join_To_XY(curel,false);
    else if (!repair_gap(elc,tol)) contiguous = false;
    
    ++elc;
  }

  cnt.cont.inval_rects();
  cnt.calc_invar();
  cnt.Begin_Par(0.0);

  return contiguous;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::get_contiguous_piece(const Cont_Isect_Cursor& isc,
                                            double tol, Contour& newpiece)
{
  isc.Pred()->Pnt.Extract_Upto(isc->Pnt, newpiece);

  newpiece.Begin_Par(0.0);
   
  Elem_Cursor elc(newpiece.el_list); ++elc;
   
  bool contiguous = true;

  while (elc) {
    Elem& prvel = elc.Pred()->El();
    Elem& curel = elc->El();

    double dist = prvel.P2().distTo2(curel.P1());

    if (dist < 2.0 * Vec2::IdentDist) prvel.Join_To_XY(curel,false);
    else if (!repair_gap(elc,tol)) contiguous = false;
    
    ++elc;
  }
   
  newpiece.inval_rects();
  newpiece.calc_invar();

  return contiguous;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Cont1_Isect_List::Extract_Offset_Closed_Special(double tol,
                                               Cont_Clsd_D_List& cnt_list)
{
  cnt_list.Delete();

  // Hunt for intersected parts

  unmark();

  Cont_Clsd_Cursor cc(cnt_list);

  if (!ilist) {
    Cont_Clsd cnt;
    if (!compile_closed_cont(tol,cnt)) return false;

    // If area is very small, just delete the contour
    if (fabs(cnt.Area_XY()) < Vec2::IdentDist/100.0/Vec2::Pi) return true;

    Remove_Short_Elems(cnt.cont.el_list,100.0*Vec2::IdentDist,true);

    cnt.cont.inval_rects();
    cnt.calc_invar();
    cnt.Begin_Par(0.0);

    if (!cnt.Empty()) {
      cc.Insert(Cont_Clsd());

      cnt.Move_To(*cc);
    }

    return true;
  }

  while (ilist) {

    Cont_Isect_Cursor isc; isc = ilist.Begin();

    // Hunt for closed loop

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) isc = isc->Other;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    unmark();

    Contour newc;
    Elem_Cursor elc(newc.el_list);

    bool contiguous = true;

    do {
      isc->Mark = true; isc->Other->Mark = true;

      if (isc->To_Left == left) {
        if (contiguous) {

          Contour newpiece;

          if (!get_contiguous_piece(isc,tol,newpiece)) {
            contiguous = false;
            break;
          }

          elc.To_Last();

          if (!newpiece.Empty()) {
             newpiece.el_list.Append_To(newc.el_list);

             if (elc && elc.Succ()) elc->El().Join_To_XY(elc.Succ()->El());
          }
        }

        isc = isc->Other;
      }
      else contiguous = false;

      ++isc; if (!isc) isc.To_Begin();

    } while (!isc->Mark);

    if (contiguous) {
      elc.To_Last();
      if (elc) elc->El().Join_To_XY(elc.Succ()->El());

      if (!newc.Empty() && check_contiguous(newc.el_list,true)) {
        Remove_Short_Elems(newc.el_list, 5.0*Vec2::IdentDist,true);

        newc.inval_rects();
        newc.calc_invar();

        cc.To_End();
        cc.Insert(Cont_Clsd());

        newc.Move_To(cc->cont);

        cc->calc_invar();
        
        // If area is very small, just delete the contour
        if (fabs(cc->Area_XY()) < Vec2::IdentDist/100.0/Vec2::Pi)
                                                          cc.Delete(); 
      }
    }

    delete_marked();
  }
  
  return true;
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
