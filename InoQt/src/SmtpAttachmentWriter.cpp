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

#include "SmtpAttachmentWriter.h"

#include "Smtp.h"
#include "Basics.h"
#include "Exceptions.h"

#include <stdio.h>

namespace Ino {

//---------------------------------------------------------------------------

SmtpAttachmentWriter::SmtpAttachmentWriter(Smtp& mailer,
                                           const char *fileName,
                                           const char *boundary)
: Writer(0), b64Wrt(*new Base64Writer(mailer)), smtp(mailer),
  file(dupStr(fileName)), headerWritten(false), closed(false),
  bndTxt(dupStr(boundary))
{
  if (!boundary)
    throw IllegalArgumentException("SmtpAttachmentWriter::SmtpAttachmentWriter");
}

//---------------------------------------------------------------------------

SmtpAttachmentWriter::~SmtpAttachmentWriter()
{
  close();

  delete[] bndTxt;
  delete[] file;
  delete &b64Wrt;
}

//---------------------------------------------------------------------------

bool SmtpAttachmentWriter::writeHeader()
{
  if (headerWritten) return true;

  headerWritten = true;

  char buf[256] = "";

  int len = sprintf(buf,"\r\n--%s\r\n",bndTxt);
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Type: application/octet-stream;\r\n"
                        " name=\"%s\"\r\n",file);
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Transfer-Encoding: base64\r\n");
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Disposition: attachment\r\n"
                    " filename=\"%s\"\r\n\r\n",file);
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  return true;
}

//---------------------------------------------------------------------------

bool SmtpAttachmentWriter::write(const char *buf, int sz)
{
  if (!writeHeader()) return false;

  return b64Wrt.write(buf,sz);
}

//---------------------------------------------------------------------------

bool SmtpAttachmentWriter::close()
{
  if (closed) return true;

  closed = true;

  return b64Wrt.flush();
}

} // namespace

//---------------------------------------------------------------------------
