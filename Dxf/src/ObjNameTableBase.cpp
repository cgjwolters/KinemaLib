//---------------------------------------------------------------------------
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

#include "ObjNameTableBase.h"

#include "Basics.h"
#include "DxfReader.h"

#include <cstring>

namespace Ino
{

//using namespace std;

//---------------------------------------------------------------------------

ObjEntryBase::ObjEntryBase(const char *chValue, int hshCode)
: hashCode(hshCode), value(dupStr(chValue)), next(NULL)
{
}

//---------------------------------------------------------------------------

ObjEntryBase::~ObjEntryBase()
{
  if (value) delete[] (char *)value;
}

//---------------------------------------------------------------------------

int ObjEntryBase::compare(const char *chVal) const
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

void ObjNameTableBase::rehash(unsigned int newCap)
{
  ObjEntryBase *fst = NULL;
  ObjEntryBase *last = NULL;

  for (unsigned int i=0; i<cap; ++i) {
    ObjEntryBase *ent = list[i];

    if (!ent) continue;

    if (!fst) fst = last = ent;
    else last = ent;

    while (last->next) last = last->next;

    list[i] = NULL;
  }

  delete[] list;

  cap = newCap;
  list = new ObjEntryBase *[newCap];
  memset(list,0,newCap*sizeof(ObjEntryBase *));

  while (fst) {
    last = fst; fst = last->next;

    int newIdx = last->hashCode % newCap;
    if (newIdx < 0) newIdx += newCap;

    last->next = list[newIdx];
    list[newIdx] = last;
  }
}

//---------------------------------------------------------------------------

ObjNameTableBase::ObjNameTableBase()
: list(new ObjEntryBase *[100]), cap(100), sz(0), extCap(100)
{
  memset(list,0,cap*sizeof(ObjEntryBase *));
}

//---------------------------------------------------------------------------

ObjNameTableBase::ObjNameTableBase(unsigned int initCap, unsigned int capExt)
: list(new ObjEntryBase *[initCap]), cap(initCap), sz(0), extCap(capExt)
{
  memset(list,0,cap*sizeof(ObjEntryBase *));
}

//---------------------------------------------------------------------------

ObjNameTableBase::~ObjNameTableBase()
{
  clear();

  if (list) delete[] list;
}

//---------------------------------------------------------------------------

void ObjNameTableBase::skipGroup(DxfReader& rdr)
{
  while (rdr.next()) {
    if (rdr.code == 102 && rdr.valueSz == 1 && rdr.value[0] == '}') return;
  }
}

//---------------------------------------------------------------------------

void ObjNameTableBase::clear()
{
  for (unsigned int i=0; i<cap; ++i) {
    ObjEntryBase *ent = list[i];

    while (ent) {
      ObjEntryBase *nxt = ent->next;
      delete ent;

      ent = nxt;
    }

    list[i] = NULL;
  }
 
  sz = 0;
}

//---------------------------------------------------------------------------

ObjEntryBase *ObjNameTableBase::getEnt(const char *val) const
{
  int hashCode = calcHashCode(val);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  ObjEntryBase *ent = list[idx];

  while (ent) {
    if (ent->compare(val) == 0) return ent;

    ent = ent->next;
  }

  return NULL;
}

//---------------------------------------------------------------------------

ObjEntryBase *ObjNameTableBase::addEnt(const char *val)
{
  int hashCode = calcHashCode(val);

  int idx = hashCode % cap;
  if (idx < 0) idx += cap;

  ObjEntryBase *ent = list[idx];

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
