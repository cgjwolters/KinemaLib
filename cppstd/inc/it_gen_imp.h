/* ---------------------------------------------------------------------- */
/* ------- General Template Implementions ------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ------------- Copyright Inofor Hoek Aut BV 1996, C. Wolters ---------- */
/* ---------------------------------------------------------------------- */

#ifndef IT_GEN_INC
#error Include it_gen.h instead
#endif

#include <stdlib.h>
#include <search.h>

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

// To avoid inclusion of all of <stdlib.h> :
//extern "C" {
//  extern void qsort(void *, size_t, size_t, int(*)(const void *,const void *));
//}

/* ---------------------------------------------------------------------- */
/* ------- Comparator Adapter ------------------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T, class Comparator>
class Q_Comp
{
public:
  static int Compare(const void *i1, const void *i2)
     { return Comparator::Compare(*(*(const T *)i1), *(*(const T *)i2)); }
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

template <class T>
class IT_Container
{
public:
  static size_t IT_Items(const T& begin, const T&end, IT_Random *ct)
                                                { return end - begin; }
  static size_t IT_Items(const T& begin, const T& end, IT_Forward *ct);

  static size_t IT_Items(const T& begin, const T&end,
                                               IT_Bidirectional * /*ct*/)
                  { return IT_Items(begin,end,(IT_Forward *)NULL); }

  static void IT_ReOrder(T& begin, T& end, T *order, size_t items,
                                                       IT_Value_Bound *vt);
};

/* ---------------------------------------------------------------------- */
/* ------- Forward Item Counter ----------------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T>
size_t IT_Container<T>::IT_Items(const T& begin, const T& end,
                                                      IT_Forward * /*ct*/)
{
  T b(begin); b.To_Begin();
  T e(end);   e.To_End();

  if (b == begin && e == end && begin.Container())
                            return begin.Container()->Length();

  size_t items=0;
  b = begin;

  while (b != end) {  // Just count them
    if (!b) b.To_Begin();
    items++;
    ++b;
  }

  return items;
}

/* ---------------------------------------------------------------------- */
/* ------- Reorder Container -------------------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T>
void IT_Container<T>::IT_ReOrder(T& begin, T& end, T *order,
                                     size_t items, IT_Value_Bound * /*vt*/)
{
  if (items < 1) return;

  T c(begin);

  for (size_t i=0; i<items; i++) {
    if (c != order[i]) c.Re_Insert(order[i]);
    if (i==0) begin = c;
    ++c;
  }

  end = c;
}

#ifdef WIN32
#pragma warning( pop )
#endif

/* ---------------------------------------------------------------------- */
/* ------- General Sort Template ---------------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T, class Comparator>
void IT_Sort<T,Comparator>::Sort(T& begin, T&end, const Comparator& /*cmp*/)
{
  size_t items = IT_Container<T>::IT_Items(begin,end,begin.Cursor_Type());
  if (items < 1) return;

#ifdef __GNUC__
  T qlst[items];
#else
  T *qlst = new T[items];
#endif

  T c(begin); size_t i=0;

  while (c != end) {
    if (!c) c.To_Begin();
    qlst[i++] = c;
    ++c;
  }
 
  qsort(qlst,items,sizeof(T),Q_Comp<T,Comparator>::Compare);

  IT_Container<T>::IT_ReOrder(begin,end,qlst,items,begin.Cursor_Binding());

#ifndef __GNUC__
  if (qlst) delete[] qlst;
#endif
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
