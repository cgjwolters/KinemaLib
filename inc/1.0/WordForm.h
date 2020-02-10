//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Use to fill in a MS Word template doc -----------------------------
//---------------------------------------------------------------------------
//------- Copyright 2005 Prodim International BV ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters Aug 2005 -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef AW_WORD_FORM_INC
#define AW_WORD_FORM_INC

#include <wchar.h>

//---------------------------------------------------------------------------

namespace AW
{

struct WordHlp;

class FieldList;

class WordForm
{
  FieldList& fldList;

public:
  enum Result { Ok, WordNotFound, TemplateNotFound, NoCurrentDoc,
                FieldNotFound, NotSaved, IllegalArgument, WordError  };

private:
  Result result;
  WordHlp& hlp;

  WordForm(const WordForm& cp);             // No Copying
  WordForm& operator=(const WordForm& src); // No Assignment

public:
  WordForm();
  ~WordForm();

  Result getResult() const { return result; }

  bool newDocument(const wchar_t *templatePath);
  bool cancelDocument();
  bool showDocument();
  bool saveDocumentAs(const wchar_t *docPath);

  int getTableCount();
  int getTableRows(int tableNr);
  bool insertTableRow(int tableNr, int rowIdx, int copyRowIdx);
  bool deleteTableRow(int tableNr, int rowIdx);

  bool setField(const wchar_t *fldName, int seqNr,const wchar_t *fldVal);
  bool optSetField(const wchar_t *fldName, int seqNr,
                                   const wchar_t *fldVal, bool *found=0);
};

} // namespace AW

//---------------------------------------------------------------------------
#endif