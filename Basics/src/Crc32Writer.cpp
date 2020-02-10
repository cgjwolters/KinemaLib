//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Calculates a crc-32 on a byte stream ------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Sept 2007 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include "zutil.h"      /* for STDC and FAR definitions */

//---------------------------------------------------------------------------

ZEXTERN uLong ZEXPORT crc32 OF((uLong crc, const Bytef *buf, uInt len));

//---------------------------------------------------------------------------

namespace Ino
{

//---------------------------------------------------------------------------

Crc32Writer::Crc32Writer(Writer& writer, ProgressReporter *rep)
: Writer(rep), wrt(writer), crc(crc32(0,Z_NULL,0))
{
}

//---------------------------------------------------------------------------

void Crc32Writer::resetCrc()
{
  crc = crc32(0,Z_NULL,0);
}

//---------------------------------------------------------------------------

bool Crc32Writer::isClosed() const
{
  return wrt.isClosed() || isAborted();
}

//---------------------------------------------------------------------------

bool Crc32Writer::isAborted() const
{
  if (wrt.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------

bool Crc32Writer::write(const char *buf, int sz)
{
  if (!buf) throw NullPointerException("Crc32Writer::write: v == NULL");
  if (sz < 0) throw IllegalArgumentException("Crc32Writer::write: sz < 0");

  if (isClosed()) return false;
  if (sz < 1) return true;

  if (!wrt.write(buf,sz)) return false;

  bytesWritten += sz;
  bytesInc     += sz;

  crc = crc32(crc,(const Bytef *)buf,sz);

  bool ok = reportProgress();
  if (!ok) wrt.flush();

  return ok;
}

//---------------------------------------------------------------------------

bool Crc32Writer::flush()
{
  return wrt.flush();
}

} // namespace Ino

//---------------------------------------------------------------------------
