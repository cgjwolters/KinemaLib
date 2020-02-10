//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- License Codeword Decoder -----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "StdAfx.h"

#include "CNagW.h"

#include "MacAddress.h"
#include "Crc.h"
#include "DesCipher.h"
#include "Hex.h"

#include <string.h>
#include <time.h>

namespace AW
{

//---------------------------------------------------------------------------

// lcw byte layout:
// 0,1  multiply with 86400 to get ultimate valid time (as a time_t)
// 2,3  feature bits
// 4    ultimate major/minor version
// 5    Application ID
// 6,7  CRC 16 checksum
// 
// See also class WNagC

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

static long decodeTime(const char tm[13])
{
  if (!tm) return 0;

  Hex b16;
  int decodeLen = 0;

  const char *decTm = b16.decode(tm,12,decodeLen);
  if (!decTm || decodeLen != 6) return 0;

  long iTm = 0;
  for (int i=0; i<4; ++i) {
    iTm <<= 8;
    iTm |= (decTm[i] & 0xff);
  }

  return iTm;
}

//---------------------------------------------------------------------------

CNagW::CNagW(unsigned char appId, const char cw[17], const char tm[13],
                                 unsigned char major, unsigned char minor)
: valid(false), regFromUser(false),
  curTime((long)(_time64(NULL)/86400)),
  majVer(major), minVer(minor)
{
  tmNow[0] = '\0';

  if (tm) {
    long lTm = decodeTime(tm);
    if (lTm > curTime) curTime = lTm;
  }

  for (int i=0; i<8; ++i) lcw[i] = '\0';
  lcw[6] = 1; lcw[7] = 2; // Make CRC invalid

  for (i=0; i<16; ++i) scw[i] = cw[i];
  scw[16] = '\0';

  MacAddressList mAddrLst;

  // Set MchId
  if (mAddrLst.size() > 0) {
    const MacAddress& mAddr = mAddrLst.getDevice(0);
    strcpy(mchId,mAddr.getHexEncryptedMacAddressString());
  }
  else {
    const MacAddress& mAddr = mAddrLst.getGeneric();
    strcpy(mchId,mAddr.getHexEncryptedMacAddressString());
  }

  Hex b16;
  int decodeLen = 0;
  const char *decStr = b16.decode(scw,16,decodeLen);

  if (!decStr || decodeLen != 8) return;

  // Check codeword:

  for (i=0; i<mAddrLst.size(); ++i) {
    const MacAddress& mAddr = mAddrLst.getDevice(i);

    char key[8];
    cpKey(mAddr.getMacAddress(),key);

    AW::DesCipher des(key);

    char ncw[8], chk[2];
    des.decrypt(decStr,8,ncw);

    AW::crc16(ncw,8,chk);

    if (chk[0] == 0 && chk[1] == 0) { // This one ok
      for (int j=0; j<8; j++) lcw[j] = ncw[j];
      valid = lcw[5] == appId;
      return;
    }
  }

  // No valid cw for any of the Ethernet interfaces
  // so try generic macAdr

  char key[8];
  cpKey(MacAddress::generic,key);

  AW::DesCipher des(key);

  char ncw[8], chk[2];
  des.decrypt(decStr,8,ncw);

  AW::crc16(ncw,8,chk);

  if (chk[0] == 0 && chk[1] == 0) { // This one ok
    for (int j=0; j<8; j++) lcw[j] = ncw[j];
    valid = lcw[5] == appId;
  }
}

//---------------------------------------------------------------------------

CNagW::~CNagW()
{
}

//---------------------------------------------------------------------------

bool CNagW::isValid() const
{
  return valid;
}

//---------------------------------------------------------------------------

bool CNagW::expired() const
{
  if (!valid) return true;
  if (!isPermanent() && getDaysLeft() <= 0) return true;
  if (getMajor() < majVer) return true;
  if (getMajor() == majVer && getMinor() < minVer) return true;

  return false;
}

//---------------------------------------------------------------------------

unsigned short CNagW::getFeats() const
{
  unsigned short f = 0;
  f |= (lcw[2] & 0xff); f <<= 8;
  f |= (lcw[3] & 0xff);

  return f;
}

//---------------------------------------------------------------------------

long CNagW::getDaysLeft() const
{
  if (!valid) return 0;

  long t = 0;

  t |= (lcw[0] & 0xff); t <<= 8;
  t |= (lcw[1] & 0xff);

  if (t == 0) return Permanent;

  t -= curTime;

  return t;
}

//---------------------------------------------------------------------------

bool CNagW::isPermanent() const
{
  if (!valid) return 0;

  long t = 0;

  t |= (lcw[0] & 0xff); t <<= 8;
  t |= (lcw[1] & 0xff);

  return t==0;
}

//---------------------------------------------------------------------------

unsigned char CNagW::getMajor() const
{
  return (lcw[4] >> 4) & 0x0f;
}

//---------------------------------------------------------------------------

unsigned char CNagW::getMinor() const
{
  return lcw[4] & 0x0f;
}

//---------------------------------------------------------------------------

const char *CNagW::getTmNow() const
{
  long now = (long)(_time64(NULL)/86400);
  if (curTime > now) now = curTime;

  char ascTmVal[7];
  for (int i=3; i>=0; --i) {
    ascTmVal[i] = (char)(now & 0xff);
    now >>= 8;
  }

  ascTmVal[4] = 'a'; ascTmVal[5] = 'G'; ascTmVal[6] = '\0';

  Hex b16;
  int encodeLen;

  const char *encTm = b16.encode(ascTmVal,6,encodeLen);
  if (encodeLen != 12) return NULL;

  for (int i=0; i<12; i++) tmNow[i] = encTm[i];
  tmNow[12] = '\0';

  return tmNow;
}

//---------------------------------------------------------------------------

unsigned char CNagW::getAppId() const
{
  return lcw[5];
}

//---------------------------------------------------------------------------

bool CNagW::getMachineId(char id[17])
{
  if (!id) return false;

  MacAddressList mAddrLst;

  // Set MchId
  if (mAddrLst.size() > 0) {
    const MacAddress& mAddr = mAddrLst.getDevice(0);
    strcpy(id,mAddr.getHexEncryptedMacAddressString());
  }
  else {
    const MacAddress& mAddr = mAddrLst.getGeneric();
    strcpy(id,mAddr.getHexEncryptedMacAddressString());
  }

  return true;
}

//---------------------------------------------------------------------------

bool CNagW::getTmNow(char tmNow[13])
{
  if (!tmNow) return false;

  long now = (long)(_time64(NULL)/86400);

  char ascTmVal[7];
  for (int i=3; i>=0; --i) {
    ascTmVal[i] = (char)(now & 0xff);
    now >>= 8;
  }

  ascTmVal[4] = 'a'; ascTmVal[5] = 'G'; ascTmVal[6] = '\0';

  Hex b16;
  int encodeLen;

  const char *encTm = b16.encode(ascTmVal,6,encodeLen);
  if (encodeLen != 12) return NULL;

  for (int i=0; i<12; i++) tmNow[i] = encTm[i];
  tmNow[12] = '\0';

  return true;
}

} // namespace

//---------------------------------------------------------------------------
