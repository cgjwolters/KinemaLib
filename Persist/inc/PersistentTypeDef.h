//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV June 2005 --------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSIST_TYPEDEF_INC
#define PERSIST_TYPEDEF_INC

#include <typeinfo>

namespace AW
{

//---------------------------------------------------------------------------

class PersistentReader;
class PersistentWriter;

class Persistable
{
public:
  virtual void definePersistentFields(PersistentWriter& po) = 0;

  virtual void readPersistentObject(PersistentReader& pi) = 0;

  virtual void writePersistentObject(PersistentWriter& po) = 0;
};

//---------------------------------------------------------------------------

class MainPersistable : public Persistable
{
public:
  virtual void readPersistentComplete(PersistentReader pi) = 0;
};

//---------------------------------------------------------------------------

class PersistentBaseType
{
  PersistentBaseType *nameNext;
  PersistentBaseType *infoNext;

  PersistentBaseType(const PersistentBaseType& cp);             // No Copying
  PersistentBaseType& operator=(const PersistentBaseType& src); // No Assignment

  static bool legalName(const char *name);

protected:
  virtual void construct(Persistable* p, PersistentReader& is) const = 0;

public:
  PersistentBaseType(const char *tpName, const type_info& inf, int tpSz);
  virtual ~PersistentBaseType();

  const type_info&  info;
  const int         sz;
  const char* const name;

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------
// ------- New operator for Persistables ------------------------------------
//---------------------------------------------------------------------------

void *operator new(size_t sz, Persistable* tp);

//---------------------------------------------------------------------------

template <class T> class PersistentType : public PersistentBaseType
{
  PersistentType(const PersistentType& cp);             // No Copying
  PersistentType& operator=(const PersistentType& src); // No Assignment

protected:

  virtual void construct(Persistable* p, PersistentReader& is) const
  { if (!p) throw NullPointerException("PersistentType::construct");
    new (p) T(is);
  }

public:
  PersistentType(const char *tpName)
    : PersistentBaseType(tpName,typeid(T),sizeof(T)) {}

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------

template <class T> class PersistentAbstractType : public PersistentBaseType
{
  PersistentAbstractType(const PersistentAbstractType& cp);             // No Copying
  PersistentAbstractType& operator=(const PersistentAbstractType& src); // No Assignment

public:
  PersistentAbstractType(const char *tpName)
    : PersistentBaseType(tpName,typeid(T),sizeof(T)) {}

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------

class PersistentTypeDef
{
public:
  const long famMagic;
  const long magic;
  const char major;
  const char minor;

private:
  enum { IncCap = 128 };

  PersistentBaseType **nameLst;
  PersistentBaseType **infoLst;

  int sz,cap;

  mutable bool completed;

  PersistentBaseType *makeChain();
  void incCapacity();

  PersistentTypeDef(const PersistentTypeDef& cp);
  PersistentTypeDef& operator=(const PersistentTypeDef& src);

protected:
  PersistentTypeDef(long famMagicNr, long magicNr, char majorNr, char minorNr);
  virtual ~PersistentTypeDef();

  void add(PersistentBaseType *tp);

public:

  const PersistentBaseType *get(const char *name) const;
  const PersistentBaseType *get(const type_info& inf) const;
};

} // namespace AW

//---------------------------------------------------------------------------
#endif
