/* ---------------------------------------------------------------------- */
/* ------------------- Rectangle Manipulations -------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1996 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

#include "Rect.h"

#include <cstdlib>
#include <cmath>

#include <algorithm>

namespace Ino
{

using namespace std;

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

Rect_Ax::Rect_Ax()
: vll(), vur(), valid(false)
{
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

Rect_Ax::Rect_Ax(double lx, double ly, double lz,
                 double hx, double hy, double hz)
: vll(lx,ly,lz), vur(hx,hy,hz), valid(true)
{
  order();
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

void Rect_Ax::order()
{
  double hold;

  if (vll.x > vur.x) { hold = vll.x; vll.x = vur.x; vur.x = hold; }
  if (vll.y > vur.y) { hold = vll.y; vll.y = vur.y; vur.y = hold; }
  if (vll.z > vur.z) { hold = vll.z; vll.z = vur.z; vur.z = hold; }
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

Rect_Ax& Rect_Ax::operator=(const Rect_Ax& src)
{
  vll = src.vll;
  vur = src.vur;

  valid = true;

  return *this;
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

Rect_Ax& Rect_Ax::operator+= (const Rect_Ax& r)
{
  if (!valid) return operator=(r);

  if (r.vll.x < vll.x) vll.x = r.vll.x;
  if (r.vur.x > vur.x) vur.x = r.vur.x;

  if (r.vll.y < vll.y) vll.y = r.vll.y;
  if (r.vur.y > vur.y) vur.y = r.vur.y;

  if (r.vll.z < vll.z) vll.z = r.vll.z;
  if (r.vur.z > vur.z) vur.z = r.vur.z;

  return *this;
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

Rect_Ax& Rect_Ax::operator+= (const Vec3& p)
{
  if (!valid) {
    vll = vur = p;
    valid = true;
  }
  else {
    if (p.x < vll.x) vll.x = p.x;
    if (p.x > vur.x) vur.x = p.x;

    if (p.y < vll.y) vll.y = p.y;
    if (p.y > vur.y) vur.y = p.y;

    if (p.z < vll.z) vll.z = p.z;
    if (p.z > vur.z) vur.z = p.z;
  }

  return *this;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void Rect_Ax::clear()
{
  vur = vll = Vec3();

  valid = false;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

bool Rect_Ax::Intersects_XY(const Rect_Ax& r, double extra_edge) const
{
  extra_edge += extra_edge;

  if (vur.x < r.vll.x-extra_edge || vll.x > r.vur.x+extra_edge) return false;
  if (vur.y < r.vll.y-extra_edge || vll.y > r.vur.y+extra_edge) return false;

  return true;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

bool Rect_Ax::Intersects(const Rect_Ax& r, double extra_edge) const
{
  extra_edge += extra_edge;

  if (vur.x < r.vll.x-extra_edge || vll.x > r.vur.x+extra_edge) return false;
  if (vur.y < r.vll.y-extra_edge || vll.y > r.vur.y+extra_edge) return false;
  if (vur.z < r.vll.z-extra_edge || vll.z > r.vur.z+extra_edge) return false;

  return true;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

bool Rect_Ax::Point_Inside_XY(const Vec2& p, double extra_edge) const
{
  if (p.x < vll.x-extra_edge || p.x > vur.x+extra_edge) return false;
  if (p.y < vll.y-extra_edge || p.y > vur.y+extra_edge) return false;

  return true;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

bool Rect_Ax::Point_Inside(const Vec3& p, double extra_edge) const
{
  if (p.x < vll.x-extra_edge || p.x > vur.x+extra_edge) return false;
  if (p.y < vll.y-extra_edge || p.y > vur.y+extra_edge) return false;
  if (p.z < vll.z-extra_edge || p.z > vur.z+extra_edge) return false;

  return true;
}

/* ------------------------------------------------------------------------- */
/* ------- Modify the rect ------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void Rect_Ax::Rect_Update(const Rect_Ax& r)
{
  vll = r.vll; vur = r.vur;

  valid = r.valid;

  order();
}

/* ------------------------------------------------------------------------- */
/* ------- Modify the rect ------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void Rect_Ax::Rect_Update(const Vec3& p1, const Vec3& p2)
{
  vll = p1; vur = p2;

  valid = true;

  order();
}

/* ------------------------------------------------------------------------- */
/* --------- Calculate surrounding rect for an arc ------------------------- */
/* ------------------------------------------------------------------------- */

