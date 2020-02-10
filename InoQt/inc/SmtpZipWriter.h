//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Utility class for Smtp mail ---------------------------------------
//------- Writes a zip file attachment to Smtp ------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim International BV Aug 2007 ------------------------
//---------------------------------------------------------------------------
//------- C. Wolters  -------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef SMTP_ZIP_WRITER_INC
#define SMTP_ZIP_WRITER_INC

#include "ZipOut.h"

namespace Ino {

class Base64Writer;
class Smtp;

//---------------------------------------------------------------------------

class SmtpZipWriter : public ZipOut
{
  Base64Writer& b64Wrt;
  Smtp& smtp;
  char *zipFile;
  bool headerWritten;
  char *bndTxt;

  bool writeHeader();

  SmtpZipWriter(const  SmtpZipWriter& cp);             // No Copying
  SmtpZipWriter& operator=(const  SmtpZipWriter& src); // No Assigment

public:
  SmtpZipWriter(Smtp& mailer, const char *zipName, const char *boundary);
  virtual ~SmtpZipWriter();

  Smtp& getSmtp() const { return smtp; }

  virtual bool isClosed() const;
  virtual bool isAborted() const { return false; }
  virtual long getErrorCode() const { return 0; }

  virtual bool addFile(const char *path, bool compressed = true);
  virtual bool write(const char *buf, int sz);

  virtual bool close();
};

} // namespace

//---------------------------------------------------------------------------
#endif
