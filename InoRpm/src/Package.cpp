//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Package ----------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Package.h"

#include "Capability.h"
#include "CapabilityList.h"
#include "FileList.h"

#include <string.h>

//---------------------------------------------------------------------------

namespace InoRpm
{
using namespace Ino;

//---------------------------------------------------------------------------

#ifndef _WIN32

static Capability *genNameVersion(Header hdr)
{
  const char *name, *ver, *rel;

  headerNVR(hdr,&name,&ver,&rel);

  char verRel[512] = "";
  strcpy(verRel,ver);
  strcat(verRel,"-");
  strcat(verRel,rel);

  return new Capability(name,verRel,0);
}

//---------------------------------------------------------------------------

static CapabilityList *genProvides(Header hdr)
{
  CapabilityList *lst = new CapabilityList(RPMTAG_PROVIDENAME);

  struct rpmtd_s names, versions, flags;

  headerGet(hdr,RPMTAG_PROVIDENAME,&names,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_PROVIDEVERSION,&versions,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_PROVIDEFLAGS,&flags,HEADERGET_MINMEM);

  for (unsigned int i=0; i<names.count; ++i) {
    const char *name    = ((const char **)names.data)[i];
    const char *version = ((const char **)versions.data)[i];
    rpm_flag_t fl       = ((rpm_flag_t *)flags.data)[i];

    lst->add(Capability(name,version,fl));
  }

  rpmtdFreeData(&names);
  rpmtdFreeData(&versions);

  return lst;
}

//---------------------------------------------------------------------------

static CapabilityList *genRequires(Header hdr)
{
  CapabilityList *lst = new CapabilityList(RPMTAG_REQUIRENAME);

  struct rpmtd_s names, versions, flags;

  headerGet(hdr,RPMTAG_REQUIRENAME,&names,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_REQUIREVERSION,&versions,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_REQUIREFLAGS,&flags,HEADERGET_MINMEM);
    
  for (unsigned int i=0; i<names.count; ++i) {
    const char *name    = ((const char **)names.data)[i];
    const char *version = ((const char **)versions.data)[i];
    rpm_flag_t fl       = ((rpm_flag_t *)flags.data)[i];
   
    lst->add(Capability(name,version,fl));
  }

  rpmtdFreeData(&names);
  rpmtdFreeData(&versions);
  rpmtdFreeData(&flags);

  return lst;
}

//---------------------------------------------------------------------------

static CapabilityList *genConflicts(Header hdr)
{
  CapabilityList *lst = new CapabilityList(RPMTAG_CONFLICTNAME);

  struct rpmtd_s names, versions, flags;

  headerGet(hdr,RPMTAG_CONFLICTNAME,&names,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_CONFLICTVERSION,&versions,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_CONFLICTFLAGS,&flags,HEADERGET_MINMEM);
    
  for (unsigned int i=0; i<names.count; ++i) {
    const char *name    = ((const char **)names.data)[i];
    const char *version = ((const char **)versions.data)[i];
    rpm_flag_t fl       = ((rpm_flag_t *)flags.data)[i];

    lst->add(Capability(name,version,fl));
  }

  rpmtdFreeData(&names);
  rpmtdFreeData(&versions);
  rpmtdFreeData(&flags);

  return lst;
}

//---------------------------------------------------------------------------

static CapabilityList *genObsoletes(Header hdr)
{
  CapabilityList *lst = new CapabilityList(RPMTAG_OBSOLETENAME);

  struct rpmtd_s names, versions, flags;

  headerGet(hdr,RPMTAG_OBSOLETENAME,&names,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_OBSOLETEVERSION,&versions,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_OBSOLETEFLAGS,&flags,HEADERGET_MINMEM);
    
  for (unsigned int i=0; i<names.count; ++i) {
    const char *name    = ((const char **)names.data)[i];
    const char *version = ((const char **)versions.data)[i];
    rpm_flag_t fl       = ((rpm_flag_t *)flags.data)[i];

    lst->add(Capability(name,version,fl));
  }

  rpmtdFreeData(&names);
  rpmtdFreeData(&versions);
  rpmtdFreeData(&flags);

  return lst;
}

//---------------------------------------------------------------------------

static CapabilityList *genPreReqs(Header hdr)
{
  CapabilityList *lst = new CapabilityList(RPMTAG_REQUIRENAME);

  struct rpmtd_s names, versions, flags;

  headerGet(hdr,RPMTAG_REQUIRENAME,&names,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_REQUIREVERSION,&versions,HEADERGET_MINMEM);
  headerGet(hdr,RPMTAG_REQUIREFLAGS,&flags,HEADERGET_MINMEM);
    
  for (unsigned int i=0; i<names.count; ++i) {
    const char *name    = ((const char **)names.data)[i];
    const char *version = ((const char **)versions.data)[i];
    rpm_flag_t fl       = ((rpm_flag_t *)flags.data)[i];

    if (isLegacyPreReq(fl) || isInstallPreReq(fl)) {
      lst->add(Capability(name,version,fl));
    }
  }

  rpmtdFreeData(&names);
  rpmtdFreeData(&versions);
  rpmtdFreeData(&flags);

  return lst;
}

//---------------------------------------------------------------------------

static char *genGroup(Header hdr)
{
  struct rpmtd_s group;

  headerGet(hdr,RPMTAG_GROUP,&group,HEADERGET_MINMEM);

  char *grp = dupStr((const char *)group.data);

  rpmtdFreeData(&group);

  return grp;
}

//---------------------------------------------------------------------------

static FileList *genFileList(Header hdr)
{
  FileList *lst = new FileList();

  struct rpmtd_s fileNames;

  headerGet(hdr,RPMTAG_FILENAMES,&fileNames,HEADERGET_EXT);
    
  for (unsigned int i=0; i<fileNames.count; ++i) {
    const char *fileName = ((const char **)fileNames.data)[i];

    lst->add(fileName);
  }

  rpmtdFreeData(&fileNames);

  return lst;
}

//---------------------------------------------------------------------------

Package::Package(Header hdr, bool genFiles, const char *file)
: Persistable(),
  st(StNew), group(genGroup(hdr)),
  fileName(dupStr(file)),
  installedVer(NULL),
  nameVersionRel(*genNameVersion(hdr)),
  provides(*genProvides(hdr)),
  requires(*genRequires(hdr)),
  conflicts(*genConflicts(hdr)),
  obsoletes(*genObsoletes(hdr)),
  preReqs(*genPreReqs(hdr)),
  files(genFiles ? *genFileList(hdr) : *new FileList())
{
  nameVersionRel.pkg = this;
}

#else

//---------------------------------------------------------------------------

Package::Package(Header hdr, bool genFiles, const char *file)
: Persistable(),
  st(StNew), group(NULL), fileName(dupStr(file)),
  nameVersionRel(*new Capability(NULL,NULL,0)),
  installedVer(NULL),
  provides(*new CapabilityList(0)),
  requires(*new CapabilityList(0)),
  conflicts(*new CapabilityList(0)),
  obsoletes(*new CapabilityList(0)),
  preReqs(*new CapabilityList(0)),
  files(*new FileList())
{
  nameVersionRel.pkg = this;
}

#endif

//---------------------------------------------------------------------------

Package::Package(const Package& cp)
: Persistable(cp),
  st(cp.st), group(dupStr(cp.group)), fileName(dupStr(cp.fileName)),
  installedVer(cp.installedVer ? new Capability(*cp.installedVer) : NULL),
  nameVersionRel(* new Capability(cp.nameVersionRel)),
  provides(* new CapabilityList(cp.provides)),
  requires(* new CapabilityList(cp.requires)),
  conflicts(* new CapabilityList(cp.conflicts)),
  obsoletes(* new CapabilityList(cp.obsoletes)),
  preReqs(* new CapabilityList(cp.preReqs)),
  files(*new FileList(cp.files))
{
  nameVersionRel.pkg = this;

  for (int i=0; i<provides.size(); ++i)  provides[i].pkg = this;
  for (int i=0; i<requires.size(); ++i)  requires[i].pkg = this;
  for (int i=0; i<conflicts.size(); ++i) conflicts[i].pkg = this;
  for (int i=0; i<obsoletes.size(); ++i) obsoletes[i].pkg = this;
  for (int i=0; i<preReqs.size(); ++i)   preReqs[i].pkg = this;
}

//---------------------------------------------------------------------------

Package::~Package()
{
  delete[] group;
  delete[] fileName;
  delete installedVer;
  delete &nameVersionRel;
  delete &provides;
  delete &requires;
  delete &conflicts;
  delete &obsoletes;
  delete &preReqs;
  delete &files;
}

//---------------------------------------------------------------------------

const char *Package::getName() const
{
  return nameVersionRel.getName();
}

//---------------------------------------------------------------------------

void Package::setInstalledVer(const Capability *instVer)
{
  delete installedVer;
  installedVer = NULL;

  if (instVer) installedVer = new Capability(*instVer);
}

//---------------------------------------------------------------------------

int Package::compare(const Package& other) const
{
  int cmp = compareStr(nameVersionRel.getName(),
                       other.nameVersionRel.getName());
  if (cmp) return cmp;

#ifdef _WIN32
  return -1;
#else
  return rpmvercmp(nameVersionRel.getVersion(),
                   other.nameVersionRel.getVersion());
#endif
}

//---------------------------------------------------------------------------
// Persistence section:

static const char fldGroup[]          = "Group";
static const char fldFileName[]       = "FileName";
static const char fldNameVersionRel[] = "NameVersionRel";
static const char fldProvides[]       = "Provides";
static const char fldRequires[]       = "Requires";
static const char fldConflicts[]      = "Conflicts";
static const char fldObsoletes[]      = "Obsoletes";
static const char fldPreReqs[]        = "PreReqs";
static const char fldFiles[]          = "Files";

//---------------------------------------------------------------------------

void Package::definePersistentFields(PersistentWriter& po) const
{
  po.addArrayField(fldGroup,(char *)NULL);
  po.addArrayField(fldFileName,(char *)NULL);
  po.addField(fldNameVersionRel,typeid(Capability));
  po.addField(fldProvides,typeid(CapabilityList));
  po.addField(fldRequires,typeid(CapabilityList));
  po.addField(fldConflicts,typeid(CapabilityList));
  po.addField(fldObsoletes,typeid(CapabilityList));
  po.addField(fldPreReqs,typeid(CapabilityList));
  po.addField(fldFiles,typeid(FileList));
}

//---------------------------------------------------------------------------

Package::Package(PersistentReader& pi)
: Persistable(), st(StNew),
  group((char *)pi.readValArray(fldGroup,NULL)),
  fileName((char *)pi.readValArray(fldFileName,NULL)),
  installedVer(NULL),
  nameVersionRel(*(Capability *)pi.readObject(fldNameVersionRel)),
  provides(*(CapabilityList *)pi.readObject(fldProvides)),
  requires(*(CapabilityList *)pi.readObject(fldRequires)),
  conflicts(*(CapabilityList *)pi.readObject(fldConflicts)),
  obsoletes(*(CapabilityList *)pi.readObject(fldObsoletes)),
  preReqs(*(CapabilityList *)pi.readObject(fldPreReqs)),
  files(*(FileList *)pi.readObject(fldFiles,new FileList())) // Mem leak!
{
}

//---------------------------------------------------------------------------

void Package::writePersistentObject(PersistentWriter& po) const
{
  po.writeArray(fldGroup,group,strlen(group)+1); // Include the trailng zero!!
  po.writeArray(fldFileName,fileName,strlen(fileName)+1);
  po.writeObject(fldNameVersionRel,&nameVersionRel);
  po.writeObject(fldProvides,&provides);
  po.writeObject(fldRequires,&requires);
  po.writeObject(fldConflicts,&conflicts);
  po.writeObject(fldObsoletes,&obsoletes);
  po.writeObject(fldPreReqs,&preReqs);
  po.writeObject(fldFiles,&files);
}

} // namespace

//---------------------------------------------------------------------------
