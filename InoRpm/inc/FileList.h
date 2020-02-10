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

#ifndef INORPM_FILE_LIST_INC
#define INORPM_FILE_LIST_INC

#include "PersistentIO.h"

namespace InoRpm
{
//---------------------------------------------------------------------------

class FileString;

//---------------------------------------------------------------------------

class FileString : public Ino::Persistable
{
  wchar_t *str;

  FileString& operator=(const FileString& src); // No Assignment

  FileString(const wchar_t *s);

public:
  FileString(const FileString& cp);
  ~FileString();

  const wchar_t *getStr() const { return str; }

  // Persistence
  FileString(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;

  friend class FileList;
};

//---------------------------------------------------------------------------

class FileList : public Ino::Persistable
{
  enum { CapInc = 6 };

  FileString **nameLst;
  int nameSz,nameCap;

  void incCapacity();

public:
  FileList();
  FileList(const FileList& cp);
  virtual ~FileList();

  FileList& operator=(const FileList& src);

  int size() const { return nameSz; }
  void clear();

  const wchar_t *get(int idx) const;
  const wchar_t *get(int idx);

  const wchar_t *operator[](int idx) const;
  const wchar_t *operator[](int idx);

  const wchar_t *add(const char *newFile);
  const wchar_t *add(const wchar_t *newFile);

  // Persistence
  FileList(Ino::PersistentReader& pi);
  virtual void definePersistentFields(Ino::PersistentWriter& po) const;
  virtual void writePersistentObject(Ino::PersistentWriter& po) const;
};

} // namespace

//---------------------------------------------------------------------------
#endif
