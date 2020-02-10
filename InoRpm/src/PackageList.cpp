//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Package List -----------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "PackageList.h"

#include "Package.h"
#include "Capability.h"
#include "CapabilityList.h"

#include "Basics.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
typedef void *Header;
#else
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#endif

namespace InoRpm
{

using namespace Ino;

//---------------------------------------------------------------------------

bool PackageList::mustInit = true;

//---------------------------------------------------------------------------

#ifndef _WIN32

static rpmds rpmDepSet = NULL;

//---------------------------------------------------------------------------

void PackageList::buildFromRpmDb(bool genFiles)
{
  rpmts trx = rpmtsCreate();

  rpmdbMatchIterator iter = rpmtsInitIterator(trx,RPMTAG_NAME,NULL,0);

  Header hdr = NULL;

  while ((hdr = rpmdbNextIterator(iter))) {
    Package pkg(hdr,genFiles);
    pkg.st = Package::StInstalled;

    add(pkg);
  }

  if (iter) rpmdbFreeIterator(iter);

  rpmtsFree(trx);
}

//---------------------------------------------------------------------------

static PackageList::RpmRc convRc(rpmRC rc)
{
  switch (rc) {
    case RPMRC_OK:         return PackageList::RpmOk;
    case RPMRC_NOTFOUND:   return PackageList::RpmNotFound;
    case RPMRC_NOTTRUSTED: return PackageList::RpmNotTrusted;
    case RPMRC_NOKEY:      return PackageList::RpmNoKey;
    case RPMRC_FAIL:
    default:               return PackageList::RpmFail;
  }
}

//---------------------------------------------------------------------------

PackageList::RpmRc PackageList::addFromRpmFile(const char *file)
{
  if (!file) return RpmFail;

  FD_t fd = Fopen(file,"rb");

  if (!fd) return RpmFail;

  rpmts trx = rpmtsCreate();

  Header hdr = NULL;
  rpmRC rc = rpmReadPackageFile(trx,fd,NULL,&hdr);

  RpmRc rpmRc = convRc(rc);

  if (rpmRc == RpmOk || rpmRc == RpmNoKey || rpmRc == RpmNotTrusted) {
    Package pkg(hdr,true,file);
    pkg.st = Package::StNew;

    add(pkg);
  }

  headerFree(hdr);
  rpmtsFree(trx);
  Fclose(fd);

  return rpmRc;
}

#endif

//---------------------------------------------------------------------------

void PackageList::incCapacity()
{
  cap += CapInc;
  lst = (Package **)realloc(lst,cap*sizeof(Package *));
}

//---------------------------------------------------------------------------

PackageList::PackageList(const char *src)
: Persistable(),
  lst(NULL), sz(0), cap(0), source(dupStr(src))
{
#ifndef _WIN32
  if (mustInit) {
    rpmReadConfigFiles(NULL,NULL);
    rpmdsRpmlib(&rpmDepSet,NULL);
  }
#endif

  mustInit = false;
}

//---------------------------------------------------------------------------

static Package **dupList(Package **lst, int sz)
{
  if (!lst) return NULL;

  Package **newLst = new Package*[sz];

  for (int i=0; i<sz; ++i) newLst[i] = new Package(*lst[i]);

  return newLst;
}

//---------------------------------------------------------------------------

PackageList::PackageList(const PackageList& cp)
: Persistable(cp),
  lst(dupList(cp.lst,cp.sz)), sz(cp.sz), cap(cp.sz),
  source(dupStr(cp.source))
{
}

//---------------------------------------------------------------------------

PackageList::~PackageList()
{
  for (int i=0; i<sz; ++i) delete lst[i];
  delete[] lst;

  delete[] source;
}

//---------------------------------------------------------------------------

PackageList& PackageList::operator=(const PackageList& src)
{
  clear();

  if (src.sz >= cap) {
    cap = src.sz;
    lst = (Package **)realloc(lst,cap*sizeof(Package *));
  }

  sz = src.sz;
  for (int i=0; i<sz; ++i) lst[i] = new Package(*src.lst[i]);

  delete[] source;
  source = dupStr(src.source);

  return *this;
}

//---------------------------------------------------------------------------

void PackageList::clear()
{
  for (int i=0; i<sz; ++i) {
    delete lst[i];
    lst[i] = NULL;
  }

  sz = 0;

  delete[] source;
  source = NULL;
}

//---------------------------------------------------------------------------

void PackageList::setInstalledVersions(const PackageList& installedLst)
{
  for (int i=0; i<sz; ++i) {
    lst[i]->setInstalledVer(NULL);

    int idx = installedLst.find(lst[i]->getName());
    
    if (idx >= 0) lst[i]->setInstalledVer(&installedLst[idx].nameVersionRel);
  }
}

//---------------------------------------------------------------------------

int PackageList::find(const char *name) const
{
  if (!name) return -1;

  for (int i=0; i<sz; ++i) {
    if (!compareStr(name,lst[i]->getName())) return i;
  }

  return -1;
}

//---------------------------------------------------------------------------

const Package& PackageList::get(int idx) const
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("PackageList::get const");

