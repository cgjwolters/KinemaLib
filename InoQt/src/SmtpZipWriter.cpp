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

#include "SmtpZipWriter.h"

#include "Smtp.h"
#include "Basics.h"

#include <stdio.h>

namespace Ino {

//---------------------------------------------------------------------------

SmtpZipWriter::SmtpZipWriter(Smtp& mailer, const char* zipName,
                                                     const char *boundary)
: ZipOut(*new Base64Writer(mailer)), 
  b64Wrt((Base64Writer &)getWriter()), smtp(mailer),
  zipFile(dupStr(zipName)), headerWritten(false),
  bndTxt(dupStr(boundary))
{
  if (!boundary)
    throw IllegalArgumentException("SmtpZipWriter::SmtpZipWriter");
}

//---------------------------------------------------------------------------

SmtpZipWriter::~SmtpZipWriter()
{
  close();

  delete[] bndTxt;
  delete[] zipFile;
  delete &b64Wrt;
}

//---------------------------------------------------------------------------

bool SmtpZipWriter::writeHeader()
{
  if (headerWritten) return true;

  headerWritten = true;

  char buf[256] = "";

  int len = sprintf(buf,"\r\n--%s\r\n",bndTxt);
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Type: application/zip;\r\n"
                        " name=\"%s\"\r\n",zipFile);
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Transfer-Encoding: base64\r\n");
  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  len = sprintf(buf,"Content-Disposition: attachment\r\n"
                    " filename=\"%s\"\r\n\r\n",zipFile);

  if (!smtp.write(buf,len)) return false;
  smtp.logOutput(buf);

  return true;
}

//---------------------------------------------------------------------------

bool SmtpZipWriter::isClosed() const
{
  return ZipOut::isClosed();
}

//---------------------------------------------------------------------------

bool SmtpZipWriter::addFile(const char *path, bool compressed)
{
  if (!writeHeader()) return false;

  bool ok = ZipOut::addFile(path,compressed);

  if (ok) smtp.addingZipFile(zipFile,path);

  return ok;
}

//---------------------------------------------------------------------------

bool SmtpZipWriter::write(const char *buf, int sz)
{
  if (!writeHeader()) return false;

  return ZipOut::write(buf,sz);
}

//---------------------------------------------------------------------------

bool SmtpZipWriter::close()
{
  return ZipOut::close();
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------
