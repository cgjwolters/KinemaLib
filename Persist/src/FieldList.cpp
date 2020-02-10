//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Type.h"
#include "Exceptions.h"

#include <cstdlib>
#include <cstring>

namespace InoPersist
{

using namespace Ino;

//---------------------------------------------------------------------------

void FieldList::insert(short idx, Field *newFld)
{
  if (sz >= cap) {
    cap += CapInc;
    fldLst = (Field **)realloc(fldLst,sizeof(Field *)*cap);
    if (!fldLst) throw OutOfMemoryException("FieldList::add");
  }

  memmove(fldLst+idx+1,fldLst+idx,(sz-idx)*sizeof(Field *));

  fldLst[idx] = newFld;
  sz++;
}

//---------------------------------------------------------------------------

FieldList::FieldList()
: fldLst(new Field *[CapInc]), sz(0), cap(CapInc)
{
}

//---------------------------------------------------------------------------

FieldList::~FieldList()
{
  if (fldLst) {
    for (int i=0; i<sz; ++i) {
      if (fldLst[i]) delete fldLst[i];
    }

    delete[] fldLst;
  }
}

//---------------------------------------------------------------------------

short FieldList::find(const char *fldName) const
{
  short lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    short idx = (lwb+upb)/2;
    int cmp = strcmp(fldName,fldLst[idx]->name);

    if (cmp < 0) upb = idx-1;
    else if (cmp > 0) lwb = idx + 1;
    else return idx;
  }

  return (short)(-(lwb+1));
}

//---------------------------------------------------------------------------

void FieldList::add(const char *fldName, const Type *fieldType)
{
  short newIdx = find(fldName);
  if (newIdx >= 0)
    throw DuplicateNameException("FieldList::add");

  insert(-newIdx-1, new Field(fldName,fieldType));
}

//---------------------------------------------------------------------------

const Field& FieldList::operator[](short idx) const
{
  if (idx < 0 || idx >= sz)
                      throw IndexOutOfBoundsException("FieldList::[] const");

  return *fldLst[idx];
}

//---------------------------------------------------------------------------

Field& FieldList::operator[](short idx)
{
  if (idx < 0 || idx >= sz) throw IndexOutOfBoundsException("FieldList::[]");

  return *fldLst[idx];
}

//---------------------------------------------------------------------------

const Field *FieldList::operator[](const char *name) const
{
  if (!name) throw NullPointerException("FieldList::[] const");
  if (strlen(name) < 1) throw IllegalArgumentException("FieldList::[] const");

  short idx = find(name);

  if (idx < 0) return NULL;

  return fldLst[idx];
}

//---------------------------------------------------------------------------

Field *FieldList::operator[](const char *name)
{
  if (!name) throw NullPointerException("FieldList::[]");
  if (strlen(name) < 1) throw IllegalArgumentException("FieldList::[]");

  short idx = find(name);

  if (idx < 0) return NULL;

  return fldLst[idx];
}

} // namespace InoPersist

//---------------------------------------------------------------------------
