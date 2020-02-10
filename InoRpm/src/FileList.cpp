//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm List of ProlinerSD File Names ------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Oct 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "FileList.h"

#include "Basics.h"

#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

namespace InoRpm
{
using namespace Ino;

//---------------------------------------------------------------------------

FileString::FileString(const wchar_t *s)
: Persistable(), str(dupStr(s))
{
}

//---------------------------------------------------------------------------

FileString::FileString(const FileString& cp)
: Persistable(cp), str(dupStr(cp.str))
{
}

//---------------------------------------------------------------------------

FileString::~FileString()
{
  delete[] str;
}

//---------------------------------------------------------------------------
// Persistence

static const char fldStr[] = "Str";

//---------------------------------------------------------------------------

void FileString::definePersistentFields(Ino::PersistentWriter& po) const
{
  po.addField(fldStr,typeid(wchar_t *));
}

//---------------------------------------------------------------------------

FileString::FileString(Ino::PersistentReader& pi)
: Persistable(),
  str(pi.readString(fldStr,NULL))
{
}

//---------------------------------------------------------------------------

void FileString::writePersistentObject(Ino::PersistentWriter& po) const
{
  po.writeString(fldStr,str);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void FileList::incCapacity()
{
  nameCap += CapInc;
  nameLst = (FileString **)realloc(nameLst,nameCap*sizeof(FileString *));
}

//---------------------------------------------------------------------------

FileList::FileList()
: Persistable(),
  nameLst(NULL), nameSz(0), nameCap(0)
{
}

//---------------------------------------------------------------------------

static FileString **dupList(const FileString *const *lst, int sz)
{
  if (!lst) return NULL;

  FileString **newLst = new FileString*[sz];

  for (int i=0; i<sz; ++i) newLst[i] = new FileString(*lst[i]);

  return newLst;
}

//---------------------------------------------------------------------------

FileList::FileList(const FileList& cp)
: Persistable(cp),
  nameLst(dupList(cp.nameLst,cp.nameSz)),
  nameSz(cp.nameSz), nameCap(cp.nameSz)
{
}

//---------------------------------------------------------------------------

FileList::~FileList()
{
  for (int i=0; i<nameSz; ++i) delete nameLst[i];
  delete[] nameLst;
}

//---------------------------------------------------------------------------

FileList& FileList::operator=(const FileList& src)
{
  for (int i=0; i<nameSz; ++i) delete nameLst[i];
  delete[] nameLst;

  nameLst = dupList(src.nameLst,src.nameSz);
  nameSz  = src.nameSz;
  nameCap = src.nameSz;

  return *this;
}

//---------------------------------------------------------------------------

void FileList::clear()
{
  for (int i=0; i<nameSz; ++i) {
    delete nameLst[i];
    nameLst[i] = NULL;
  }

  nameSz = 0;
}

//---------------------------------------------------------------------------

const wchar_t *FileList::get(int idx) const
{
  if (idx < 0 || idx >= nameSz)
    throw IndexOutOfBoundsException("FileList::get const");

  return nameLst[idx]->getStr();
}

//---------------------------------------------------------------------------

const wchar_t *FileList::get(int idx)
{
  if (idx < 0 || idx >= nameSz)
    throw IndexOutOfBoundsException("FileList::get");

  return nameLst[idx]->getStr();
}

//---------------------------------------------------------------------------

const wchar_t *FileList::operator[](int idx) const
{
  if (idx < 0 || idx >= nameSz)
    throw IndexOutOfBoundsException("FileList::operator[] const");

  return nameLst[idx]->getStr();
}

//---------------------------------------------------------------------------

const wchar_t *FileList::operator[](int idx)
{
  if (idx < 0 || idx >= nameSz)
    throw IndexOutOfBoundsException("FileList::operator[]");

  return nameLst[idx]->getStr();
}

//---------------------------------------------------------------------------

static int find(const FileString *const *const lst, int lstSz,
                                                    const wchar_t *item)
{
  if (!lst) return 0;

  int lwb = 0, upb = lstSz-1;

  while (lwb <= upb) {
    int idx = (lwb + upb)/2;

    int cmp = compareStr(lst[idx]->getStr(),item);

    if (!cmp) return idx;

    if (cmp < 0) lwb = idx + 1;
    else upb = idx - 1;
  }

  return lwb;
}

//---------------------------------------------------------------------------

const wchar_t *FileList::add(const char *newFile)
{
  if (!newFile) return NULL;

  int sz = strlen(newFile)+1;

  wchar_t *wBuf = (wchar_t *)alloca(sz*sizeof(wchar_t));

  char2WChar(newFile,wBuf);

  return add(wBuf);
}

//---------------------------------------------------------------------------

const wchar_t *FileList::add(const wchar_t *newFile)
{
  if (!newFile) return NULL;

  int idx = find(nameLst,nameSz,newFile);

  if (idx < nameSz && !compareStr(nameLst[idx]->getStr(),newFile)) // Existing!
    return nameLst[idx]->getStr();

  if (nameSz >= nameCap) incCapacity();

  memmove(nameLst+idx+1,nameLst+idx,(nameSz-idx)*sizeof(FileString *));

  nameLst[idx] = new FileString(newFile);

  nameSz++;

  return nameLst[idx]->getStr();
}

//---------------------------------------------------------------------------
// Persistence

static const char fldList[] = "List";

//---------------------------------------------------------------------------

void FileList::definePersistentFields(Ino::PersistentWriter& po) const
{
  po.addObjectArrayField(fldList,(FileString **)NULL);
}

//---------------------------------------------------------------------------

FileList::FileList(Ino::PersistentReader& pi)
: Persistable(),
  nameLst((FileString **)pi.readObjArray(fldList,NULL)),
  nameSz(pi.readArraySize(fldList,0)),
  nameCap(nameSz)
{
  if (!nameLst) {
    nameSz  = 0;
    nameCap = 0;
  }
}

//---------------------------------------------------------------------------

void FileList::writePersistentObject(Ino::PersistentWriter& po) const
{
  po.writeArray(fldList,nameLst,nameSz);
}

} // namespace

//---------------------------------------------------------------------------
