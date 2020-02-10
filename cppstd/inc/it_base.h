/* ---------------------------------------------------------------------- */
/* ------- Basic Template Definitions ----------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ------------- Copyright Inofor Hoek Aut BV 1996, C. Wolters ---------- */
/* ---------------------------------------------------------------------- */

// $Id: it_base.h,v 1.2 2009/03/06 10:40:30 clemens Exp $

#ifndef IT_BASE_INC
#define IT_BASE_INC

#include <stddef.h>

#include "Basics.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Cursor Categories -------------------------------------------- */
/* ---------------------------------------------------------------------- */

struct IT_Forward
{
  const IT_Forward *Cursor_Type() const { return this; }
        IT_Forward *Cursor_Type()       { return this; }
};

struct IT_Bidirectional
{
  const IT_Bidirectional *Cursor_Type() const { return this; }
        IT_Bidirectional *Cursor_Type()       { return this; }
};

struct IT_Random
{
  const IT_Random *Cursor_Type() const { return this; }
        IT_Random *Cursor_Type()       { return this; }
};

/* ---------------------------------------------------------------------- */
/* ------- Storage Categories ------------------------------------------- */
/* ---------------------------------------------------------------------- */

struct IT_Value_Bound
{
  const IT_Value_Bound *Cursor_Binding() const { return this; }
        IT_Value_Bound *Cursor_Binding()       { return this; }
};

struct IT_Location_Bound
{
  const IT_Location_Bound *Cursor_Binding() const { return this; }
        IT_Location_Bound *Cursor_Binding()       { return this; }
};

/* ---------------------------------------------------------------------- */
/* ------- New operator for standard types ------------------------------ */
/* ---------------------------------------------------------------------- */

#ifndef __GNUC__
template <class T>
void *operator new(size_t sz, T* tp) { return tp; }
#endif

/* ---------------------------------------------------------------------- */
/* ------ Standard Allocator for Inofor Templates ----------------------- */
/* ---------------------------------------------------------------------- */

template <class T>
struct IT_Std_Alloc
{
  static T* New() { return new char[sizeof(T)]; }
  static void Free(T *v) { if (v) delete[] v; }
};

/* ---------------------------------------------------------------------- */
/* ------ Chain Allocator for Inofor Templates -------------------------- */
/* ---------------------------------------------------------------------- */

template <class T>
class IT_Chain_Alloc
{
    IT_Chain_Alloc *next;
    static IT_Chain_Alloc *root;

  public:
    static T* New();
    static void Free(T *v);

    static void Cleanup();
};

#include "it_base_imp.h"

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
