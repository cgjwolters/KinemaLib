/* ---------------------------------------------------------------------- */
/* ------- Doubly Linked List Template ---------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ------------- Copyright Inofor Hoek Aut BV 1996, C. Wolters ---------- */
/* ---------------------------------------------------------------------- */

// $Id: it_dlist.h,v 1.2 2009/03/06 10:40:30 clemens Exp $

#ifndef IT_DLIST_INC
#define IT_DLIST_INC


#include "it_base.h"
#include "dlstbase.h"

#include "Basics.h"

#include <stddef.h>

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Item in Doubly Linked Container List ------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

template <class T>
class IT_D_Item : public d_item
{
public:
   T m;
   IT_D_Item(const T& a) : m(a) {}
   IT_D_Item(const IT_D_Item& cp) : m(cp.m) {}

   void * operator new(size_t /*sz*/, IT_D_Item *ip) { return ip; }
   void operator delete(void * /*p*/) {}

#ifndef __BORLANDC__
   void operator delete(void * /*p*/, IT_D_Item * /*ip*/) {}
#endif
};

#ifdef WIN32
#pragma warning( pop )
#endif

/* ---------------------------------------------------------------------- */
/* ------- Const Cursor on Doubly Linked Container List ----------------- */
/* ---------------------------------------------------------------------- */

template <class T, class Alloc> class IT_D_List;
template <class T, class Alloc> class IT_D_Cursor;

template <class T, class Alloc>
class IT_D_C_Cursor : private d_curs
{
 public:
  const IT_Bidirectional *Cursor_Type() const
                              { return (const IT_Bidirectional *)NULL; }
  const IT_Value_Bound *Cursor_Binding() const
                              { return (const IT_Value_Bound *)NULL; }

  IT_D_C_Cursor()                            : d_curs()  {}
  IT_D_C_Cursor(const IT_D_List<T,Alloc>& l) : d_curs(l) {}
  IT_D_C_Cursor(const IT_D_C_Cursor& c)      : d_curs(c) {}

  IT_D_C_Cursor& operator= (const IT_D_C_Cursor& src);

  const IT_D_List<T,Alloc>* Container() const
                             { return (const IT_D_List<T,Alloc>*)list(); }

  IT_D_C_Cursor& To_Begin()       { to_begin(); return *this; }
  IT_D_C_Cursor& To_End  ()       { to_end();   return *this; }
  IT_D_C_Cursor& To_Last ()       { to_last();  return *this; }

  IT_D_C_Cursor& operator++()     { advance();  return *this; }
  IT_D_C_Cursor  operator++(int);
  IT_D_C_Cursor& operator--()     { backup();   return *this; }
  IT_D_C_Cursor  operator--(int);

  bool operator!=(const IT_D_C_Cursor& c) const
                             { return !d_curs::operator==(c);  }
  bool operator==(const IT_D_C_Cursor& c) const
                             { return d_curs::operator==(c);  }
  operator bool() const      { return d_curs::operator bool(); }

  const T* Pred() const;
  const T* Self() const;
  const T* Succ() const;

  const T& operator *() const;
  const T* operator->() const;

  friend class IT_D_Cursor<T,Alloc>;
};

/* ---------------------------------------------------------------------- */
/* ------- Cursor on Doubly Linked Container List ----------------------- */
/* ---------------------------------------------------------------------- */

template <class T, class Alloc>
class IT_D_Cursor
{
  IT_D_C_Cursor<T,Alloc> crs;

 public:
  IT_Bidirectional* Cursor_Type()  const { return (IT_Bidirectional *)NULL; }
  IT_Value_Bound *Cursor_Binding() const { return (IT_Value_Bound *)NULL; }

  IT_D_Cursor()                      : crs()  {}
  IT_D_Cursor(IT_D_List<T,Alloc>& l) : crs(l) {}
  IT_D_Cursor(const IT_D_Cursor& c)  : crs(c) {}