  return *lst[idx];
}

//---------------------------------------------------------------------------

Package& PackageList::get(int idx)
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("PackageList::get");

  return *lst[idx];
}

//---------------------------------------------------------------------------

const Package& PackageList::operator[](int idx) const
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("PackageList::[] const");

  return *lst[idx];
}

//---------------------------------------------------------------------------

Package& PackageList::operator[](int idx)
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("PackageList::[]");

  return *lst[idx];
}

//---------------------------------------------------------------------------

Package& PackageList::add(const Package& pkg)
{
  if (sz >= cap) incCapacity();

  int lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    int idx = (lwb+upb)/2;

    int cmp = lst[idx]->compare(pkg);

    if (cmp < 0) lwb = idx+1;
    else if (cmp > 0) upb = idx-1;
    else {
      lwb = idx;
      break;
    }
  }

  memmove(lst+lwb+1,lst+lwb,(sz-lwb)*sizeof(Package *));

  lst[lwb] = new Package(pkg);

  Package& newPkg = *lst[lwb];

  sz++;

  return newPkg;
}

//---------------------------------------------------------------------------

void PackageList::remove(int idx)
{
  if (idx < 0 || idx >= sz)
    throw IndexOutOfBoundsException("PackageList::remove");

  delete lst[idx];

  sz--;
  memmove(lst+idx,lst+idx+1,(sz-idx)*sizeof(Package *));
}

//---------------------------------------------------------------------------

PackageList::RpmRc PackageList::addRpmFile(const char *filePath)
{
#ifndef _WIN32
  return addFromRpmFile(filePath);
#else
  return RpmFail;
#endif
}

//---------------------------------------------------------------------------

bool PackageList::buildFromInstalled(bool genFiles)
{
  clear();

#ifndef _WIN32
  buildFromRpmDb(genFiles);
  return true;
#else
  return false;
#endif
}

//---------------------------------------------------------------------------

#ifdef _WIN32
void PackageList::buildReqLst(CapabilityList& reqLst) const
{
}

//---------------------------------------------------------------------------

void PackageList::buildConflictLst(CapabilityList& conflictLst) const
{
}

#else

//---------------------------------------------------------------------------

void PackageList::buildReqLst(CapabilityList& reqLst) const
{
  for (int i=0; i<sz; ++i) {
    if (lst[i]->getState() == Package::StDelete) continue;

    rpmds reqDs = lst[i]->requires.getDepSet();

    rpmdsInit(reqDs);

    int idx = -1;

    while ((idx = rpmdsNext(reqDs)) >= 0) {
      bool found = false;

      for (int j=0; j<sz; ++j) {
        if (lst[j]->getState() == Package::StDelete) continue;

        rpmds provDs = lst[j]->provides.getDepSet();

        if (rpmdsSearch(provDs,reqDs) >= 0 || 
                             rpmdsSearch(rpmDepSet,reqDs) >= 0) {
          found = true;
          break;
        }
      }

      if (!found) {
        Capability reqCap(rpmdsN(reqDs),rpmdsEVR(reqDs),rpmdsFlags(reqDs));
        reqCap.pkg = lst[i];

        reqLst.addUnique(reqCap);
      }
    }
  }
}

