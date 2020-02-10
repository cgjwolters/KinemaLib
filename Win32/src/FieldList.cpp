//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Hash Map of Word MergeFields --------------------------------------
//---------------------------------------------------------------------------
//------- Copyright 2005 Prodim International BV ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters Aug 2005 -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include <StdAfx.h>

#include "FieldList.h"

#include "Basics.h"
#include <wchar.h>

namespace AW
{

using namespace Word;
using namespace std;

//---------------------------------------------------------------------------

class Field 
{
  enum { CapInc = 64 };

  FieldPtr **ptrLst;
  int ptrSz, ptrCap;

  Field(const Field& cp);             // No Copying
  Field& operator=(const Field& src); // No Assignment

  void incCapacity();

public:
  Field(const wchar_t *fldName);
  ~Field();

  void add(FieldPtr& ptr);
  bool get(int idx, FieldPtr& ptr) const;

  const wchar_t *const name;
  Field *next;
};

//---------------------------------------------------------------------------

void Field::incCapacity()
{
  ptrCap += CapInc;

  ptrLst = (FieldPtr **)realloc(ptrLst,ptrCap*sizeof(FieldPtr *));
}

//---------------------------------------------------------------------------

Field::Field(const wchar_t *fldName)
: next(NULL), name(dupStr(fldName)),
  ptrLst(NULL), ptrSz(0), ptrCap(0)
{
}

//---------------------------------------------------------------------------

Field::~Field()
{
  if (name) delete[] name;

  if (ptrLst) {
    for (int i=0; i<ptrSz; ++i) {
      if (ptrLst[i]) {
        *ptrLst[i] = NULL;
        delete ptrLst[i];
      }
    }

    delete[] ptrLst;
  }
}

//---------------------------------------------------------------------------

void Field::add(FieldPtr& ptr)
{
  if (ptrSz >= ptrCap) incCapacity();

  ptrLst[ptrSz++] = new FieldPtr(ptr);
}

//---------------------------------------------------------------------------

bool Field::get(int idx, FieldPtr& ptr) const
{
  if (idx < 0 || idx >= ptrSz) return false;

  if (!ptrLst[idx]) return false;

  ptr = *ptrLst[idx];

  return true;
}

//---------------------------------------------------------------------------

void FieldList::incCapacity()
{
  int oldCap = fldCap;
  if (!fldLst) oldCap = 0;

  fldCap += CapInc;

  Field **newLst = new Field*[fldCap];
  memset(newLst,0,fldCap*sizeof(Field *));

  for (int i=0; i<oldCap; ++i) {
    Field *cur = fldLst[i];

    while (cur) {
      Field *nxt = cur->next;

      int idx = hashCode(cur->name) % fldCap;
      if (idx < 0) idx += fldCap;

      cur->next = newLst[idx];
      newLst[idx] = cur;

      cur = nxt;
    }
  }

  if (fldLst) delete[] fldLst;
  fldLst = newLst;
}

//---------------------------------------------------------------------------

FieldList::FieldList()
: fldLst(NULL), fldSz(0), fldCap(0), valid(false)
{
}

//---------------------------------------------------------------------------

FieldList::~FieldList()
{
  clear();

  if (fldLst) delete[] fldLst;
}

//---------------------------------------------------------------------------

void FieldList::clear()
{
  valid = false;

  for (int i=0; i<fldCap; ++i) {
    Field *cur = fldLst[i];

    while (cur) {
      Field *nxt = cur->next;
      delete cur;

      cur = nxt;
    }

    fldLst[i] = NULL;
  }

  fldSz = 0;
}

//---------------------------------------------------------------------------

void FieldList::addField(const wchar_t *name, FieldPtr& fldPtr)
{
  if (!name) return;

  if (fldSz >= fldCap) incCapacity();

  int idx = hashCode(name) % fldCap;
  if (idx < 0) idx += fldCap;

  Field *cur = fldLst[idx];

  while (cur) {
    if (!wcscmp(cur->name,name)) {
      cur->add(fldPtr);
      return;
    }

    cur = cur->next;
  }

  // Not found, so add:

  Field *fld = new Field(name);
  fld->add(fldPtr);

  fld->next = fldLst[idx];
  fldLst[idx] = fld;

  fldSz++;
}

//---------------------------------------------------------------------------

static const wchar_t *getFldName(const wchar_t *mrgFld)
{
  if (!wcsstr(mrgFld,L"MERGEFIELD")) return NULL;

  return mrgFld+11;
}

//---------------------------------------------------------------------------

void FieldList::buildField(FieldPtr& fieldPtr)
{
  enum WdFieldType fldType;
  if (!SUCCEEDED(fieldPtr->get_Type(&fldType))||
                            fldType != wdFieldMergeField) return;

  RangePtr rangePtr;
  if (!SUCCEEDED(fieldPtr->get_Code(&rangePtr)) ||
                                    rangePtr == NULL) return;

  _bstr_t name;
  if (!SUCCEEDED(rangePtr->get_Text(name.GetAddress()))) return;

  if (!wcsstr(name,L"MERGEFIELD")) return;

  wchar_t *s = name; s += 11;     // Skip word MERGEFIELD
  while (*s && iswspace(*s)) s++; // Skip leading whitespace
  if (!*s) return;

  const wchar_t *fldName = s;

  s++;
  while (*s && !iswspace(*s)) s++; // Find end of name
  *s = 0;

  addField(fldName,fieldPtr);
}

//---------------------------------------------------------------------------

bool FieldList::build(_DocumentPtr& docPtr)
{
  FieldsPtr fieldsPtr;
  if (!SUCCEEDED(docPtr->get_Fields(&fieldsPtr)) ||
                                   fieldsPtr == NULL) return false;


  long fieldCount = 0;
  if (!SUCCEEDED(fieldsPtr->get_Count(&fieldCount))) return false;

  FieldPtr fieldPtr;

  for (long i=1; i<=fieldCount; ++i) {
    if (!SUCCEEDED(fieldsPtr->Item(i,&fieldPtr)) ||
                                          fieldPtr == NULL) continue;
    buildField(fieldPtr);
  }

  // Fields in frames (old fashioned)

  fieldsPtr = NULL;
  valid = true;

  FramesPtr framesPtr;
  if (!SUCCEEDED(docPtr->get_Frames(&framesPtr)) ||
                                   framesPtr == NULL) return true;

  long frameCount = 0;
  if (!SUCCEEDED(framesPtr->get_Count(&frameCount))) return true;

  for (i=1; i<=frameCount; ++i) {
    FramePtr framePtr;
    if (!SUCCEEDED(framesPtr->Item(i,&framePtr)) ||
                                          framePtr == NULL) continue;

    RangePtr rangePtr;
    if (!SUCCEEDED(framePtr->get_Range(&rangePtr)) ||
                                      rangePtr == NULL) continue;

    if (!SUCCEEDED(rangePtr->get_Fields(&fieldsPtr)) ||
                                      fieldsPtr == NULL) continue;
    
    fieldCount = 0;
    if (!SUCCEEDED(fieldsPtr->get_Count(&fieldCount))) return false;

    for (long i=1; i<=fieldCount; ++i) {
      if (!SUCCEEDED(fieldsPtr->Item(i,&fieldPtr)) ||
                                            fieldPtr == NULL) continue;
      buildField(fieldPtr);
    }
  }

  // No fields in shapes yet!!!

  return true;
}

//---------------------------------------------------------------------------

bool FieldList::find(const wchar_t *name, int seqNr, FieldPtr& fld) const
{
  if (!name) return false;

  int idx = hashCode(name) % fldCap;
  if (idx < 0) idx += fldCap;

  const Field *cur = fldLst[idx];

  while (cur) {
    if (!wcscmp(cur->name,name)) return cur->get(seqNr,fld);

    cur = cur->next;
  }

  return false;
}

} // namespace AW

//---------------------------------------------------------------------------

