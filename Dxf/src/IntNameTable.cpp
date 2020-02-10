//---------------------------------------------------------------------------
//--------- Dxf Format Reader -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters June 2005----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "IntNameTable.h"

#include "Basics.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

class IntEntry
{
  IntEntry(const IntEntry&cp);              // No Copying
  IntEntry& operator=(const IntEntry& src); // No Assignment

public:
  IntEntry(const char *chValue, int hshCode, int newId);
  ~IntEntry();

  const int id;
  const int hashCode;
  const char *const value;

  IntEntry *next;

  int compare(const char *str) const;
};

//---------------------------------------------------------------------------

IntEntry::IntEntry(const char *chValue, int hshCode, int newId)
: id(newId), hashCode(hshCode), value(dupStr(chValue)), next(NULL)
{
}

//---------------------------------------------------------------------------

IntEntry::~IntEntry()
{
  if (value) delete[] (char *)value;
}

//---------------------------------------------------------------------------

int IntEntry::compare(const char *str) const
{
  if (!value) {
    if (str) return 1;
  }
  else if (!str) return -1;

  return strcmp(value,str);
}

//---------------------------------------------------------------------------

static int calcHashCode(const char *value)
{
  if (!value) return 0;

  int code = 0;

  while (*value) {
    code += *value++;
    code <<= 1;
  }

  return code;
}

//---------------------------------------------------------------------------
//---------- Name Table Methods ---------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void IntNameTable::rehash(unsigned int newCap)
{
  IntEntry *fst = NULL;
  IntEntry *last = NULL;

  for (unsigned int i=0; i<cap; ++i) {
    IntEntry *ent = list[i];

    if (!ent) continue;

    if (!fst) fst = last = ent;
    else last = ent;

    while (last->next) last = last->next;

    list[i] = NULL;
  }

  delete[] list;

  cap = newCap;
  list = new IntEntry *[newCap];
  memset(list,0,newCap*sizeof(IntEntry *));

  while (fst) {
    last = fst; fst = last->next;

    int newIdx = last->hashCode % newCap;
    if (newIdx < 0) newIdx += newCap;

    last->next = list[newIdx];
    list[newIdx] = last;
  }
}

//---------------------------------------------------------------------------

IntNameTable::IntNameTable()
: list(new IntEntry *[100]), sz(0), cap(100), extCap(100)
{
  memset(list,0,cap*sizeof(IntEntry *));
}

//---------------------------------------------------------------------------

IntNameTable::IntNameTable(int initCap, int capExt)
: list(new IntEntry *[initCap]), sz(0), cap(initCap), extCap(capExt)
{
  memset(list,0,cap*sizeof(IntEntry *));
}

//---------------------------------------------------------------------------

IntNameTable::~IntNameTable()
{
  if (list) {
    for (unsigned int i=0; i<cap; ++i) {
      IntEntry *ent = list[i];

      while (ent) {
        IntEntry *nxt = ent->next;
        delete ent;

        ent = nxt;
      }
    }

    delete[] list;
  }
}

//---------------------------------------------------------------------------

int IntNameTable::getNameId(const char *value) const
{
  int hashCode = calcHashCode(value);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  IntEntry *ent = list[idx];

  while (ent) {
    if (ent->compare(value) == 0) return ent->id;

    ent = ent->next;
  }

  return -1;
}

//---------------------------------------------------------------------------

int IntNameTable::addName(const char *value, int id)
{
  int hashCode = calcHashCode(value);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  IntEntry *ent = list[idx];

  while (ent) {
    if (ent->compare(value) == 0) return ent->id;

    ent = ent->next;
  }

  if (sz >= cap) {
    rehash(sz+extCap);

    idx = hashCode % cap;
    if (idx < 0) idx += cap;
  }

  ent = new IntEntry(value,hashCode,id);
  ent->next = list[idx];
  list[idx] = ent;

  sz++;

  return ent->id;
}

} // namespace Ino

//---------------------------------------------------------------------------
