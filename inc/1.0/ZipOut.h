//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Creates a zip formatted output stream -----------------------------
//------- (Files cannot be NOT compressed (yet)) ----------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV SEP 2007 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOZIPOUT_INC
#define INOZIPOUT_INC

#include "Writer.h"

namespace Ino {

//---------------------------------------------------------------------------

class ZipOut : public Writer
{
  class ZipWriter& wrt;
  ByteArrayWriter& bWrt;
  CompressedWriter& cWrt;
  DataWriter& dWrt;
  class ZipFileList& fileLst;
  bool closed;

  bool writeLastFile();
  bool writeDirectory();

  ZipOut(const ZipOut& cp);             // No copying
  ZipOut& operator=(const ZipOut& src); // No assignment

public:
  ZipOut(Writer& writer);
  virtual ~ZipOut();

  Writer& getWriter();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;

  virtual bool isBuffered() const;

  virtual bool addFile(const char *path, bool compressed = true);
  virtual bool write(const char *buf, int sz);

  virtual bool flush() { return true; }

  virtual bool close();
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif


