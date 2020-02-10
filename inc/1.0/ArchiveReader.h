//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- CArchive Reader ---------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright 2005 AntWorks Engineering Software BV -------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters Aug 2005 -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// This is UNTESTED code, DO NOT USE!!!

#ifndef ARCHIVE_RDR_INC
#define ARCHIVE_RDR_INC

#include "Reader.h"

namespace AW
{

//---------------------------------------------------------------------------

class ArchiveReader : public Reader
{
  CArchive& arRdr;
  mutable bool eof;

public:
  ArchiveReader(CArchive& ar, ProgressReporter *rep=NULL);
  virtual ~ArchiveReader() {}

  virtual bool isEof() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;

  virtual bool isBuffered() const { return true; }

  virtual int read(char *buf, int cap);
};

} // namespace AW

//---------------------------------------------------------------------------
#endif
