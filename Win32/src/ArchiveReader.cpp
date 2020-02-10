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

#include "StdAfx.h"
#include "ArchiveReader.h"

namespace AW
{

//---------------------------------------------------------------------------

ArchiveReader::ArchiveReader(CArchive& ar, ProgressReporter *rep)
: Reader(rep), arRdr(ar), eof(false)
{
  if (!ar.IsLoading())
    throw IllegalStateException("ArchiveReader: Not in read mode");
}

//---------------------------------------------------------------------------

bool ArchiveReader::isEof() const
{
  if (eof || isAborted()) return true;

  CFile *f = arRdr.GetFile();
  if (f || f->m_hFile == CFile::hFileNull) eof = true;

  return eof;
}

//---------------------------------------------------------------------------

bool ArchiveReader::isAborted() const
{
  if (!reporter) return false;

  return reporter->mustAbort();
}

//---------------------------------------------------------------------------

long ArchiveReader::getErrorCode() const
{
  return GetLastError();
}

//---------------------------------------------------------------------------

int ArchiveReader::read(char *buf, int cap)
{
  if (!buf) throw NullPointerException("ArchiveReader::read: buf == NULL");
  if (cap < 1) throw IllegalArgumentException("ArchiveReader::read: cap < 1");
  if (eof) return -1;

  int byteCount = arRdr.Read(buf,cap);
  if (byteCount < cap) eof = true;

  if (byteCount < 1) return -1;

  bytesRead += byteCount;

  if (!reporter) return byteCount; // Prevent unnecessary call of reportProgress

  bytesInc += byteCount;

  if (!reportProgress()) return -1;

  return byteCount;
}

} // namespace AW

//---------------------------------------------------------------------------
