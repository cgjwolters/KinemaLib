/* ---------------------------------------------------------------------- */
/* ------- Doubly Linked List Base Classes ------------------------------ */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* -------------------- Copyright Inofor Hoek Aut. BV 1996, C. Wolters -- */
/* ---------------------------------------------------------------------- */

#include "dlstbase.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void d_head::reverse()
{
   if (!head) return;

   d_item *cur = head;

   do {
     d_item *hold = cur->nxt; cur->nxt = cur->prv; cur->prv = hold;

     cur = cur->prv;
   } while (cur != head);

   head = head->nxt;
}

/* ---------------------------------------------------------------------- */
/* ------- Insert new item(s) in the list ------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_head::insert(d_item *before, d_item *ins_item, unsigned long newcount)
{
  if (!ins_item) return false;

  count += newcount;

  if (!head) head = ins_item;
  else {
    d_item *lbef = before;

    if (!lbef) lbef = head;

    d_item *prv = ins_item->prv;

    ins_item->prv = lbef->prv; lbef->prv->nxt = ins_item;
    prv->nxt = lbef; lbef->prv = prv;

    if (before == head) head = ins_item;
  }

  return true;
}

/* ---------------------------------------------------------------------- */
/* ------- Remove an item ----------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_head::remove(d_item *del_item)
{
  if (!del_item || !head) return false;

  count--;

  if (head == del_item) head = del_item->nxt;
  if (head == del_item) head = NULL;

  if (!head) return true;

  d_item *prv = del_item->prv;
  del_item->prv = del_item; prv->nxt = del_item->nxt;
  del_item->nxt->prv = prv; del_item->nxt = del_item;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ------- Remove a list of items from the list ------------------------- */
/* ---------------------------------------------------------------------- */

unsigned long d_head::remove(d_item *from_item, d_item *upto_item)
{
  if (!head || !from_item) return 0;

  if (from_item == upto_item) return 0;

  if (!upto_item) upto_item = head;

  if (from_item == upto_item) {
    head = NULL;
    unsigned long oldcount = count;
    count = 0;
    return oldcount;
  }

  unsigned long delcount = 0;
  d_item *it = from_item;

  while (it != upto_item) {
    if (it == head) head = upto_item;
    delcount++;
    it = it->nxt;
  }

  count -= delcount;

  d_item *prv = from_item->prv;
  from_item->prv = upto_item->prv;
  prv->nxt = upto_item;
  upto_item->prv = prv;
  from_item->prv->nxt = from_item;

  return delcount;
}

/* ---------------------------------------------------------------------- */
/* ------- Move the contents of the list to another list ---------------- */
/* ---------------------------------------------------------------------- */

void d_head::move_to(d_head& target)
{
  target.head = head;
  target.count = count;
  head = NULL;
  count = 0;
}

/* ---------------------------------------------------------------------- */
/* ------- Cursor ------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_curs::become_first()
{
  if (lst && lst->head && item) {
    lst->head = item;
    return true;
  }

  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

d_item *d_curs::pred() const
{
  if (!lst || !lst->head) return NULL;

  if (!item) return lst->head->prv;

  return item->prv;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

d_item *d_curs::self() const
{
  if (!lst || !lst->head) return NULL;

  return item;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

d_item *d_curs::succ() const
{
  if (!lst || !lst->head) return NULL;

  if (!item) return lst->head;

  return item->nxt;
}

/* ---------------------------------------------------------------------- */
/* ------- Called when a NULL cursor dereference occurs!!! -------------- */
/* ---------------------------------------------------------------------- */

void d_curs::ill_deref() const
{
  int i = 10;
  i += 5;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void d_curs::to_begin()
{
  if (lst) item = lst->head;
  else     item = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void d_curs::to_last()
{
  if (lst) {
    item = lst->head;
    if (item) item = item->prv;
  }
  else item = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void d_curs::advance()
{
  if (lst && lst->head) {
    if (item) {
      item = item->nxt;
      if (item == lst->head) item = NULL;
    }
  }
  else item = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void d_curs::backup()
{
  if (lst && lst->head) {
    if (item) {
      if (item == lst->head) item = NULL;
      else item = item->prv;
    }
  }
  else item = NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_curs::insert(d_item* it, int inscount)
{
  if (lst && lst->insert(item,it,inscount)) {
    item = it;
    return true;
  }
  return false;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_curs::re_insert(d_curs& it)
{
  if (!lst) return false;  // Don't even remove if you can't insert
  if (operator==(it)) return true;

  return insert(it.remove(),1);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool d_curs::re_insert(d_curs& from, const d_curs& upto)
{
  if (!lst) return false;

  d_item *from_item = from.item;

  int remove_count;
  d_item *it = from.remove(upto,remove_count);

  if (!lst->head) item = NULL;
  else if (it && lst == from.lst) {
    d_item *cit = it;
    do {
      if (cit == item) {
        item = upto.item;
        from.item = from_item; // This entire method will turn out a no-op
        break;
      }
      cit = cit->nxt;
    } while (cit != it);
  }

  return insert(it,remove_count);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

d_item *d_curs::remove()
{
  if (!lst || !item) return NULL;

  d_item *hold    = item;
  d_item *holdnxt = item->nxt;
  if (holdnxt == lst->head) holdnxt = NULL;

  if (lst->remove(item)) {
    item = holdnxt;
    return hold;
  }

  return NULL;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

d_item *d_curs::remove(const d_curs& upto, int &remove_count)
{
  if (!lst || lst != upto.lst) return NULL;

  remove_count = lst->remove(item,upto.item);
  if (remove_count) {
    d_item *hold = item;
    if (!lst->head) item = NULL;
    else item = upto.item;
    return hold;
  }

  return NULL;
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ------- End of module ------------------------------------------------ */
/* ---------------------------------------------------------------------- */
