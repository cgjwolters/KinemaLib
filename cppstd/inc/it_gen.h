/* ---------------------------------------------------------------------- */
/* ------- General Template Definitions --------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ------------- Copyright Inofor Hoek Aut BV 1996, C. Wolters ---------- */
/* ---------------------------------------------------------------------- */

// $Id: it_gen.h,v 1.1 2008/10/16 09:10:57 clemens Exp $

#ifndef IT_GEN_INC
#define IT_GEN_INC

#include <stddef.h>

#include "Basics.h"
#include "it_base.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- General Sort Template ---------------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T,class Comparator>
class IT_Sort
{
public:
  static void Sort(T& begin, T&end, const Comparator& cmp);
};

#include "it_gen_imp.h"

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
