//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Calculate CCITT CRC checksums -------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Crc.h"

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup encoding Encoding/Decoding
 @{
*/

//---------------------------------------------------------------------------

/** \file Crc.h
  Contains functions that perform the calculation of a 16-bit <em>CCITT
  Cyclic Redundancy Check</em> on a string of bytes or Unicode characters.

  \author C. Wolters
  \date Nov 2005
*/

//---------------------------------------------------------------------------

static bool crc16Initialized = false;
static unsigned short crc16Tab[256];

static void crc16Init()
{
 unsigned short bin=2, idx=1;

 crc16Tab[0] = 0; crc16Tab[1] = 0x1021; /* CCITT in hex */

 do {
   crc16Tab[bin] = crc16Tab[idx]<<1;
   if (crc16Tab[idx] & 0x8000) crc16Tab[bin] = crc16Tab[bin]^crc16Tab[1];

   for (idx=1; idx<bin; idx++) crc16Tab[bin+idx]=crc16Tab[bin]^crc16Tab[idx];

   idx = bin; bin <<= 1;
  }
  while (bin <= 128);
}

//---------------------------------------------------------------------------
/** Calculates a 16 bit CRC on a string of 8-bit bytes.
  \param data The string of bytes to calculate the CRC of, must \b not be
  \c NULL.
  \param len The number of bytes in the buffer that parameter \c
  data points to.
  \param chksum The 2 byte buffer that receives the calculated CRC,
  must \b not be \c NULL.

  \attention Behavior is undefined if \c data or \c chksum are \c NULL.\n
  Most likely a program crash will occur.
*/

void crc16(const char *data, int len, char chksum[2])
{
  if (!crc16Initialized) {
    crc16Init();
    crc16Initialized = true;
  }

  unsigned short wrd = 0;

  for (int i=0; i<len; i++) {
    wrd = ((wrd<<8) | ((*data++)&0xff)) ^ crc16Tab[(unsigned short)wrd>>8];
  }

  for (int i=0; i<2; i++) {
    wrd =  (wrd<<8) ^ crc16Tab[(unsigned short)wrd>>8];
  }

  chksum[0] = (unsigned short)wrd>>8;
  chksum[1] = wrd & 0xff;
}

//---------------------------------------------------------------------------
/** Calculates a 16 bit CRC on a string of 16-bit Unicode characters.
  \param data The string of characters to calculate the CRC of, must \b not be
  \c NULL.
  \param len The number of characters in the buffer that parameter \c data
  points to.
  \param chksum The 2 byte buffer that receives the calculated CRC,
  must \b not be \c NULL.

  \attention Behavior is undefined if \c data or \c chksum are \c NULL.\n
  Most likely a program crash will occur.
*/

void crc16(const wchar_t *data, int len, char chksum[2])
{
  const char *chData = (const char *)data;

  crc16(chData,len*2,chksum);
}

/**
 @}
*/

} // namespace Ino

//---------------------------------------------------------------------------
