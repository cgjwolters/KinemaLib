/* ---------------------------------------------------------------------- */
/* ---------------- Cilinder Hoogte Programma --------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Alcoa Drunen NL, Oct 1994 ------------- */
/* ---------------------------- (Inofor Hoek Aut BV, C. Wolters) -------- */
/* ---------------------------------------------------------------------- */

// $Id: Geo.h,v 1.1 2008/10/16 09:10:58 clemens Exp $

#ifndef GEO_INC
#define GEO_INC

#include "Vec.h"

namespace Ino
{

/* --------------------------------------------------------------------- */
/* ------------ Correct centre of arc such that both endpoints lie------ */
/* ------------ exactly on the arc (radius is taken as dist(p1<->c)) --- */
/* --------------------------------------------------------------------- */

extern void Geo_Correct_Arc(const Vec2& p1, const Vec2&p2, Vec2& c);

/* --------------------------------------------------------------------- */
/* ------------ Normalize angle ---------------------------------------- */
/* --------------------------------------------------------------------- */

extern double Geo_Norm_Angle(bool acw, double a);

/* --------------------------------------------------------------------- */
/* ----------- Calculate arcspan of arc in radians (acw = positive) ---- */
/* --------------------------------------------------------------------- */

extern double Geo_Arc_Span(const Vec2& p1, const Vec2& p2,
                                           const Vec2& centre, bool acw);

/* --------------------------------------------------------------------- */
/* ----------- Calculate arclength of arc ------------------------------ */
/* --------------------------------------------------------------------- */

extern double Geo_Arc_Len(const Vec2& p1, const Vec2& p2,
                                          const Vec2& centre, bool acw);

/* --------------------------------------------------------------------- */
/* -------- Find point on arc halfway between start- and endpoint ------ */
/* --------------------------------------------------------------------- */

extern Vec2 Geo_Arc_Mid(const Vec2& p1, const Vec2& p2,
                                        const Vec2& centre, bool acw);

/* --------------------------------------------------------------------- */
/* ------------ Is c in between a and b in acw direction? -------------- */
/* --------------------------------------------------------------------- */

extern bool Geo_In_Between(const Vec2& a, const Vec2& b, const Vec2& c);

/* --------------------------------------------------------------------- */
/* ------------ Project Point on a Line -------------------------------- */
/* --------------------------------------------------------------------- */

extern bool Geo_Project_P_on_Line(const Vec2& p,
                                  const Vec2& s, const Vec2& e,
                                  bool  strict, double tol,
                                  Vec2& pp,
                                  double& pr, double& dist);

/* --------------------------------------------------------------------- */
/* ------------ Project Point on an Arc -------------------------------- */
/* --------------------------------------------------------------------- */

extern bool Geo_Project_P_on_Arc(const Vec2& p,
                                 const Vec2& s, const Vec2& e,
                                 const Vec2& c, bool acw,
                                 bool  strict, double tol,
                                 Vec2& pp, double& pr, double& dist);

/* --------------------------------------------------------------------- */
/* ------------ Intersect two lines ------------------------------------ */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Lines(const Vec2& s1, const Vec2& e1,
                           const Vec2& s2, const Vec2& e2,
                           bool strict, double tol,
                           Vec2& ip, double& pr1, double& pr2);

/* --------------------------------------------------------------------- */
/* ------------ Intersect a line with an arc --------------------------- */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Line_Arc(const Vec2& s1, const Vec2& e1,
                              const Vec2& s2, const Vec2& e2,
                              const Vec2& c2, bool acw,
                              bool strict, bool tangok, double tol,
                              Vec2& ipa, Vec2& ipb,
                              double& pr1a, double& pr1b,
                              double& pr2a, double& pr2b);

/* --------------------------------------------------------------------- */
/* ---------- Intersect an arc with an arc ----------------------------- */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Arc_Arc(const Vec2& s1, const Vec2& e1,
                             const Vec2& c1, bool acw1,
                             const Vec2& s2, const Vec2& e2,
                             const Vec2& c2, bool acw2,
                             bool strict, bool tangok, double tol,
                             Vec2& ipa, Vec2& ipb,
                             double& pr1a, double& pr1b,
                             double& pr2a, double& pr2b);

/* --------------------------------------------------------------------- */
/* ------------ Intersect a line with a circle ------------------------- */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Line_Circle(const Vec2& s1, const Vec2& e1,
                                const Vec2& s2, const Vec2& c2, bool acw2,
                                bool strict, bool tangok, double tol,
                                Vec2& ipa, Vec2& ipb,
                                double& pr1a, double& pr1b,
                                double& pr2a, double& pr2b);

/* --------------------------------------------------------------------- */
/* ---------- Intersect an arc with a circle --------------------------- */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Arc_Circle(const Vec2& s1, const Vec2& e1,
                               const Vec2& c1, bool acw1,
                               const Vec2& s2, const Vec2& c2, bool acw2,
                               bool strict, bool tangok, double tol,
                               Vec2& ipa, Vec2& ipb,
                               double& pr1a, double& pr1b,
                               double& pr2a, double& pr2b);

/* --------------------------------------------------------------------- */
/* ---------- Intersect a circle with a circle ------------------------- */
/* --------------------------------------------------------------------- */

extern int Geo_Isct_Circle_Circle(const Vec2& s1, const Vec2& c1, bool acw1,
                                  const Vec2& s2, const Vec2& c2, bool acw2,
                                  bool tangok, double tol,
                                  Vec2& ipa, Vec2& ipb,
                                  double& pr1a, double& pr1b,
                                  double& pr2a, double& pr2b);

} // namespace Ino

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
#endif
