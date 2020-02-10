/* ---------------------------------------------------------------------- */
/* ------- Doubly Linked List Base Classes ------------------------------ */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Inofor Hoek Aut. BV 1996, C. Wolters -- */
/* ---------------------------------------------------------------------- */

// $Id: dlstbase.h,v 1.1 2008/10/16 09:10:57 clemens Exp $

#ifndef DLIST_BASE_INC
#define DLIST_BASE_INC

#include "Basics.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Base item in a linked list ----------------------------------- */
/* ---------------------------------------------------------------------- */

class d_curs;

class d_item
{
  d_item(const d_item& cs);              // No copying
  d_item& operator=(const d_item& cs);   // No Assignment

 public:

  d_item *prv;
  d_item *nxt;

  d_item() : prv(NULL), nxt(NULL) { prv = this; nxt = this; }
};

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

class d_head
{
  d_item *head;
  unsigned long   count;

  d_head& operator=(const d_head& chead); // No assignment
  d_head(const d_head& chead);            // No copying

 protected:

  d_head() : head(NULL),count(0) {};

  bool empty() const { return head == NULL; }

  unsigned long length() const { return count; };

  void reverse();

  bool  insert(d_item *before, d_item *ins_item, unsigned long newcount);
  bool  remove(d_item *del_item);
  unsigned long remove(d_item *from_item, d_item *upto_item);

  void move_to(d_head& target);

  friend class d_curs;
};

/* ---------------------------------------------------------------------- */
/* ----------------- Cursor on list ------------------------------------- */
/* ---------------------------------------------------------------------- */

class d_curs
{
  d_head  *lst;
  d_item  *item;

 protected:

  d_curs() : lst(NULL), item(NULL) {}

  // Initialize to first item
  d_curs(const d_head& mylst) : lst(&(d_head&)mylst), item(mylst.head) {}

  d_curs(const d_curs& cp_cursor)
                            : lst(cp_cursor.lst), item(cp_cursor.item) {}

  d_curs& operator=(const d_curs& src)
                        { lst = src.lst; item = src.item; return *this; }

  d_head* list() const { return lst; }

  bool become_first();

  d_item *pred() const;
  d_item *self() const;
  d_item *succ() const;

  void ill_deref() const;

  void to_begin();
  void to_end() { item = NULL; }
  void to_last();

  void advance();
  void backup();

  bool operator==(const d_curs& c) const
                       { return lst && lst == c.lst && item == c.item; }
  operator bool() const { return lst && item; }

  bool insert(d_item* it, int inscount);                // Insert before

  bool re_insert(d_curs& it);                           // Re_insert before
  bool re_insert(d_curs& from, const d_curs& upto);     // Re_insert range before

  d_item *remove();                                     // Take from list
  d_item *remove(const d_curs& upto,int &remove_count); // Take range from list

  friend class d_head;
};

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
