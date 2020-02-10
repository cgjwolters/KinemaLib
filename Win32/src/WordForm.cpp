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

#include "StdAfx.h"
#include "WordForm.h"
#include "FieldList.h"

namespace AW
{

using namespace std;
using namespace Word;

//---------------------------------------------------------------------------

struct WordHlp
{
  _ApplicationPtr appPtr;
  _DocumentPtr    docPtr;

  WordHlp() : appPtr(), docPtr() {}
  ~WordHlp() { docPtr = NULL; appPtr = NULL; }
};

//---------------------------------------------------------------------------

WordForm::WordForm()
: fldList(*new FieldList()), result(Ok), hlp(*new WordHlp)
{
}

//---------------------------------------------------------------------------

WordForm::~WordForm()
{
  fldList.clear();

  hlp.docPtr = NULL;

  if (hlp.appPtr) {
     bool mustQuit = true;

     DocumentsPtr docsPtr;
     hlp.appPtr->get_Documents(&docsPtr);

     if (docsPtr != NULL) { // How many docs open?
       long count = 0;
       if (SUCCEEDED(docsPtr->get_Count(&count))&& count > 0) mustQuit = false;
     }

     docsPtr = NULL;

     _variant_t vt_false(false);
     if (mustQuit) hlp.appPtr->Quit(&vt_false);
     else { // make visible
       _variant_t vt_true(true);
       hlp.appPtr->put_Visible(vt_true);
       hlp.appPtr->Activate();
     }
  }

  hlp.appPtr = NULL;

  delete &hlp;
  delete &fldList;
}

//---------------------------------------------------------------------------

bool WordForm::newDocument(const wchar_t *templatePath)
{
  fldList.clear();

  if (!templatePath || wcslen(templatePath) < 1) {
    result = TemplateNotFound;
    return false;
  }

  bool wasRunning = hlp.appPtr != NULL;

  HRESULT ret = S_OK;

  if (hlp.appPtr == NULL) {
    // Try to find an already running MSWord instance first

    if (!SUCCEEDED(hlp.appPtr.GetActiveObject(L"Word.Application")) ||
                                                      hlp.appPtr == NULL) {
      // Ok, attempt to start a new one
      if (!SUCCEEDED(hlp.appPtr.CreateInstance(L"Word.Application")) ||
                                                      hlp.appPtr == NULL) {
        result = WordNotFound;
        return false;
      }
    }
  }

  _variant_t vt_false(false);
  _variant_t vt_true(true);
//  hlp.appPtr->put_Visible(vt_false);

  DocumentsPtr docsPtr;
  if (!SUCCEEDED(hlp.appPtr->get_Documents(&docsPtr)) || docsPtr == NULL) {
    if (!wasRunning) {
      hlp.appPtr->Quit(&vt_false);
      hlp.appPtr = NULL;
    }

    result = WordError;
    return false;
  }

  _variant_t tmplPath(templatePath);
  ret = docsPtr->Add(&tmplPath,&vtMissing,&vtMissing,&vt_true,&hlp.docPtr);

  if (ret != S_OK || hlp.docPtr == NULL) {
    if (!wasRunning) {
      docsPtr = NULL;

      hlp.appPtr->Quit(&vt_false);
      hlp.appPtr = NULL;
    }

    result = TemplateNotFound;
    return false;
  }

  MailMergePtr mailMergePtr;
  if (SUCCEEDED(hlp.docPtr->get_MailMerge(&mailMergePtr)) && mailMergePtr != NULL) {
    mailMergePtr->put_ViewMailMergeFieldCodes(VARIANT_FALSE);
  }

  result = Ok;

  return true;
}

//---------------------------------------------------------------------------

bool WordForm::cancelDocument()
{
  fldList.clear();

  if (hlp.docPtr == NULL) {
    result = NoCurrentDoc;
    return false;
  }

  _variant_t vt_false(false);
  hlp.docPtr->Close(&vt_false);

  hlp.docPtr = NULL;

  result = Ok;

  return true;
}

//---------------------------------------------------------------------------

bool WordForm::showDocument()
{
   if (hlp.docPtr == NULL) {
    result = NoCurrentDoc;
    return false;
  }

  if (hlp.appPtr == NULL) return false;

  _variant_t vt_true(true);
  hlp.appPtr->put_Visible(vt_true);
  hlp.appPtr->Activate();
  hlp.docPtr->Activate();

  hlp.docPtr = NULL;

  result = Ok;

  return true;
}

//---------------------------------------------------------------------------

bool WordForm::saveDocumentAs(const wchar_t *docPath)
{
  if (!docPath || wcslen(docPath) < 1){
    result = IllegalArgument;
    return false;
  }

  if (hlp.docPtr == NULL) {
    result = NoCurrentDoc;
    return false;
  }

  DeleteFile(docPath);

  _variant_t varDocPath(docPath);
  HRESULT ret = hlp.docPtr->SaveAs(&varDocPath);

  if (ret == S_OK) {
    result = Ok;
    return true;
  }
  else {
    result = NotSaved;

    return false;
  }
}

//---------------------------------------------------------------------------

bool WordForm::setField(const wchar_t *fldName, int seqNr,
                                                const wchar_t *fldVal)
{
  if (hlp.docPtr == NULL) {
    result = NoCurrentDoc;
    return false;
  }

  if (!fldName || wcslen(fldName) < 1 || !fldVal || seqNr < 0) {
    result = IllegalArgument;
    return false;
  }

  if (!fldList.isValid()) {
    if (!fldList.build(hlp.docPtr)) {
      result = WordError;
      return false;
    }
  }

  FieldPtr fldPtr;
  if (!fldList.find(fldName,seqNr,fldPtr)) {
    result = FieldNotFound;
    return false;
  }

  RangePtr rangePtr;
  if (!SUCCEEDED(fldPtr->get_Result(&rangePtr)) || rangePtr == NULL) {
    result = WordError;
    return false;
  }

  _FontPtr fontPtr;
  rangePtr->get_Font(&fontPtr);

  _bstr_t val(fldVal);
  HRESULT ret = rangePtr->put_Text(val);

  if (ret != S_OK && ret != RPC_E_SERVERFAULT) {
    result = WordError;
    return false;
  }

  if (fontPtr != NULL) rangePtr->put_Font(fontPtr);

  return true;
}

//---------------------------------------------------------------------------

bool WordForm::optSetField(const wchar_t *fldName, int seqNr,
                                        const wchar_t *fldVal, bool *found)
{
  if (setField(fldName,seqNr,fldVal)) {
    if (found != NULL) *found = true;
    return true;
  }
  else if (result == FieldNotFound) {
    if (found != NULL) *found = false;
    return true;
  }

  if (found != NULL) *found = false;
  return false;
}

//---------------------------------------------------------------------------

int WordForm::getTableCount()
{
  if (hlp.docPtr == NULL) {
    result = NoCurrentDoc;
    return false;
  }

  TablesPtr tablesPtr;
  if (!SUCCEEDED(hlp.docPtr->get_Tables(&tablesPtr))) {
    result = WordError;
    return -1;
  }

  long tableCount = 0;
  if (!SUCCEEDED(tablesPtr->get_Count(&tableCount))) {
    result = WordError;
    return -1;
  }

  return tableCount;
}

//---------------------------------------------------------------------------

static bool getTable(_DocumentPtr& docPtr, int tableNr, TablePtr& tablePtr,
                                                   WordForm::Result& result)
{
  if (docPtr == NULL) {
    result = WordForm::NoCurrentDoc;
    return false;
  }

  TablesPtr tablesPtr;
  if (!SUCCEEDED(docPtr->get_Tables(&tablesPtr))) {
    result = WordForm::WordError;
    return false;
  }

  long tableCount = 0;
  if (!SUCCEEDED(tablesPtr->get_Count(&tableCount))) {
    result = WordForm::WordError;
    return false;
  }

  if (tableNr < 0 || tableNr >= tableCount) {
    result = WordForm::IllegalArgument;
    return false;
  }

  if (!SUCCEEDED(tablesPtr->Item(tableNr+1,&tablePtr)) || tablePtr == NULL) {
    result = WordForm::WordError;
    return false;
  }

  result = WordForm::Ok;
  return true;
}

//---------------------------------------------------------------------------

int WordForm::getTableRows(int tableNr)
{
  TablePtr tablePtr;

  if (!getTable(hlp.docPtr,tableNr,tablePtr,result)) return -1;

  RowsPtr rowsPtr;
  if (!SUCCEEDED(tablePtr->get_Rows(&rowsPtr)) || rowsPtr == NULL) {
    result = WordError;
    return -1;
  }

  long rowCount = 0;
  if (!SUCCEEDED(rowsPtr->get_Count(&rowCount))) {
    result = WordError;
    return -1;
  }

  return rowCount;
}

//---------------------------------------------------------------------------

bool WordForm::insertTableRow(int tableNr, int rowIdx, int copyRowIdx)
{
  fldList.clear();

  TablePtr tablePtr;

  if (!getTable(hlp.docPtr,tableNr,tablePtr,result)) return false;

  RowsPtr rowsPtr;
  if (!SUCCEEDED(tablePtr->get_Rows(&rowsPtr)) || rowsPtr == NULL) {
    result = WordError;
    return false;
  }

  long rowCount = 0;
  if (!SUCCEEDED(rowsPtr->get_Count(&rowCount))) {
    result = WordError;
    return false;
  }

  if (rowIdx < 0 || rowIdx > rowCount || copyRowIdx >= rowCount) {
    result = IllegalArgument;
    return false;
  }

  if (copyRowIdx >= 0) { // make a copy of a row
    RowPtr rowPtr;
    if (!SUCCEEDED(rowsPtr->Item(copyRowIdx+1,&rowPtr)) || rowPtr == NULL)
    {
      result = WordError;
      return false;
    }

    RangePtr rangePtr;
    if (!SUCCEEDED(rowPtr->get_Range(&rangePtr)) ||
                    rangePtr == NULL || !SUCCEEDED(rangePtr->Copy())) {
      result = WordError;
      return false;
    }

    rowPtr = NULL;
    rangePtr = NULL;

    _variant_t dir(wdCollapseStart);
    if (rowIdx >= rowCount) {
      dir = wdCollapseEnd;
      rowIdx = rowCount-1;
    }

    if (!SUCCEEDED(rowsPtr->Item(rowIdx+1,&rowPtr)) || rowPtr == NULL) {
      result = WordError;
      return false;
    }

    if (!SUCCEEDED(rowPtr->get_Range(&rangePtr)) || rangePtr == NULL) {
      result = WordError;
      return false;
    }

     if (!SUCCEEDED(rangePtr->Collapse(&dir))) {
      result = WordError;
      return false;
    }

    HRESULT ret = rangePtr->Paste();
    if (ret != S_OK && ret != RPC_E_SERVERFAULT) {
      result = WordError;
      return false;
    }
  }
  else { // Just add/insert a new row
    RowPtr rowPtr;

    if (rowIdx < rowCount) {
      if (!SUCCEEDED(rowsPtr->Item(rowIdx+1,&rowPtr))) {
        result = WordError;
        return false;
      }
    }

    _variant_t vt_row(rowPtr);
    if (!SUCCEEDED(rowsPtr->Add(&vt_row,&rowPtr))) {
      result = WordError;
      return false;
    }
  }

  result = Ok;

  return true;
}

//---------------------------------------------------------------------------

bool WordForm::deleteTableRow(int tableNr, int rowIdx)
{
  fldList.clear();

  TablePtr tablePtr;

  if (!getTable(hlp.docPtr,tableNr,tablePtr,result)) return false;

  RowsPtr rowsPtr;
  if (!SUCCEEDED(tablePtr->get_Rows(&rowsPtr)) || rowsPtr == NULL) {
    result = WordError;
    return false;
  }

  long rowCount = 0;
  if (!SUCCEEDED(rowsPtr->get_Count(&rowCount))) {
    result = WordError;
    return false;
  }

  if (rowIdx < 0 || rowIdx >= rowCount) {
    result = IllegalArgument;
    return false;
  }

  RowPtr rowPtr;
  if (!SUCCEEDED(rowsPtr->Item(rowIdx+1,&rowPtr)) || rowPtr == NULL) {
    result = WordError;
    return false;
  }

  if (!SUCCEEDED(rowPtr->Delete())) {
    result = WordError;
    return false;
  }
  
  return true;
}

} // namespace AW

//---------------------------------------------------------------------------
