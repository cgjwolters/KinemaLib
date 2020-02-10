//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Base64 encoding/decoding ------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Base64.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup encoding Encoding/Decoding
 @{
*/

//---------------------------------------------------------------------------
/** \file Base64.h
  Contains class Ino::Base64, which can be used to convert an array from/to
  Base64 encoding.

  The Base64 encoding is used to convert an array of 8-bit bytes to a
  representation that contains only readable ASCII characters.\n
  The resulting array is generally about 1/3 larger than the input.

  \author C. Wolters
  \date Nov 2005
*/

//---------------------------------------------------------------------------
/** \class Base64
   Use this class to convert an array of bytes to/from Base64 format.

  The Base64 encoding is used to convert an array of 8-bit bytes to a
  representation that contains only readable ASCII characters.\n
  The resulting array is generally about 1/3 larger than the input.

  \author C. Wolters
  \date Nov 2005
*/
  
//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------

static bool revListValid = false;

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

static char b64Rev[128];

//---------------------------------------------------------------------------

static void calcRevList()
{
  if (revListValid) return;

  for (int i=0; i<128; ++i) b64Rev[i] = -1;
  for (int i=0; i<64; ++i) b64Rev[(int)(b64Chars[i])] = (char)i;

  revListValid = true;
}

//---------------------------------------------------------------------------

void Base64::ensureCap(int minCap)
{
  if (minCap < 1) minCap = 1;

  if (!buf) {
    buf = new char[minCap];
    cap = minCap;
  }
  else {
    if (minCap <= cap) return;

    char *newBuf = new char[minCap];
    memmove(newBuf,buf,cap);

    delete[] buf;

    buf = newBuf;
    cap = minCap;
  }
}

//---------------------------------------------------------------------------
/** Constructor.
*/

Base64::Base64()
: buf(NULL), cap(0)
{
  calcRevList();
}

//---------------------------------------------------------------------------
/** Destructor.
*/

Base64::~Base64()
{
  if (buf) delete[] buf;
}

//---------------------------------------------------------------------------
/** Converts an array of 8-bit bytes to Base64 format.
  \param msg The data to convert, may be \c NULL.
  \param len The number of valid bytes in \c data.
  \param codeLen On return contains the number of bytes in the returned
  buffer.\n
  The value returned will always be a multiple of four.\n
  Will be zero if <tt>msg == NULL || len <= 0</tt>.
  \return A pointer to an internal buffer that contains the Base64
  representation of the input data.\n
  A trailing zero character is \b not automatically appended.\n
  \c NULL will be returned if <tt>msg == NULL || len < 0</tt>.\n
  The data in the returned buffer is only valid until the next call to this
  method or to method \ref decode(const char *msg, int len, int& decodeLen)
  "decode".
*/

const char *Base64::encode(const char *msg, int len, int& codeLen)
{
  codeLen = 0;

  if (!msg || len < 0) return NULL;

  if (len < 1) {
    ensureCap(1);
    buf[0] = '\0';

    return buf;
  }

  int mod = len % 3;
  codeLen = len;

  if (mod != 0)  codeLen += 3 - mod;
  codeLen = codeLen * 4 / 3;

  ensureCap(codeLen+1);

  mod = 0;
  int msk = 0, pos = 0;

  for (int i=0; i<len; ++i) {
    msk <<= 8; msk |= (msg[i] & 0xFF);

    if (mod >= 2) {
      buf[pos+3] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[pos+2] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[pos+1] = b64Chars[msk & 0x3F]; msk >>= 6;
      buf[pos]   = b64Chars[msk & 0x3F];

      mod = 0;
      msk = 0;
      pos += 4;
    }
    else mod++;
  }

  if (mod == 1) {
    buf[pos+3] = '=';
    buf[pos+2] = '=';

    msk <<= 4;
    buf[pos+1] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[pos]   = b64Chars[msk & 0x3F];

    pos += 4;
  }
  else if (mod == 2) {
    buf[pos+3] = '=';

    msk <<= 2;
    buf[pos+2] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[pos+1] = b64Chars[msk & 0x3F]; msk >>= 6;
    buf[pos]   = b64Chars[msk & 0x3F];

    pos += 4;
  }

  buf[codeLen] = '\0';

  return buf;
}

//---------------------------------------------------------------------------
/** Converts an array of 8-bit bytes back from Base64 format.
  \param msg The data to convert back, may be \c NULL.
  \param len The number of valid bytes in \c data, must be a multiple of four.
  \param decodeLen On return contains the number of bytes in the returned buffer.\n
  Will be zero if <tt>msg == NULL || len <= 0 || len\%4 != 0</tt> or else if a
  character is found in the buffer that could not possibly be present in a Base64
  encoded string.
  \return A pointer to an internal buffer that contains the decoded input data.\n
  A trailing zero character is \b not automatically appended.\n
  \c NULL is returned if <tt>msg == NULL || len < 0 || len\%4 != 0</tt> or else if a
  character is found in the buffer that could not possibly be present in a Base64
  encoded string.\n
  The data in the returned buffer is only valid until the next call to this method or
  to method \ref encode(const char *msg, int len, int& codeLen) "encode".
*/

const char *Base64::decode(const char *msg, int len, int& decodeLen)
{
  decodeLen = 0;

  if (!msg || len < 0 || len % 4 != 0) return NULL;

  if (len < 1) {
    ensureCap(1);
    buf[0] = '\0';

    return buf;
  }

  decodeLen = len/4*3;
  if (msg[len-1] == '=') decodeLen--;
  if (msg[len-2] == '=') decodeLen--;

  ensureCap(decodeLen+1);

  int msk = 0, pos = 0, mod = 0;

  for (int i=0; i<len; ++i) {
    msk <<= 6;

    if (msg[i] < 0) {
      decodeLen = 0;
      return NULL;
    }

    int code = b64Rev[(int)(msg[i])];
    if (code < 0) {
      if (i < len-2 || msg[i] != '=') {
        decodeLen = 0;
        return NULL;
      }
    }
    else msk |= (code & 0x3F);

    if (mod >= 3) {
      if (pos+2 < len) buf[pos+2] = (char)(msk & 0xFF); msk >>= 8;
      if (pos+1 < len) buf[pos+1] = (char)(msk & 0xFF); msk >>= 8;
      buf[pos] = (char)(msk & 0xFF);

      mod = 0;
      msk = 0;
      pos += 3;
    }
    else mod++;
  }

  return buf;
}

} // namespace Ino

//---------------------------------------------------------------------------
