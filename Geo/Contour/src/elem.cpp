/* ---------------------------------------------------------------------- */
/* ---------------- Basic 3D Element ------------------------------------ */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "Elem.h"

#include "El_Info.h"

#include <math.h>

namespace Ino
{

long Elem::defaultColor = 0;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Info *Clone_Elem_Info (const Elem_Info * /*from*/)
{
  return NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Delete_Elem_Info(Elem_Info * /*info*/)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem_Calc_Par_Len(const Elem_List& ellst)
{
  double curlen = 0.0;
  
  Elem_C_Cursor elc(ellst);
  
  for (;elc;++elc) curlen += elc->El().Par_Len();
  
  return curlen;
}

/* ---------------------------------------------------------------------- */
/* ------- Cloning ------------------------------------------------------ */
/* ---------------------------------------------------------------------- */

void Elem::clone_from(const Elem& from, bool keep_info)
{
  Rect_Ax::operator=(from);

  plen     = from.plen;
  len      = from.len;
  len_xy   = from.len_xy;
  bpar     = from.bpar;
  el_id    = from.el_id;
  cnt_id   = from.cnt_id;
  p_cnt_id = from.p_cnt_id;
  cam_inf  = from.cam_inf;
  insArc   = from.insArc;
  color    = from.color;

  if (inf) Delete_Elem_Info(inf);
  if (keep_info && from.inf) inf = Clone_Elem_Info(from.inf);
  else inf = NULL;
}

/* ---------------------------------------------------------------------- */
/* ----------- Destructor ----------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem::~Elem()
{
  if (inf) Delete_Elem_Info(inf);
}

/* ---------------------------------------------------------------------- */
/* ------- Add Info ----------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Add_Info(const Elem_Info& info)
{
  if (inf) Delete_Elem_Info(inf);
  inf = Clone_Elem_Info(&info);
}

/* ---------------------------------------------------------------------- */
/* ------- Delete Info -------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Del_Info()
{
  if (inf) Delete_Elem_Info(inf);
  inf = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

long Elem::getDefaultColor()
{
  return defaultColor;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::setDefaultColor(long newColor)
{
  defaultColor = newColor;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem::setColor(long newColor)
{
  if (color == newColor) return false;

  color = newColor;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem::Par_Valid(double par) const
{
  par -= bpar;

  if (par < -Vec2::IdentDist ||
      par > plen+Vec2::IdentDist) return false;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Mid_Par_XY(Vec2& p) const
{
  double midpar;
  
  Par(bpar + len_xy/2.0,midpar);
  
  Vec3 mp;
  At_Par(midpar,mp);

  p = mp;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Mid_Par(Vec3& p) const
{
  At_Par(bpar + len/2.0,p);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem::Mid_Par_XY() const
{
  double midpar;
  
  Par(bpar + len_xy/2.0,midpar);
  
  return midpar;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

double Elem::Mid_Par() const
{
  return bpar + len/2.0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem::Par_XY(double par, double& par_xy) const
{
  if (len <= NumAccuracy) return false;

  par_xy = (par-bpar)*len_xy/len + bpar;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem::Par(double par_xy, double& par) const
{
  if (len_xy < NumAccuracy*len) return false;

  par = (par_xy-bpar)*len/len_xy + bpar;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Join_To_XY(Elem& el)
{
  double lenprv = len_xy;
  double lensum = lenprv + el.len_xy;

  if (lensum > 0.0) lenprv /= lensum;
  else              lenprv = 0.5;

  Vec3 newp = el.P1(); newp -= P2();

  newp *= lenprv;
  newp += P2();

  Stretch_End_XY(newp);
  el.Stretch_Begin_XY(newp);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem::Join_To_XY(Elem& el, bool check)
{
  double lenprv = len_xy;
  double lensum = lenprv + el.len_xy;

  if (lensum > 0.0) lenprv /= lensum;
  else              lenprv = 0.5;

  Vec3 newp = el.P1(); newp -= P2();

  newp *= lenprv;
  newp += P2();

  Stretch_End_XY(newp,check);
  el.Stretch_Begin_XY(newp,check);
}

//---------------------------------------------------------------------------

void Elem::Sort(Elem_List& elLst)
{
  const double sqIdDist = sqr(Vec2::IdentDist);

  Elem_List newLst;
  Elem_Cursor bElc(newLst), eElc(newLst);

  Elem_List cirLst;
  Elem_Cursor cirElc(cirLst);

  Elem_Cursor elc(elLst);
  if (!elc) return;

  bElc.Re_Insert(elc);

  bElc.To_Begin();
  Elem_Cursor fstElc = bElc;

  while (elLst.Length() > 0) {
    eElc.To_Last();

    const Elem& bEl = bElc->El();
    const Elem& eEl = eElc->El();

    eElc.To_End();

    elc.To_Begin();
    bool reInserted = false;

    for (;elc;++elc) {
      const Elem& el = elc->El();

      if (el.isCircle()) { // Separate the circles
        cirElc.To_End();
        cirElc.Re_Insert(elc);
        reInserted = true;
        break;
      }

      if (el.P1().sqDistTo2(eEl.P2()) <= sqIdDist){
        eElc.Re_Insert(elc);
        reInserted = true;
        break;
      }
      else if (el.P2().sqDistTo2(eEl.P2()) <= sqIdDist){
        elc->El().Reverse();
        eElc.Re_Insert(elc);
        reInserted = true;
        break;
      }

      if (el.P2().sqDistTo2(bEl.P1()) <= sqIdDist) {
        bElc.Re_Insert(elc);
        reInserted = true;
        break;
      }
      else if (el.P1().sqDistTo2(bEl.P1()) <= sqIdDist) {
        elc->El().Reverse();
        bElc.Re_Insert(elc);
        reInserted = true;
        break;
      }
    }

    if (!reInserted) {
      elc.To_Begin();
      eElc.Re_Insert(elc);
      bElc = eElc;
    }
  }

  // Append all circles
  cirElc.To_Begin();
  
  while (cirElc) {
    eElc.To_End();
    eElc.Re_Insert(cirElc);
  }

  if (IsContiguous(newLst,true)) fstElc.Become_First();

  newLst.Move_To(elLst);
}

//---------------------------------------------------------------------------

bool Elem::IsContiguous(const Elem_List& elLst, bool mustBeClosed, double tol)
{
  double sqTol = sqr(tol);

  Elem_C_Cursor elc(elLst);
  if (!elc) return false;

  if (elLst.Length() == 1 && elc->El().isCircle()) return true;

  Elem_C_Cursor prvElc(elc); ++elc;

  for (;elc;++elc) {
    if (prvElc->El().P2().sqDistTo2(elc->El().P1()) > sqTol) return false;
    prvElc = elc;
  }

  if (mustBeClosed) {
    elc.To_Begin();
    if (prvElc->El().P2().sqDistTo2(elc->El().P1()) > sqTol) return false;
  }

  return true;
}

//---------------------------------------------------------------------------
// Persistence section

static const char fldBPar[]  = "bPar";
static const char fldColor[] = "Color";
static const char fldId[]    = "Id";

void Elem::definePersistentFields(PersistentWriter& po) const
{
  po.addField(fldBPar,typeid(double));
  po.addField(fldColor,typeid(long));
  po.addField(fldId,typeid(long));
}

//---------------------------------------------------------------------------

Elem::Elem(PersistentReader& pi)
: Rect_Ax(),inf(NULL),plen(0.0),len(0.0),len_xy(0.0),
  bpar(pi.readDouble(fldBPar,0.0)),
  el_id(pi.readInt(fldId,0)), cnt_id(0), p_cnt_id(0),
  cam_inf(0), insArc(false),
  color((long)pi.readInt(fldColor,defaultColor))
{
}

//---------------------------------------------------------------------------

void Elem::writePersistentObject(PersistentWriter& po) const
{
  po.writeDouble(fldBPar,bpar);
  po.writeInt(fldColor,color);
  po.writeInt(fldId,el_id);
}

/* ---------------------------------------------------------------------- */
/* ------- End temp solution -------------------------------------------- */
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Ref::Elem_Ref(const Elem& el)
: elp(el.Clone())
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Ref::Elem_Ref(const Elem_Ref& cp)
: elp(cp.elp ? cp.elp->Clone() : NULL)
{
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Ref::~Elem_Ref()
{
  if (elp) delete elp;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Ref& Elem_Ref::operator=(const Elem_Ref& src)
{
  if (elp) delete elp;
  elp = NULL;
  if (src) elp = src.elp->Clone();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Ref& Elem_Ref::operator=(const Elem& src)
{
  if (elp) delete elp;
  elp = src.Clone();

  return *this;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Elem_Ref::setElem(Elem *el)
{
  if (elp) delete elp;

  elp = el;
}

/* ---------------------------------------------------------------------- */
/* ------- Element Sort Optimizer --------------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Elem_Cursor> > El_Cursor_Alloc;
typedef IT_D_List<Elem_Cursor, El_Cursor_Alloc> Elem_Ptr_List;
typedef Elem_Ptr_List::Cursor Ptr_Cursor;

typedef Elem_Ptr_List Elem_Ptr_Array[Elem_Search_Size];

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static int find_index(double val, int upb_idx, double base, double delta)
{
  int idx, lwb = 0, upb = upb_idx-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    double cmpval = base + idx*delta;
    if (cmpval > val) upb = idx-1;
    else              lwb = idx+1;
  }

  if (upb < 0) upb = 0;

  return upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void find_elem_one(const Vec2& srch_pt,
                          Elem_Ptr_Array el_arr[], int i, int j,
                          Ptr_Cursor& ptrc, double &mindist)
{
  Ptr_Cursor sc(el_arr[i][j]);
  ptrc.To_End();

  for (;sc;++sc) {
    Elem_Cursor& elc = *sc;

    double dist1 = srch_pt.distTo2(elc->El().P1());
    double dist2 = srch_pt.distTo2(elc->El().P2());

    if (dist2 < dist1) dist1 = dist2;

    if (!ptrc || dist1 < mindist) {
      mindist = dist1;
      ptrc = sc;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Determine which neighbours to search ------------------------- */
/* ---------------------------------------------------------------------- */

static void find_range(const Vec2& srch_pt, double tol,
                       int upb_x, int upb_y,
                       const Vec2& base, const Vec2& delta,
                       int &lwb_i, int& upb_i, int &lwb_j, int& upb_j)
{
  lwb_i = upb_i = find_index(srch_pt.x,upb_x,base.x,delta.x);
  lwb_j = upb_j = find_index(srch_pt.y,upb_y,base.y,delta.y);

  // Perhaps neighbours must be checked as well

  double dist_x = srch_pt.x - base.x + lwb_i*delta.x ;
  double dist_y = srch_pt.y - base.y + lwb_j*delta.y ;

  if (lwb_i > 0 && dist_x <= tol) lwb_i--;
  else if (upb_i < upb_x-1 &&
               delta.x - dist_x <= tol) upb_i++;

  if (lwb_j > 0 && dist_y <= tol) lwb_j--;
  else if (upb_j < upb_y-1 &&
               delta.y - dist_y <= tol) upb_j++;
}
            

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void find_elem(const Vec2& srch_pt, double tol,
                      const Vec2& base, const Vec2& delta,
                      Elem_Ptr_Array el_arr[], int upb_x, int upb_y,
                      Ptr_Cursor& ptrc, double &mindist)
{
  int lwb_i, upb_i, lwb_j, upb_j;

  find_range(srch_pt,tol,upb_x,upb_y,base,delta,lwb_i,upb_i,lwb_j,upb_j);

  ptrc.To_End();

  Ptr_Cursor sc;

  for (int i=lwb_i; i<=upb_i; i++) {
    for (int j=lwb_j; j<=upb_j; j++) {
      double dist = 0.0;
      find_elem_one(srch_pt,el_arr,i,j,sc,dist);

      if (sc && (!ptrc || dist < mindist)) {
        mindist = dist;
        ptrc = sc;
      }
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void delete_cursor(const Vec2& srch_pt, double tol,
                          const Vec2& base, const Vec2& delta,
                          Elem_Ptr_Array el_arr[], int upb_x, int upb_y,
                          const Elem_Cursor& elc)
{
  int i,j,lwb_i, upb_i, lwb_j, upb_j;

  find_range(srch_pt,tol,upb_x,upb_y,base,delta,lwb_i,upb_i,lwb_j,upb_j);

  for (i=lwb_i; i<=upb_i; i++) {
    for (j=lwb_j; j<=upb_j; j++) {
      Ptr_Cursor sc(el_arr[i][j]);

      for (;sc;++sc) {
        if (*sc == elc) {
          sc.Delete();
          return;
        }
      }
    }
  }

  // Not found, search entire array as a last resort

  for (i=0; i<upb_x; i++) {
    for (j=0; j<upb_y; j++) {
      Ptr_Cursor sc(el_arr[i][j]);

      for (;sc;++sc) {
        if (*sc == elc) {
          sc.Delete();
          return;
        }
      }
    }
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Move list and also move forward closed elements -------------- */
/* ------- Also discard element shorter than Vec2::Ident_Dist ----------- */
/* ---------------------------------------------------------------------- */

static void bubble_up_closed(double tol, Elem_List& src, Elem_List& dst)
{
  dst.Delete();

  Elem_Cursor sc(src);
  Elem_Cursor dc(dst);

  // Move closed elements first
  while (sc) {
    if (sc->El().P1().distTo2(sc->El().P2()) <= tol) {
      double ellen = sc->El().Len_XY();
      if (ellen > tol) {
        dc.Re_Insert(sc); dc.To_End();
      }
      else if (ellen < Vec2::IdentDist) sc.Delete();
      else ++sc;
    }
    else ++sc;
  }

  // Now append the rest

  sc.To_Begin();
  Elem_Cursor ec(src); ec.To_End();

  dc.Re_Insert(sc,ec);
}

/* ---------------------------------------------------------------------- */
/* ------- Close small gaps --------------------------------------------- */
/* ---------------------------------------------------------------------- */

void close_gaps(Elem_List& list, bool closed)
{
  if (list.Length() < 2) return;

  Elem_Cursor elc(list);

  elc.To_Last();

  if (!closed) --elc;

  for (;elc;--elc) {
    Elem& nxtel = elc.Succ()->El();

    elc->El().Join_To_XY(nxtel);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Sort::Elem_Sort(Elem_List& el_lst, double dist_tol)
 : arr(NULL), upb_x(Elem_Search_Size), upb_y(Elem_Search_Size),
   list(), base(), delta(), tol(dist_tol)
{
  bubble_up_closed(tol,el_lst,list);

  Rect_Ax el_rect;

  Elem_Cursor elc(list);
  if (!elc) return;

  el_rect = elc->El().Rect();
  ++elc;

  for (;elc;++elc) el_rect += elc->El().Rect();

  base  = el_rect.Ll();
  delta = el_rect.Ur(); delta -= base; delta /= Elem_Search_Size;

  while (upb_x > 1 && delta.x < tol) {
    upb_x /= 2; delta.x *= 2.0;
  }

  while (upb_y > 1 && delta.y < tol) {
    upb_y /= 2; delta.y *= 2.0;
  }

  Elem_Ptr_Array *el_arr = new Elem_Ptr_Array[Elem_Search_Size];
  arr = el_arr;
  if (!arr) return;

  elc.To_Begin();

  for (;elc;++elc) {
    const Elem& el = elc->El();

    int i = find_index(el.P1().x,upb_x,base.x,delta.x);
    int j = find_index(el.P1().y,upb_y,base.y,delta.y);

    el_arr[i][j].Push_Back(elc);

    i = find_index(el.P2().x,upb_x,base.x,delta.x);
    j = find_index(el.P2().y,upb_y,base.y,delta.y);

    el_arr[i][j].Push_Back(elc);
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Elem_Sort::~Elem_Sort()
{
  Elem_Ptr_Array *el_arr = (Elem_Ptr_Array *)arr;
  if (el_arr) delete[] el_arr;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Elem_Sort::Chain(Elem_List& new_cont)
{
  new_cont.Delete();

  if (!arr) return false;

  Elem_Cursor newc(new_cont);

  Elem_Cursor elc(list);
  if (!elc) return false;

  Vec2 start_pt = elc->El().P1();
  Vec2 srch_pt  = elc->El().P2();

  Elem_Ptr_Array *el_arr = (Elem_Ptr_Array *)arr;

  delete_cursor(start_pt, tol, base, delta, el_arr, upb_x, upb_y, elc);
  delete_cursor(srch_pt,  tol, base, delta, el_arr, upb_x, upb_y, elc);

  newc.Re_Insert(elc); newc.To_End();

  Ptr_Cursor ptrc;

  bool closed = false;

  while (list) {
    double start_end_dist = srch_pt.distTo2(start_pt);
    if (start_end_dist <= tol) {
      closed = true;
      break;
    }

    double dist = 0.0;
    find_elem(srch_pt,tol,base,delta,el_arr,upb_x,upb_y,ptrc,dist);

    if (!ptrc || dist > tol) break;

    elc = *ptrc;
    if (srch_pt.distTo2(elc->El().P1()) >
                   srch_pt.distTo2(elc->El().P2())) elc->El().Reverse();

    if (start_end_dist < dist) {
      closed = true;
      break;
    }

    srch_pt = elc->El().P2();

    ptrc.Delete();  // Delete this end and other end in array
    delete_cursor(srch_pt, tol, base, delta, el_arr, upb_x, upb_y, elc);
    
    newc.Re_Insert(elc); newc.To_End();
  }

// Tijdelijk CW ivm ProlinerCalc
//  close_gaps(new_cont,closed);

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool Remove_Short_Elems(Elem_Cursor& elc, double maxlen, double maxdist)
{
  elc.To_Begin();
  if (!elc) return false;
  
  bool any_modified = false, modified = false;

  // Test any element shorter than maxlen


  do {
    modified = false;

    // First each element with its predecessor

    Elem_Cursor predc(elc); predc.To_Last();
    if (predc == elc) return any_modified;

    while (elc) {
      Elem& curel = elc->El();
      Elem& prvel = predc->El();

      double curlen = curel.Len_XY();
      double prvlen = prvel.Len_XY();

      if (prvlen < curlen && prvlen < maxlen) {
        Vec3 pnt;
        double parm,dist;

        Vec3 midp;
        prvel.Mid_Par(midp);

        if (curel.Project_Pnt_XY(prvel.P1(),Vec2::IdentDist,false,
                                  pnt,parm,dist) && fabs(dist) < maxdist &&
            curel.Project_Pnt_XY(midp,Vec2::IdentDist,false,
                                  pnt,parm,dist) && fabs(dist) < maxdist) {

          // We take midp as the join point

          Elem_Cursor predc2(predc); --predc2; if (!predc2) predc2.To_Last();

          if (predc2 == elc) {
            // This to be elaborated, but for now:
            break;
          }

          predc.Delete();

          predc2->El().Stretch_End_XY(midp,false);
          curel.Stretch_Begin_XY(midp,false);

          modified = true;
        }
      }

      predc = elc;
      ++elc;
    }


    // Then each element with its successor, run the loop in reverse

    elc.To_Last();

    predc.To_Begin();
    if (predc == elc) return any_modified;

    while (elc) {
      Elem& curel = elc->El();
      Elem& prvel = predc->El(); // Actually the successor!!

      double curlen = curel.Len_XY();
      double prvlen = prvel.Len_XY();

      if (prvlen < curlen && prvlen < maxlen) {
        Vec3 pnt;
        double parm,dist;

        Vec3 midp;
        prvel.Mid_Par(midp);

        if (curel.Project_Pnt_XY(prvel.P1(),Vec2::IdentDist,false,
                                  pnt,parm,dist) && fabs(dist) < maxdist &&
            curel.Project_Pnt_XY(midp,Vec2::IdentDist,false,
                                  pnt,parm,dist) && fabs(dist) < maxdist) {

          // We take midp as the join point

          Elem_Cursor predc2(predc); ++predc2; if (!predc2) predc2.To_Begin();

          if (predc2 == elc) {
            // This to be elaborated, but for now:
            break;
          }

          predc.Delete();

          predc2->El().Stretch_Begin_XY(midp,false);
          curel.Stretch_End_XY(midp,false);

          modified = true;
        }
      }

      predc = elc;
      --elc;
    }


    if (modified) any_modified = true;

  } while (modified);
  
  return any_modified;
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
