//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Capability List --------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "CapabilityList.h"

#include "Capability.h"

#include "Basics.h"
#include "Exceptions.h"

#include <malloc.h>
#include <string.h>

#include <stdlib.h>

namespace InoRpm
{

using namespace Ino;

//---------------------------------------------------------------------------

void CapabilityList::incCapacity()
{
  cap += CapInc;
  lst = (Capability **)realloc(lst,cap*sizeof(Capability *));
}

//---------------------------------------------------------------------------

CapabilityList::CapabilityList(int capType)
: Persistable(), type(capType),
  lst(NULL), sz(0), cap(0),
  depSet(NULL)
{
}

//---------------------------------------------------------------------------

static Capability **dupList(Capability **lst, int sz)
{
  if (!lst) return NULL;

  Capability **newLst = new Capability*[sz];

  for (int i=0; i<sz; ++i) newLst[i] = new Capability(*lst[i]);

  return newLst;
}

//---------------------------------------------------------------------------

CapabilityList::CapabilityList(const CapabilityList& cp)
: Persistable(cp), type(cp.type),
  lst(dupList(cp.lst,cp.sz)), sz(cp.sz), cap(cp.sz),
  depSet(NULL)
{
}

//---------------------------------------------------------------------------

CapabilityList::~CapabilityList()
{
  for (int i=0; i<sz; ++i) { // Leave the braces or else crash!! (Compiler error ???)
    delete lst[i];
  }

  delete[] lst;

#ifndef _WIN32
  rpmdsFree(depSet);
#endif
}

//---------------------------------------------------------------------------

void CapabilityList::setType(int capType)
{
  type = capType;
}

//---------------------------------------------------------------------------

void CapabilityList::clear()
{
  for (int i=0; i<sz; ++i) {
    delete lst[i];
    lst[i] = NULL;
  }

  sz = 0;

#ifndef _WIN32
  depSet = rpmdsFree(depSet);
#endif
}

//---------------------------------------------------------------------------

rpmds CapabilityList::getDepSet() const
{
#ifdef _WIN32
  return NULL;
#else
  if (depSet) return depSet;

  for (int i=0; i<sz; ++i) {
    const Capability& capa = *lst[i];

    rpmds ds = rpmdsSingle((rpmTag)type,capa.getName(),capa.getVersion(),
                                           (rpmsenseFlags)capa.getFlags());
    rpmdsMerge(&depSet,ds);
    rpmdsFree(ds);
  }

  return depSet;
#endif
}

//---------------------------------------------------------------------------

const Capability& CapabilityList::operator[](int idx) const
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("CapabilityList::[] const");

  return *lst[idx];
}

//---------------------------------------------------------------------------

Capability& CapabilityList::operator[](int idx)
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("CapabilityList::[]");

  return *lst[idx];
}

//---------------------------------------------------------------------------

const Capability *CapabilityList::find(const char *name) const
{
  if (!name) return NULL;

  for (int i=0; i<sz; ++i) {
    if (!compareStr(name,lst[i]->getName())) return lst[i];
  }

  return NULL;
}

//---------------------------------------------------------------------------

const Capability& CapabilityList::get(int idx) const
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("CapabilityList::get");

  return *lst[idx];
}

//---------------------------------------------------------------------------

void CapabilityList::add(const Capability& capability)
{
  if (sz >= cap) incCapacity();

  lst[sz++] = new Capability(capability);

#ifndef _WIN32
  depSet = rpmdsFree(depSet);
#endif
}

//---------------------------------------------------------------------------

void CapabilityList::addUnique(const Capability& capability)
{
  for (int i=0; i<sz; ++i) {
    if (!compareStr(lst[i]->getName(),capability.getName())) {
      if (&lst[i]->getPackage() == &capability.getPackage()) return;
    }
  }
  
  add(capability);
}

//---------------------------------------------------------------------------

void CapabilityList::remove(int idx)
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("CapabilityList::remove");

  delete lst[idx];

  sz--;
  memmove(lst+idx,lst+idx+1,(sz-idx)*sizeof(Capability *));

#ifndef _WIN32
  depSet = rpmdsFree(depSet);
#endif
}

//---------------------------------------------------------------------------
// Persistence section:

static const char fldType[] = "Type";
static const char fldLst[]  = "List";

//---------------------------------------------------------------------------

void CapabilityList::definePersistentFields(Ino::PersistentWriter& po) const
{
  po.addField(fldType,typeid(long));
  po.addObjectArrayField(fldLst,(Capability **)NULL);
}

//---------------------------------------------------------------------------

CapabilityList::CapabilityList(Ino::PersistentReader& pi)
: Persistable(),
  type(pi.readInt(fldType,0)),
  lst((Capability **)pi.readObjArray(fldLst,NULL)),
  sz(pi.readArraySize(fldLst,0)),cap(sz),
  depSet(NULL)
{
  if (!lst) sz = cap = 0;
}

//---------------------------------------------------------------------------

void CapabilityList::writePersistentObject(Ino::PersistentWriter& po) const
{
  po.writeInt(fldType,type);
  po.writeArray(fldLst,lst,sz);
}

} // namespace

//---------------------------------------------------------------------------
