//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Package ----------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INORPM_PACKAGE_INC
#define INORPM_PACKAGE_INC

#include "PersistentIO.h"

#ifdef _WIN32
typedef void *Header;
#else
#include <rpm/rpmlib.h>
#endif

namespace InoRpm
{

class Capability;
class CapabilityList;
class FileList;

//---------------------------------------------------------------------------

class Package : public Ino::Persistable
{
public:
  enum State { StInstalled, StDelete, StNew };

private:
  State st;
  char *group;
  char *fileName;
  Capability *installedVer;

  Package& operator=(const Package& src); // No assignment

  Package(Header hdr, bool genFiles, const char *file=NULL);

public:
  Capability& nameVersionRel;

  CapabilityList& provides;
  CapabilityList& requires;
  CapabilityList& conflicts;
  CapabilityList& obsoletes;
  CapabilityList& preReqs;
  FileList& files;

  Package(const Package& cp);
  virtual ~Package();

  State getState() const { return st; }

  const char *getName() const;
  const char *getGroup() const { return group; }
  const char *getFileName() const { return fileName; }

  int compare(const Package& other) const;

  const Capability *getInstalledVer() const { return installedVer; }
  void setInstalledVer(const Capability *instVer);

  Package(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;

  friend class PackageList;
};

} // namespace

//---------------------------------------------------------------------------
#endif


