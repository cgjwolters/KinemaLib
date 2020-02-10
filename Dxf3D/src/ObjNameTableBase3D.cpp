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

#include "ObjNameTableBase3D.h"

#include "Basics.h"
#include "DxfReader3D.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

ObjEntryBase3D::ObjEntryBase3D(const char *chValue, int hshCode)
: hashCode(hshCode), value(dupStr(chValue)), next(NULL)
{
}

//---------------------------------------------------------------------------

ObjEntryBase3D::~ObjEntryBase3D()
{
  if (value) delete[] (char *)value;
}

//---------------------------------------------------------------------------

int ObjEntryBase3D::compare(const char *chVal) const
{
  if (!value) {
    if (chVal) return 1;
  }
  else if (!chVal) return -1;

  return strcmp(value,chVal);
}

//---------------------------------------------------------------------------
//------------ Name Table Methods -------------------------------------------
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

void ObjNameTableBase3D::rehash(unsigned int newCap)
{
  ObjEntryBase3D *fst = NULL;
  ObjEntryBase3D *last = NULL;

  for (unsigned int i=0; i<cap; ++i) {
    ObjEntryBase3D *ent = list[i];

    if (!ent) continue;

    if (!fst) fst = last = ent;
    else last = ent;

    while (last->next) last = last->next;

    list[i] = NULL;
  }

  delete[] list;

  cap = newCap;
  list = new ObjEntryBase3D *[newCap];
  memset(list,0,newCap*sizeof(ObjEntryBase3D *));

  while (fst) {
    last = fst; fst = last->next;

    int newIdx = last->hashCode % newCap;
    if (newIdx < 0) newIdx += newCap;

    last->next = list[newIdx];
    list[newIdx] = last;
  }
}

//---------------------------------------------------------------------------

ObjNameTableBase3D::ObjNameTableBase3D()
: list(new ObjEntryBase3D *[100]), cap(100), sz(0), extCap(100)
{
  memset(list,0,cap*sizeof(ObjEntryBase3D *));
}

//---------------------------------------------------------------------------

ObjNameTableBase3D::ObjNameTableBase3D(unsigned int initCap, unsigned int capExt)
: list(new ObjEntryBase3D *[initCap]), cap(initCap), sz(0), extCap(capExt)
{
  memset(list,0,cap*sizeof(ObjEntryBase3D *));
}

//---------------------------------------------------------------------------

ObjNameTableBase3D::~ObjNameTableBase3D()
{
  clear();

  if (list) delete[] list;
}

//---------------------------------------------------------------------------

void ObjNameTableBase3D::skipGroup(DxfReader3D& rdr)
{
  while (rdr.next()) {
    if (rdr.code == 102 && rdr.valueSz == 1 && rdr.value[0] == '}') return;
  }
}

//---------------------------------------------------------------------------

void ObjNameTableBase3D::clear()
{
  for (unsigned int i=0; i<cap; ++i) {
    ObjEntryBase3D *ent = list[i];

    while (ent) {
      ObjEntryBase3D *nxt = ent->next;
      delete ent;

      ent = nxt;
    }

    list[i] = NULL;
  }
 
  sz = 0;
}

//---------------------------------------------------------------------------

ObjEntryBase3D *ObjNameTableBase3D::getEnt(const char *val) const
{
  int hashCode = calcHashCode(val);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  ObjEntryBase3D *ent = list[idx];

  while (ent) {
    if (ent->compare(val) == 0) return ent;

    ent = ent->next;
  }

  return NULL;
}

//---------------------------------------------------------------------------

ObjEntryBase3D *ObjNameTableBase3D::addEnt(const char *val)
{
  int hashCode = calcHashCode(val);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  ObjEntryBase3D *ent = list[idx];

  while (ent) {
    if (ent->compare(val) == 0) return ent;

    ent = ent->next;
  }

  if (sz >= cap) {
    rehash(sz+extCap);

    idx = hashCode % cap;
    if (idx < 0) idx += cap;
  }

  ent = newObjEntry(val,hashCode);
  ent->next = list[idx];
  list[idx] = ent;

  sz++;

  return ent;
}

} // namespace Ino

//---------------------------------------------------------------------------
