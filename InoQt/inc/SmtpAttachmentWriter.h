//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Utility class for Smtp mail ---------------------------------------
//------- Writes a file attachment to Smtp ----------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim International BV Aug 2007 ------------------------
//---------------------------------------------------------------------------
//------- C. Wolters  -------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef SMTP_ATTACHMENT_WRITER_INC
#define SMTP_ATTACHMENT_WRITER_INC

#include "Writer.h"

//---------------------------------------------------------------------------

namespace Ino {

class Base64Writer;
class Smtp;

//---------------------------------------------------------------------------

class SmtpAttachmentWriter : public Writer
{
  Base64Writer& b64Wrt;
  Smtp& smtp;
  char *file;
  bool headerWritten;
  bool closed;
  char *bndTxt;

  bool writeHeader();

  SmtpAttachmentWriter(const SmtpAttachmentWriter& cp);             // No Copying
  SmtpAttachmentWriter& operator=(const SmtpAttachmentWriter& src); // No Assigmment

public:
  SmtpAttachmentWriter(Smtp& mailer, const char *fileName, const char *boundary);
  virtual ~SmtpAttachmentWriter();

  Smtp& getSmtp() const { return smtp; }

  virtual bool isClosed() const { return closed; }
  virtual bool isAborted() const { return false; }
  virtual long getErrorCode() const { return 0; }
  virtual bool isBuffered() const  { return true; }

  virtual bool write(const char *buf, int sz);
  virtual bool flush() { return true; }

  virtual bool close();
};

} // namespace

//---------------------------------------------------------------------------
#endif
