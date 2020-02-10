//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Base64 formatted Output Stream ------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

namespace Ino {

//---------------------------------------------------------------------------

static const char b64Chars[] = {
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/',
  };

//---------------------------------------------------------------------------

Base64Writer::Base64Writer(Writer& writer, ProgressReporter *rep)
: Writer(rep), wrt(writer), bufSz(0), mod(0), msk(0)
{
}

//---------------------------------------------------------------------------

Base64Writer::~Base64Writer()
{
  flush();
}

//---------------------------------------------------------------------------

bool Base64Writer::reportProgress()
{
  if (!reporter || bytesInc < reporter->getReportInc()) return true;

  bool ok = reporter->setProgress(bytesWritten);

  bytesInc = 0;

  return ok;
}

//---------------------------------------------------------------------------
/** File closed indicator.
  \return \c true if:
  \li The underlying Writer reports a closed condition.
  \li isAborted() would return \c true.
*/

bool Base64Writer::isClosed() const
{
  return wrt.isClosed() || isAborted();
}

//---------------------------------------------------------------------------
/** Abort Indicator.
  \return The abort status of the underlying writer or else, if there is
  a ProgressReporter, the abort status of the ProgressReporter.

  \note
  Even if \c true is returned, further data may still be written unless there
  is another reason for failure.
*/

bool Base64Writer::isAborted() const
{
  if (wrt.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------

bool Base64Writer::writeBuf()
{
  char hb[4];
  int sz = 0;

  if (bufSz > 76) {
    for (int i=76; i<bufSz; ++i) hb[sz++] = buf[i];
    bufSz = 76;
  }

  buf[bufSz++] = '\r';
  buf[bufSz++] = '\n';

  if (!wrt.write(buf,bufSz)) {
    bufSz = 0;
    msk   = 0;
    mod   = 0;

    return false;
  }

  bytesWritten += bufSz;
  bytesInc     += bufSz;

  bufSz = sz;
  for (int i=0; i<bufSz; ++i) buf[i] = hb[i];

  return reportProgress();
}

//---------------------------------------------------------------------------

bool Base64Writer::write(const char *msg, int msgLen)
{
  if (!msg || msgLen < 0) return false;

  for (int i=0; i<msgLen; ++i) {
    msk <<= 8; msk |= (msg[i] & 0xFF);
    mod++;

    if (mod >= 3) {
      buf[bufSz+3] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[bufSz+2] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[bufSz+1] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[bufSz]   = b64Chars[msk & 0x3F];
      
      bufSz += 4;

      if (bufSz >= 76 && !writeBuf()) return false;

      mod = 0;
      msk = 0;
    }
  }

  return true;
}

//---------------------------------------------------------------------------

bool Base64Writer::flush()
{
  if (mod == 1) {
    buf[bufSz+3] = '=';
    buf[bufSz+2] = '=';

    msk <<= 4;
    buf[bufSz+1] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[bufSz]   = b64Chars[msk & 0x3F];

    bufSz += 4;
  }
  else if (mod == 2) {
    buf[bufSz+3] = '=';

    msk <<= 2;
    buf[bufSz+2] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[bufSz+1] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[bufSz]   = b64Chars[msk & 0x3F];

    bufSz += 4;
  }

  if (bufSz > 0 && !writeBuf()) return false;
  if (bufSz > 0 && !writeBuf()) return false;

  msk = 0;
  mod = 0;

  return true;
}

} // namespace

//---------------------------------------------------------------------------