//---------------------------------------------------------------------------

void PackageList::buildConflictLst(CapabilityList& conflictLst) const
{
  for (int i=0; i<sz; ++i) {
    if (lst[i]->getState() == Package::StDelete) continue;

    rpmds cfltDs = lst[i]->conflicts.getDepSet();

    rpmdsInit(cfltDs);

    int idx = -1;

    while ((idx = rpmdsNext(cfltDs)) >= 0) {
      for (int j=0; j<sz; ++j) {
        if (lst[j]->getState() == Package::StDelete) continue;

        rpmds provDs = lst[j]->provides.getDepSet();

        if (rpmdsSearch(provDs,cfltDs) >= 0 || 
                             rpmdsSearch(rpmDepSet,cfltDs) >= 0) {
          Capability cfltCap(rpmdsN(cfltDs),rpmdsEVR(cfltDs),rpmdsFlags(cfltDs));
          cfltCap.pkg = lst[i];

          conflictLst.addUnique(cfltCap);
        }
      }
    }
  }
}

#endif

//---------------------------------------------------------------------------

void PackageList::prepUpdatePackages(const PackageList& newPkgs,
                                     CapabilityList& reqLst,
                                     CapabilityList& conflictLst)
{
  reqLst.clear();
  conflictLst.clear();

#ifdef _WIN32
  reqLst.setType(0);
  conflictLst.setType(0);
#else
  reqLst.setType(RPMTAG_REQUIRENAME);
  conflictLst.setType(RPMTAG_CONFLICTNAME);
#endif

  for (int i=0; i<newPkgs.size(); ++i) {
    const Package& newPkg = newPkgs[i];

    int oldIdx = find(newPkg.getName());
    if (oldIdx >= 0) remove(oldIdx);

    Package& pkg = add(newPkg);
    pkg.st = Package::StNew;

    for (int j=0; j<pkg.obsoletes.size(); ++j) {
      int obsIdx = find(pkg.obsoletes[j].getName());
      if (obsIdx >= 0) lst[obsIdx]->st = Package::StDelete;
    }
  }

  buildReqLst(reqLst);
  buildConflictLst(conflictLst);
}

//---------------------------------------------------------------------------

void PackageList::prepErasePackages(int *idxLst, int idxLstSz,
                                                    CapabilityList& reqLst)
{
  reqLst.clear();

#ifdef _WIN32
  reqLst.setType(0);
#else
  reqLst.setType(RPMTAG_REQUIRENAME);
#endif

  if (!idxLst) throw NullPointerException("PackageList::erasePackage");

  for (int i=0; i<idxLstSz; ++i) {
    if (idxLst[i] < 0 || idxLst[i] >= sz)
      throw IndexOutOfBoundsException("PackageList::erasePackage");

    lst[idxLst[i]]->st = Package::StDelete;
  }

  buildReqLst(reqLst);
}

//---------------------------------------------------------------------------

void PackageList::removeInstalledPackages()
{
  for (int i=sz-1; i>=0; --i) {
    if (lst[i]->getState() != Package::StInstalled) continue;

    remove(i);
  }
}

//---------------------------------------------------------------------------
// Persistence section:

static const char fldLst[] = "List";

//---------------------------------------------------------------------------

void PackageList::definePersistentFields(Ino::PersistentWriter& po) const
{
  po.addObjectArrayField(fldLst,(Package **)NULL);
}

//---------------------------------------------------------------------------

PackageList::PackageList(Ino::PersistentReader& pi)
: Persistable(),
  lst((Package **)pi.readObjArray(fldLst,NULL)),
  sz(pi.readArraySize(fldLst,0)),cap(sz),
  source(NULL)
{
  if (!lst) sz = cap = 0;

#ifndef _WIN32
  if (mustInit) rpmReadConfigFiles(NULL,NULL);
#endif

  mustInit = false;
}

//---------------------------------------------------------------------------

void PackageList::writePersistentObject(Ino::PersistentWriter& po) const
{
  po.writeArray(fldLst,lst,sz);
}

} //namespace

//---------------------------------------------------------------------------