  IT_D_Cursor& operator=(const IT_D_Cursor& src);
  IT_D_List<T,Alloc>* Container() const
                                 { return (IT_D_List<T,Alloc>*)crs.list(); }

  IT_D_Cursor& To_Begin()    { crs.to_begin(); return *this; }
  IT_D_Cursor& To_End()      { crs.to_end();   return *this; }
  IT_D_Cursor& To_Last()     { crs.to_last();  return *this; }

  IT_D_Cursor& operator++()  { crs.advance();  return *this; }
  IT_D_Cursor  operator++(int);

  IT_D_Cursor& operator--()  { crs.backup();   return *this; }
  IT_D_Cursor  operator--(int);

  bool operator!=(const IT_D_C_Cursor<T,Alloc>& c) const
                                                    { return !(crs == c);  }
  bool operator==(const IT_D_C_Cursor<T,Alloc>& c) const
                                                       { return crs == c;  }

  bool operator!=(const IT_D_Cursor& c) const { return !(crs == c.crs);  }
  bool operator==(const IT_D_Cursor& c) const { return crs == c.crs;  }

  operator bool() const      { return crs.operator bool(); }

  operator const IT_D_C_Cursor<T,Alloc>& () const { return crs; }

  T* Pred() const;
  T* Self() const;
  T* Succ() const;

  T& operator *() const;
  T* operator->() const;

  bool Insert(const T& a);
  bool Delete();
  bool Delete(const IT_D_Cursor& upto);

  bool Become_First()     { return crs.become_first(); }

  bool Re_Insert(IT_D_Cursor& it) { return crs.re_insert(it.crs); }
  bool Re_Insert(IT_D_Cursor& from, const IT_D_Cursor& upto)
                              { return crs.re_insert(from.crs,upto.crs); }

  bool Copy_From(const IT_D_C_Cursor<T,Alloc>& from);
  bool Copy_From(const IT_D_C_Cursor<T,Alloc>& from,
                 const IT_D_C_Cursor<T,Alloc>& upto);
};

/* ---------------------------------------------------------------------- */
/* ------- Doubly Linked Container List --------------------------------- */
/* ---------------------------------------------------------------------- */

template <class T, class Alloc>
class IT_D_List : private d_head
{
 public:
  ~IT_D_List() { Delete(); }                 // Destructor

  typedef T Item_T;
  typedef IT_D_Cursor<T,Alloc> Cursor;
  typedef IT_D_C_Cursor<T,Alloc> C_Cursor;

  IT_D_List() : d_head() {}

  IT_D_List(const IT_D_List& clst); // Copy Constructor

  IT_D_List& operator=(const IT_D_List& clst); // Assignment

  IT_D_C_Cursor<T,Alloc> Begin() const
                             { return IT_D_C_Cursor<T,Alloc>(*this); }
  IT_D_C_Cursor<T,Alloc> End  () const;
  IT_D_C_Cursor<T,Alloc> Last () const;

  IT_D_Cursor<T,Alloc> Begin() { return IT_D_Cursor<T,Alloc>(*this); }
  IT_D_Cursor<T,Alloc> End();
  IT_D_Cursor<T,Alloc> Last();

  bool Push_Front(const T& t) { return Begin().Insert(t); }
  bool Push_Back (const T& t) { return End().Insert(t);   }

  bool Pop_Front()            { return Begin().Delete();  }

  bool Pop_Back()             { return Last().Delete();   }

  unsigned int Length() const { return length(); }; // Return number of elements

  operator bool() const { return !empty(); };// Not Empty??

  void Reverse() { reverse(); }

  void Delete();

  void Move_To(IT_D_List& target);
  void Append_To(IT_D_List& target);

  friend class IT_D_C_Cursor<T,Alloc>;
  friend class IT_D_Cursor<T,Alloc>;
};

#include "it_dlist_imp.h"

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
