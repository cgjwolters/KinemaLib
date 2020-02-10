//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Capability List --------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INORPM_CAPABILITYLIST_INC
#define INORPM_CAPABILITYLIST_INC

#include "PersistentIO.h"

#ifdef _WIN32
  typedef void *rpmds;
#else
#include "rpm/rpmlib.h"
#endif

namespace InoRpm
{

class Capability;

//---------------------------------------------------------------------------

class CapabilityList : public Ino::Persistable
{
  enum { CapInc = 6 };

  int type;

  Capability **lst;
  int sz,cap;

  mutable rpmds depSet;

  void incCapacity();

  CapabilityList& operator=(const CapabilityList& src); // No assignment

public:
  CapabilityList(int capType=0);
  CapabilityList(const CapabilityList& cp);
  ~CapabilityList();

  int getType() const { return type; }
  void setType(int capType);

  int size() const { return sz; }

  void clear();

  rpmds getDepSet() const;

  const Capability& operator[](int idx) const;
  Capability& operator[](int idx);

  const Capability *find(const char *name) const;
  const Capability& get(int idx) const;

  void add(const Capability& capability);
  void addUnique(const Capability& capability);
  void remove(int idx);

  CapabilityList(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;
};

} // namespace

//---------------------------------------------------------------------------
#endif
