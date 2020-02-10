//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Type.h"

#include "Basics.h"
#include "Exceptions.h"
#include "Reader.h"
#include "Writer.h"

namespace InoPersist
{

using namespace Ino;

//---------------------------------------------------------------------------

int Type::readRecordLen(DataReader& dRdr, const char *exceptMsg)
{
  short sLen = dRdr.readShort(exceptMsg);

  int len = sLen;

  if (len < 0) {
    sLen = dRdr.readShort(exceptMsg);

    len <<= 16;
    len |= (sLen & 0xFFFF);

    len = -len;
  }

  return len;
}

//---------------------------------------------------------------------------

void Type::writeRecordLen(DataWriter& dWrt, int len, const char *exceptMsg)
{
  if (len <= 0x7FFF) dWrt.writeShort((short)len,exceptMsg);
  else dWrt.writeInt(-len,exceptMsg);
}

} // namespace Ino

//---------------------------------------------------------------------------