void Rect_Ax::Around_Arc(const Vec3& s, const Vec3& e, const Vec2& c, bool acw)
{
double lx,hx,ly,hy,r;
short int code;
Vec2 a,b;

  r = s.distTo2(c);

  lx = c.x - r; hx = c.x + r;
  ly = c.y - r; hy = c.y + r;

  if (acw) { a = s; b = e; }
  else     { a = e; b = s; }
  
  code = 0;
  if (a.x >= c.x) code += 8;
  if (a.y >= c.y) code += 4;
  if (b.x >= c.x) code += 2;
  if (b.y >= c.y) code += 1;

  switch (code) {
   case  0 : if (b.y < a.y || b.x > a.x) {
               lx = a.x; hx = b.x; ly = b.y; hy = a.y;
              }
   break;
   case  1 : lx = min(a.x,b.x);
   break;
   case  2 : lx = a.x; hx = b.x; hy = max(a.y,b.y);
   break;
   case  3 : lx = a.x; hy = b.y;
   break;
   case  4 : hy = a.y; ly = b.y; hx = max(a.x,b.x);
   break;
   case  5 : if (b.y < a.y || b.x < a.x) {
               lx = b.x; hx = a.x; ly = b.y; hy = a.y;
              }
   break;
   case  6 : hy = a.y; hx = b.x;
   break;
   case  7 : hy = max(a.y,b.y);
   break;
   case  8 : ly = min(a.y,b.y);
   break;
   case  9 : ly = a.y; lx = b.x;
   break;
   case 10 : if (b.y > a.y || b.x > a.x) {
               lx = a.x; hx = b.x; ly = a.y; hy = b.y;
              }
   break;
   case 11 : ly = a.y; hy = b.y; lx = min(a.x,b.x);
   break;
   case 12 : hx = a.x; ly = b.y;
   break;
   case 13 : hx = a.x; lx = b.x; ly = min(a.y,b.y);
   break;
   case 14 : hx = max(a.x,b.x);
   break;
   case 15 : if (b.y > a.y || b.x < a.x) {
               lx = b.x; hx = a.x; ly = a.y; hy = b.y;
              }
   break;
  } /* case */

  vll.x = lx; vll.y = ly;
  vur.x = hx; vur.y = hy;

  if (s.z < e.z) {
    vll.z = s.z; vur.z = e.z;
  }
  else {
    vll.z = e.z; vur.z = s.z;
  }

  valid = true;
}

/* ------------------------------------------------------------------------- */
/* ------- Calculate area of rectangle ------------------------------------- */
/* ------------------------------------------------------------------------- */

double Rect_Ax::Area_XY() const
{
  double dx = fabs(vur.x - vll.x);
  double dy = fabs(vur.y - vll.y);

  return dx * dy;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

Vec3 Rect_Ax::Ul() const
{
  Vec2 diag(vur); diag -= vll;

  double dia_len = diag.unitLen2();
  Vec2 hgt(0,vur.y-vll.y);
  double rel_hgt = (hgt * diag)/dia_len;

  double z = vll.z + rel_hgt * (vur.z - vll.z);

  return Vec3(vll.x,vur.y,z);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

Vec3 Rect_Ax::Lr() const
{
  Vec2 diag(vur); diag -= vll;

  double dia_len = diag.unitLen2();
  Vec2 hgt(0,vur.y-vll.y);
  double rel_hgt = (hgt * diag)/dia_len;

  double z = vur.z + rel_hgt * (vll.z - vur.z);

  return Vec3(vur.x,vll.y,z);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */


double Rect_Ax::Height() const
{
  return fabs(vur.y - vll.y);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

double Rect_Ax::Width() const
{
  return fabs(vur.x - vll.x);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

bool Rect_Ax::MidPoint(Vec3& midPt) const
{
  if (!valid) return false;

  midPt.x = (vll.x + vur.x)/2.0;
  midPt.y = (vll.y + vur.y)/2.0;
  midPt.z = (vll.z + vur.z)/2.0;

  return true;
}

/* ------------------------------------------------------------------------- */
/* ------- Distance to Rect, if p inside --> 0.0 is returned --------------- */
/* ------------------------------------------------------------------------- */

double Rect_Ax::Dist_To_XY(const Vec2& p) const
{
  Vec2 d(0,0);

  if      (p.x <= vll.x) d.x = vll.x - p.x;
  else if (p.x >= vur.x) d.x = p.x   - vur.x;

  if      (p.y <= vll.y) d.y = vll.y - p.y;
  else if (p.y >= vur.y) d.y = p.y   - vur.y;

  return d.len2();
}

} // namespace Ino

/* ------------------------------------------------------------------------- */
/* ------------------ End of module ---------------------------------------- */
/* ------------------------------------------------------------------------- */
