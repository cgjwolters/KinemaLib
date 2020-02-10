/* ---------------------------------------------------------------------- */
/* --------- Template Implementations ----------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, May 1995 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include <StdAfx.h>

#define TEMPLATE

#include "it_dlist.hi"
#include "it_gen.hi"

#include "isect.h"
#include "contisct.hi"

#include "elem.h"
#include "sub_rect.hi"
#include "contour.h"
#include "contisct.hi"

namespace AWOld
{

using namespace AW;

/* ---------------------------------------------------------------------- */
/* ------- Intersection Pairs ------------------------------------------- */
/* ---------------------------------------------------------------------- */

Isect_Alloc *Isect_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Isect_Pair> >;
template class IT_D_Item<Isect_Pair>;
template class IT_D_List<Isect_Pair, Isect_Alloc>;
template class IT_D_C_Cursor<Isect_Pair, Isect_Alloc>;
template class IT_D_Cursor<Isect_Pair, Isect_Alloc>;

template class Q_Comp<Isect_Cursor,Isect_Cmp>;
template class IT_Container<Isect_Cursor>;
template class IT_Sort<Isect_Cursor,Isect_Cmp>;
template class IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>;

/* ---------------------------------------------------------------------- */
/* ------- Element Lists ------------------------------------------------ */
/* ---------------------------------------------------------------------- */

Elem_Alloc *Elem_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Elem_Ref> >;
template class IT_D_Item<Elem_Ref>;
template class IT_D_List<Elem_Ref,Elem_Alloc>;
template class IT_D_C_Cursor<Elem_Ref,Elem_Alloc>;
template class IT_D_Cursor<Elem_Ref,Elem_Alloc>;

/* ---------------------------------------------------------------------- */
/* ------- Element Sort Optimizer --------------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Elem_Cursor> > Elem_Cursor_Alloc;

Elem_Cursor_Alloc* Elem_Cursor_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Elem_Cursor> >;
template class IT_D_Item<Elem_Cursor>;
template class IT_D_List<Elem_Cursor, Elem_Cursor_Alloc>;
template class IT_D_C_Cursor<Elem_Cursor, Elem_Cursor_Alloc>;
template class IT_D_Cursor<Elem_Cursor, Elem_Cursor_Alloc>;

/* ---------------------------------------------------------------------- */
/* ------- Element SubRect Lists ---------------------------------------- */
/* ---------------------------------------------------------------------- */

Sub_Rect_Alloc *Sub_Rect_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Sub_Rect> >;
template class IT_D_Item<Sub_Rect>;
template class IT_D_List<Sub_Rect,Sub_Rect_Alloc>;
template class IT_D_C_Cursor<Sub_Rect,Sub_Rect_Alloc>;
template class IT_D_Cursor<Sub_Rect,Sub_Rect_Alloc>;

/* ---------------------------------------------------------------------- */
/* ------- Contour Point Pair Lists ------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_PPair_Alloc *Cont_PPair_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_PPair> >;
template class IT_D_Item<Cont_PPair>;
template class IT_D_List<Cont_PPair,Cont_PPair_Alloc>;
template class IT_D_C_Cursor<Cont_PPair, Cont_PPair_Alloc>;
template class IT_D_Cursor<Cont_PPair, Cont_PPair_Alloc>;

/* ---------------------------------------------------------------------- */
/* ------- Contour Intersection Point Lists ----------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Ref_Alloc *Cont_Ref_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Ref> >;
template class IT_D_Item<Cont_Ref>;
template class IT_D_List<Cont_Ref,Cont_Ref_Alloc>;
template class IT_D_C_Cursor<Cont_Ref, Cont_Ref_Alloc>;
template class IT_D_Cursor<Cont_Ref, Cont_Ref_Alloc>;

Cont_Isect_Alloc *Cont_Isect_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Isect> >;
template class IT_D_Item<Cont_Isect>;
template class IT_D_List<Cont_Isect,Cont_Isect_Alloc>;
template class IT_D_C_Cursor<Cont_Isect, Cont_Isect_Alloc>;
template class IT_D_Cursor<Cont_Isect, Cont_Isect_Alloc>;

IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >
               *IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Isect_Cursor> >;

template class Q_Comp<Cont_Isect_D_Cursor,Cont_Isect_Cmp>;
template class IT_Container<Cont_Isect_D_Cursor>;
template class IT_Sort<Cont_Isect_D_Cursor,Cont_Isect_Cmp>;

/* ---------------------------------------------------------------------- */
/* ------- Contour Lists ------------------------------------------------ */
/* ---------------------------------------------------------------------- */

Cont_Alloc *Cont_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Contour> >;
template class IT_D_Item<Contour>;
template class IT_D_List<Contour, Cont_Alloc>;
template class IT_D_C_Cursor<Contour, Cont_Alloc>;
template class IT_D_Cursor<Contour, Cont_Alloc>;

/* ---------------------------------------------------------------------- */
/* -------- Closed Contour Lists ---------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Clsd_Alloc *Cont_Clsd_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Clsd> >;
template class IT_D_Item<Cont_Clsd>;
template class IT_D_List<Cont_Clsd, Cont_Clsd_Alloc>;
template class IT_D_C_Cursor<Cont_Clsd, Cont_Clsd_Alloc>;
template class IT_D_Cursor<Cont_Clsd, Cont_Clsd_Alloc>;

template class Q_Comp<Cont_Clsd_Cursor,Cont_Area_Cmp>;
template class IT_Container<Cont_Clsd_Cursor>;
template class IT_Sort<Cont_Clsd_Cursor,Cont_Area_Cmp>;

/* ---------------------------------------------------------------------- */
/* -------- Nested Contour Lists ---------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Nest_Alloc *Cont_Nest_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Nest> >;
template class IT_D_Item<Cont_Nest>;
template class IT_D_List<Cont_Nest, Cont_Nest_Alloc>;
template class IT_D_C_Cursor<Cont_Nest, Cont_Nest_Alloc>;
template class IT_D_Cursor<Cont_Nest, Cont_Nest_Alloc>;

/* ---------------------------------------------------------------------- */
/* -------- Area Lists -------------------------------------------------- */
/* ---------------------------------------------------------------------- */

Cont_Area_Alloc *Cont_Area_Alloc::root = NULL;

template class IT_Chain_Alloc<IT_D_Item<Cont_Area> >;
template class IT_D_Item<Cont_Area>;
template class IT_D_List<Cont_Area, Cont_Area_Alloc>;
template class IT_D_C_Cursor<Cont_Area, Cont_Area_Alloc>;
template class IT_D_Cursor<Cont_Area, Cont_Area_Alloc>;

} // namespace AWOld

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
