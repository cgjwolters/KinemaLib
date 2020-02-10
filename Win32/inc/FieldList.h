//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Hash Map of Word MergeFields --------------------------------------
//---------------------------------------------------------------------------
//------- Copyright 2005 Prodim International BV ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters Aug 2005 -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef AW_WORD_FIELDLIST_INC
#define AW_WORD_FIELDLIST_INC

#include <wchar.h>

//---------------------------------------------------------------------------

namespace AW
{
using namespace Word;

class Field;

class FieldList
{
  enum { CapInc = 64 };

  Field **fldLst;
  int fldSz, fldCap;

  bool valid;

  void buildField(FieldPtr& fieldPtr);

  void incCapacity();
  void addField(const wchar_t *name, FieldPtr& fldPtr);

  FieldList(const FieldList& cp);             // No Copying
  FieldList& operator=(const FieldList& src); // No Assigment

public:
  FieldList();
  ~FieldList();

  bool isValid() const { return valid; }

  void clear();

  bool build(_DocumentPtr& docPtr);
  bool find(const wchar_t *name, int seqNr, FieldPtr& fld) const;
};

} // namespace AW

//---------------------------------------------------------------------------
#endif
