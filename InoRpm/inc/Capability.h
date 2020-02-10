//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Capability -------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INORPM_CAPABILITY_INC
#define INORPM_CAPABILITY_INC

#include "PersistentIO.h"

namespace InoRpm
{

//---------------------------------------------------------------------------

class Package;

class Capability : public Ino::Persistable
{
  Package *pkg;

  char *name;
  char *version;
  unsigned int flags;

  Capability& operator=(const Capability& src); // No assignment

public:
  Capability(const char *capName, const char *capVer, int flg);
  Capability(const Capability& cp);
  virtual ~Capability();

  Package& getPackage() const { return *pkg; }
  const char *getName() const { return name; }
  const char *getVersion() const { return version; }

  //bool getVersion(char& major, char& minor, char& release,
  //                                                  char& pkgRelease);

  unsigned int getFlags() const { return flags; }

  bool getNvr(char *buf, int bufSz) const;
  bool getFullString(char *buf, int bufSz) const;

  Capability(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;

  friend class Package;
  friend class PackageList;
};

} // namespace

//---------------------------------------------------------------------------
#endif
