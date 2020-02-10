//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf 3D Format Reader --------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2008 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Sept 2008----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "IntNameTable3D.h"

#include "Basics.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

class IntEntry3D
{
  IntEntry3D(const IntEntry3D&cp);              // No Copying
  IntEntry3D& operator=(const IntEntry3D& src); // No Assignment

public:
  IntEntry3D(const char *chValue, int hshCode, int newId);
  ~IntEntry3D();

  const int id;
  const int hashCode;
  const char *const value;

  IntEntry3D *next;

  int compare(const char *str) const;
};

//---------------------------------------------------------------------------

IntEntry3D::IntEntry3D(const char *chValue, int hshCode, int newId)
: id(newId), hashCode(hshCode), value(dupStr(chValue)), next(NULL)
{
}

//---------------------------------------------------------------------------

IntEntry3D::~IntEntry3D()
{
  if (value) delete[] (char *)value;
}

//---------------------------------------------------------------------------

int IntEntry3D::compare(const char *str) const
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

void IntNameTable3D::rehash(unsigned int newCap)
{
  IntEntry3D *fst = NULL;
  IntEntry3D *last = NULL;

  for (unsigned int i=0; i<cap; ++i) {
    IntEntry3D *ent = list[i];

    if (!ent) continue;

    if (!fst) fst = last = ent;
    else last = ent;

    while (last->next) last = last->next;

    list[i] = NULL;
  }

  delete[] list;

  cap = newCap;
  list = new IntEntry3D *[newCap];
  memset(list,0,newCap*sizeof(IntEntry3D *));

  while (fst) {
    last = fst; fst = last->next;

    int newIdx = last->hashCode % newCap;
    if (newIdx < 0) newIdx += newCap;

    last->next = list[newIdx];
    list[newIdx] = last;
  }
}

//---------------------------------------------------------------------------

IntNameTable3D::IntNameTable3D()
: list(new IntEntry3D *[100]), sz(0), cap(100), extCap(100)
{
  memset(list,0,cap*sizeof(IntEntry3D *));
}

//---------------------------------------------------------------------------

IntNameTable3D::IntNameTable3D(int initCap, int capExt)
: list(new IntEntry3D *[initCap]), sz(0), cap(initCap), extCap(capExt)
{
  memset(list,0,cap*sizeof(IntEntry3D *));
}

//---------------------------------------------------------------------------

IntNameTable3D::~IntNameTable3D()
{
  if (list) {
    for (unsigned int i=0; i<cap; ++i) {
      IntEntry3D *ent = list[i];

      while (ent) {
        IntEntry3D *nxt = ent->next;
        delete ent;

        ent = nxt;
      }
    }

    delete[] list;
  }
}

//---------------------------------------------------------------------------

int IntNameTable3D::getNameId(const char *value) const
{
  int hashCode = calcHashCode(value);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  IntEntry3D *ent = list[idx];

  while (ent) {
    if (ent->compare(value) == 0) return ent->id;

    ent = ent->next;
  }

  return -1;
}

//---------------------------------------------------------------------------

int IntNameTable3D::addName(const char *value, int id)
{
  int hashCode = calcHashCode(value);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  IntEntry3D *ent = list[idx];

  while (ent) {
    if (ent->compare(value) == 0) return ent->id;

    ent = ent->next;
  }

  if (sz >= cap) {
    rehash(sz+extCap);

    idx = hashCode % cap;
    if (idx < 0) idx += cap;
  }

  ent = new IntEntry3D(value,hashCode,id);
  ent->next = list[idx];
  list[idx] = ent;

  sz++;

  return ent->id;
}

} // namespace Ino

//---------------------------------------------------------------------------
