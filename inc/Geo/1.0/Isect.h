/* ---------------------------------------------------------------------- */
/* --------- Element Intersection Points -------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

// $Id: Isect.h,v 1.1 2008/10/16 09:10:58 clemens Exp $

#ifndef ISECT_INC
#define ISECT_INC

#include "Vec.h"
#include "it_dlist.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Single Intersection Point on Element ------------------------- */
/* ---------------------------------------------------------------------- */

struct Isect_Single
{
  Vec3   Tg;             // Tangent vector
  double Par;            // 3D Parametric coordinate (rel. to startpoint)
  double Curve;          // Gaussian (2D) curvature

  Isect_Single(const Vec3& tg, double par, double curve)
                       : Tg(tg), Par(par), Curve(curve) {}

  Isect_Single(const Isect_Single& cp)
           : Tg(cp.Tg), Par(cp.Par), Curve(cp.Curve) {}

  Isect_Single& operator=(const Isect_Single& src);
};

/* ---------------------------------------------------------------------- */
/* ------- Coinciding Pair of Intersection Points ----------------------- */
/* ------- One on either element ---------------------------------------- */
/* ---------------------------------------------------------------------- */

class Isect_Pair
{
  Vec3 isp;              // Intersection point
  Isect_Single fst,lst;  // First and second element resp.

 public:

  Isect_Pair(const Vec3& p,
             const Vec3& tg1, double par1, double curve1,
             const Vec3& tg2, double par2, double curve2);

  Isect_Pair(const Isect_Pair& cp);

  Isect_Pair& operator=(const Isect_Pair& src);

  void Swap();

  const Vec3& P;
  const Isect_Single& First;
  const Isect_Single& Last;
};

/* ---------------------------------------------------------------------- */
/* ------- Comparator for Isect_Pair ------------------------------------ */
/* ---------------------------------------------------------------------- */

class Isect_Cmp
{
 public:
  static int Compare(const Isect_Pair& p1, const Isect_Pair& p2);
};

/* ---------------------------------------------------------------------- */
/* ------- List of Intersection Pairs ----------------------------------- */
/* ---------------------------------------------------------------------- */

typedef IT_Chain_Alloc<IT_D_Item<Isect_Pair> > Isect_Alloc;
typedef IT_D_List<Isect_Pair, Isect_Alloc> Isect_Lst;
typedef Isect_Lst::Cursor                  Isect_Cursor;
typedef Isect_Lst::C_Cursor                Isect_C_Cursor;

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
