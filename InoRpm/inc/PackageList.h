//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Package List -----------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INORPM_PACKAGE_LIST_INC
#define INORPM_PACKAGE_LIST_INC

#include "PersistentIO.h"

namespace InoRpm
{

class Package;
class Capability;
class CapabilityList;

//---------------------------------------------------------------------------

class PackageList : public Ino::Persistable
{
public:
  enum RpmRc { RpmOk, RpmNotFound, RpmFail, RpmNotTrusted, RpmNoKey };

private:
  enum { CapInc = 20 };

  static bool mustInit;

  Package **lst;
  int sz,cap;

  char *source;

  void buildFromRpmDb(bool genFiles);
  RpmRc addFromRpmFile(const char *filePath);

  void incCapacity();

  void remove(int idx);

  void buildReqLst(CapabilityList& reqLst) const;
  void buildConflictLst(CapabilityList& conflictLst) const;

public:
  PackageList(const char *src = NULL);
  PackageList(const PackageList& cp);
  virtual ~PackageList();

  PackageList& operator=(const PackageList& src);

  int size() const { return sz; }
  void clear();

  const char *getSource() const { return source; }

  void setInstalledVersions(const PackageList& installedLst);

  int find(const char *name) const;
  const Package& get(int idx) const;
  Package& get(int idx);

  const Package& operator[](int idx) const;
  Package& operator[](int idx);

  Package& add(const Package& pkg);

  RpmRc addRpmFile(const char *file);
  bool buildFromInstalled(bool genFiles = false);

  void prepUpdatePackages(const PackageList& newPkgs, CapabilityList& reqLst,
                                            CapabilityList& conflictLst);
  void prepErasePackages(int *idxLst, int idxLstSz, CapabilityList& reqLst);

  void removeInstalledPackages();

  PackageList(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;
};

} //namespace

//---------------------------------------------------------------------------
#endif
