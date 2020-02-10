//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- License Codeword Encoder ------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "StdAfx.h"

#include "WNagC.h"
#include "Crc.h"
#include "DesCipher.h"
#include "MacAddress.h"
#include "Hex.h"

#include <stdlib.h>

namespace AW
{

//---------------------------------------------------------------------------

// lcw byte layout:
// 0,1  multiply with 86400 to get ultimate valid time (as a time_t)
// 2,3  feature bits
// 4    ultimate major/minor version
// 5    appId
// 6,7  CRC 16 checksum
// 
// See also class CNagW

//---------------------------------------------------------------------------

static void cpKey(__int64 ma, char key[8])
{
  for (int i=5; i>=0; --i) {
    key[i] = (char)(ma & 0xff);
    ma >>= 8;
  }

  key[6] = 98;
  key[7] = 52;
}

//---------------------------------------------------------------------------

WNagC::WNagC(unsigned char appId, const char mac[17], time_t limitTime,
             unsigned short feats,
                             unsigned char maxMajor, unsigned char maxMinor)
: intMac(0), valid(false)
{
  char rawCw[9];

  long tm = (long)(limitTime/86400);

  rawCw[1] = (char)(tm & 0xff);
  rawCw[0] = (char)((tm >> 8) & 0xff);
  rawCw[3] = (char)(feats & 0xff);
  rawCw[2] = (char)((feats >> 8) & 0xff);
  rawCw[4] = (char)(((maxMajor & 0x0f) << 4) | (maxMinor & 0x0f));
  rawCw[5] = appId;

  AW::crc16(rawCw,6,rawCw+6);

  if (!MacAddress::decodeHexDecryptedMacStr(mac,intMac)) return;

  char key[8];
  cpKey(intMac,key);

  AW::DesCipher des(key);

  char encCw[8];
  des.encrypt(rawCw,8,encCw);

  Hex b16;
  int encodeLen = 0;
  const char *encStr = b16.encode(encCw,8,encodeLen);

  if (encStr && encodeLen == 16) {
    for (int i=0; i<16; ++i) lcw[i] = encStr[i];
    lcw[16] = '\0';
    valid = true;
  }
}

//---------------------------------------------------------------------------

const char *WNagC::getCodeWord() const
{
  if (!valid) return NULL;

  return lcw;
}

//---------------------------------------------------------------------------

bool WNagC::macIsGeneric() const
{
  return intMac == MacAddress::generic;
}

} // namespace

//---------------------------------------------------------------------------
