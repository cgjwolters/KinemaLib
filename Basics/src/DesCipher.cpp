//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- DES Encryption/Decryption -----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "DesCipher.h"

//---------------------------------------------------------------------------

namespace Ino
{
//---------------------------------------------------------------------------
/** \addtogroup encoding Encoding/Decoding
 @{
*/

//---------------------------------------------------------------------------
/** \file DesCipher.h
  Contains class Ino::DesCipher, which implements the DES encryption
  algorithm.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** \class DesCipher
  Implements the DES encryption algorithm.

  This class contains the methods to encrypt as well as decrypt a string
  of bytes using the DES algorithm.\n
  \note
  Since DES uses a 64 bit key, data to be encrypted must have a length
  (in bytes) that is a multiple of 8. Pad the data with zeroes if required.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** \typedef DesBlock
  Not documented.
*/

typedef unsigned char DesBlock[64];

/* Transposition (0..63) */
/** \typedef DesOrdering
  Not documented.
*/
typedef unsigned char DesOrdering[64];

static DesOrdering
initialTr = {
     58, 50, 42, 34, 26, 18, 10,  2,
     60, 52, 44, 36, 28, 20, 12,  4,
     62, 54, 46, 38, 30, 22, 14,  6,
     64, 56, 48, 40, 32, 24, 16,  8,
     57, 49, 41, 33, 25, 17,  9,  1,
     59, 51, 43, 35, 27, 19, 11,  3,
     61, 53, 45, 37, 29, 21, 13,  5,
     63, 55, 47, 39, 31, 23, 15,  7 };

static DesOrdering
finalTr = {
     40,  8, 48, 16, 56, 24, 64, 32,
     39,  7, 47, 15, 55, 23, 63, 31,
     38,  6, 46, 14, 54, 22, 62, 30,
     37,  5, 45, 13, 53, 21, 61, 29,
     36,  4, 44, 12, 52, 20, 60, 28,
     35,  3, 43, 11, 51, 19, 59, 27,
     34,  2, 42, 10, 50, 18, 58, 26,
     33,  1, 41,  9, 49, 17, 57, 25 };

static DesOrdering
swap = {
     33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48,
     49, 50, 51, 52, 53, 54, 55, 56,
     57, 58, 59, 60, 61, 62, 63, 64,
      1,  2,  3,  4,  5,  6,  7,  8,
      9, 10, 11, 12, 13, 14, 15, 16,
     17, 18, 19, 20, 21, 22, 23, 24,
     25, 26, 27, 28, 29, 30, 31, 32 };

static DesOrdering
keyTr1 = {
     57, 49, 41, 33, 25, 17,  9,  1,
     58, 50, 42, 34, 26, 18, 10,  2,
     59, 51, 43, 35, 27, 19, 11,  3,
     60, 52, 44, 36,
     63, 55, 47, 39, 31, 23, 15,  7,
     62, 54, 46, 38, 30, 22, 14,  6,
     61, 53, 45, 37, 29, 21, 13,  5,
		                 28, 20, 12,  4 };

static DesOrdering
keyTr2 = {
     14, 17, 11, 24,  1,  5,
      3, 28, 15,  6, 21, 10,
     23, 19, 12,  4, 26,  8,
     16,  7, 27, 20, 13,  2,
     41, 52, 31, 37, 47, 55,
     30, 40, 51, 45, 33, 48,
     44, 49, 39, 56, 34, 53,
     46, 42, 50, 36, 29, 32 };

static DesOrdering
etr = {
     32,  1,  2,  3,  4,  5,
      4,  5,  6,  7,  8,  9,
      8,  9, 10, 11, 12, 13,
     12, 13, 14, 15, 16, 17,
     16, 17, 18, 19, 20, 21,
     20, 21, 22, 23, 24, 25,
     24, 25, 26, 27, 28, 29,
     28, 29, 30, 31, 32,  1 };

static DesOrdering
ptr = {
     16,  7, 20, 21,
     29, 12, 28, 17,
      1, 15, 23, 26,
      5, 18, 31, 10,
      2,  8, 24, 14,
     32, 27,  3,  9,
     19, 13, 30,  6,
     22, 11,  4, 25 };

static unsigned char s[8][64] = {
    {14,  4, 13,  1,  2, 15, 11,  8,
      3, 10,  6, 12,  5,  9,  0,  7,
      0, 15,  7,  4, 14,  2, 13,  1,
     10,  6, 12, 11,  9,  5,  3,  8,
      4,  1, 14,  8, 13,  6,  2, 11,
     15, 12,  9,  7,  3, 10,  5,  0,
     15, 12,  8,  2,  4,  9,  1,  7,
      5, 11,  3, 14, 10,  0,  6, 13 },

    {15,  1,  8, 14,  6, 11,  3,  4,
      9,  7,  2, 13, 12,  0,  5, 10,
      3, 13,  4,  7, 15,  2,  8, 14,
     12,  0,  1, 10,  6,  9, 11,  5,
      0, 14,  7, 11, 10,  4, 13,  1,
      5,  8, 12,  6,  9,  3,  2, 15,
     13,  8, 10,  1,  3, 15,  4,  2,
     11,  6,  7, 12,  0,  5, 14,  9 },

    {10,  0,  9, 14,  6,  3, 15,  5,
      1, 13, 12,  7, 11,  4,  2,  8,
     13,  7,  0,  9,  3,  4,  6, 10,
      2,  8,  5, 14, 12, 11, 15,  1,
     13,  6,  4,  9,  8, 15,  3,  0,
     11,  1,  2, 12,  5, 10, 14,  7,
      1, 10, 13,  0,  6,  9,  8,  7,
      4, 15, 14,  3, 11,  5,  2, 12 },

    { 7, 13, 14,  3,  0,  6,  9, 10,
      1,  2,  8,  5, 11, 12,  4, 15,
     13,  8, 11,  5,  6, 15,  0,  3,
      4,  7,  2, 12,  1, 10, 14,  9,
     10,  6,  9,  0, 12, 11,  7, 13,
     15,  1,  3, 14,  5,  2,  8,  4,
      3, 15,  0,  6, 10,  1, 13,  8,
      9,  4,  5, 11, 12,  7,  2, 14 },

    { 2, 12,  4,  1,  7, 10, 11,  6,
      8,  5,  3, 15, 13,  0, 14,  9,
     14, 11,  2, 12,  4,  7, 13,  1,
      5,  0, 15, 10,  3,  9,  8,  6,
      4,  2,  1, 11, 10, 13,  7,  8,
     15,  9, 12,  5,  6,  3,  0, 14,
     11,  8, 12,  7,  1, 14,  2, 13,
      6, 15,  0,  9, 10,  4,  5,  3 },

    {12,  1, 10, 15,  9,  2,  6,  8,
      0, 13,  3,  4, 14,  7,  5, 11,
     10, 15,  4,  2,  7, 12,  9,  5,
      6,  1, 13, 14,  0, 11,  3,  8,
      9, 14, 15,  5,  2,  8, 12,  3,
      7,  0,  4, 10,  1, 13, 11,  6,
      4,  3,  2, 12,  9,  5, 15, 10,
     11, 14,  1,  7,  6,  0,  8, 13 },

    { 4, 11,  2, 14, 15,  0,  8, 13,
      3, 12,  9,  7,  5, 10,  6,  1,
     13,  0, 11,  7,  4,  9,  1, 10,
     14,  3,  5, 12,  2, 15,  8,  6,
      1,  4, 11, 13, 12,  3,  7, 14,
     10, 15,  6,  8,  0,  5,  9,  2,
      6, 11, 13,  8,  1,  4, 10,  7,
      9,  5,  0, 15, 14,  2,  3, 12},

    {13,  2,  8,  4,  6, 15, 11,  1,
     10,  9,  3, 14,  5,  0, 12,  7,
      1, 15, 13,  8, 10,  3,  7,  4,
     12,  5,  6, 11,  0, 14,  9,  2,
      7, 11,  4,  1,  9, 12, 14,  2,
      0,  6, 10, 13, 15,  3,  5,  8,
      2,  1, 14,  7,  4, 10,  8, 13,
     15, 12,  9,  0,  3,  5,  6, 11 }
    };

static int lRots[16] = {
              1,  1,  2,  2,  2,  2,  2,  2,
              1,  2,  2,  2,  2,  2,  2,  1 };

static int rRots[16] = {
              0,  1,  2,  2,  2,  2,  2,  2,
              1,  2,  2,  2,  2,  2,  2,  1 };

//---------------------------------------------------------------------------

#define transpose(data,t,n,odata) {int ii; for (ii=0; ii<n; ii++)\
                                                   odata[ii]=data[t[ii]-1]; }

//---------------------------------------------------------------------------

static void copyBlock(const DesBlock src, DesBlock dst)
{
  for (int i=0; i<64; ++i) dst[i] = src[i];
}

//---------------------------------------------------------------------------
// ---- 1 bit left rotate on two 28 bit units -------------------------------
//---------------------------------------------------------------------------

static void lrotatel(DesBlock key)
{
 unsigned char k0 = key[0], k28 = key[28];

 for (int i=0; i<55; i++) key[i] = key[i+1];

 key[27] = k0; key[55] = k28;
}

//---------------------------------------------------------------------------

static void rrotater(DesBlock key)
{
 unsigned char k27 = key[27], k55 = key[55];

 for (int i=55; i>0; i--) key[i] = key[i-1];

 key[0] = k27; key[28] = k55;
}

//---------------------------------------------------------------------------

static void f(int i, DesBlock key, DesBlock a, DesBlock x, bool crypt)
{
 DesBlock e, ikey, y;

 transpose(a, etr, 48, e);
 /* expand e to 48 bits */

 if (crypt) {
   for (int j=0; j<lRots[i]; j++) lrotatel(key);
 }
 else {
   for (int j=0; j<rRots[i]; j++) rrotater(key);
 }

 transpose(key, keyTr2, 48, ikey);

 for (int j=0; j<48; j++) y[j] = (e[j]) ^ (ikey[j]);

 for (int k=0; k<8; k++) {
   /* substitute part */
   int k6=6*(k+1)-1, k4=4*(k+1)-1;
   int r = (y[k6-5]<<5) + (y[k6]<<4) + (y[k6-4]<<3) + (y[k6-3]<<2)
                                     + (y[k6-2]<<1) + y[k6-1];

   unsigned char skr = s[k][r];
   e[k4-3] = (skr & 8) != 0;
   e[k4-2] = (skr & 4) != 0;
   e[k4-1] = (skr & 2) != 0;
   e[k4]   = (skr & 1) != 0;
  }

 transpose(e,ptr,32,x);
}

//---------------------------------------------------------------------------

static void descrypt(DesBlock intext, DesBlock key, DesBlock outtext, bool crypt)
{
 DesBlock a, b, x;

 /* Initial transposition */
 transpose(intext, initialTr, 64, a);

 /* Mixup key and reduce to 56 bits */
 copyBlock(key,x);
 transpose(x, keyTr1, 56, key);

 /* 16 iterations */
 for (int i=0; i<16; i++) {
   copyBlock(a,b);

   /* Current left = old right */
   for (int j=0; j<32; j++) a[j] = b[j+32];

   /* Compute x = f(r[i-1],k[i]) */
   f(i,key,a,x,crypt);
   for (int j=0; j<32; j++) a[j+32] = (b[j]) ^ (x[j]);
  }

 /* swap left and right halves */
 transpose(a,swap,64,b);

 /* Final transposition */
 transpose(b,finalTr,64,outtext);
}

/* ----------------------------------------------------------------------- */
/* ---- Expand 8x8 bits to 8x8 bytes ------------------------------------- */
/* ----------------------------------------------------------------------- */

static void desExpand(const char intxt[8], DesBlock outtxt)
{
 for (int i=0; i<8; i++) {
   unsigned char c = intxt[i];

   for (int j=0; j<8; j++) {
     *outtxt++ = (c & 128) != 0;
     c <<= 1;
    }
  }
}
/* ----------------------------------------------------------------------- */
/* ---- Compress 8x8 bytes to 8x8 bits ----------------------------------- */
/* ----------------------------------------------------------------------- */

static void desCompress(const DesBlock intxt, char outtxt[8])
{
 for (int i=0; i<8; i++) {
   unsigned char c = 0;

   for (int j=0; j<8; j++) {
     c <<= 1;
     c |= *intxt++;
   }
   
   outtxt[i] = c;
  }
}

//---------------------------------------------------------------------------
/** Constructor.
  \param key The encryption/decryption key to be used, must \b not be \c NULL,
  the key must be 8 bytes long.

  A copy of the supplied key is stored internally. 
  \note
  Behavior is undefined if parameter \c key is \c NULL, most likely a program
  crash will follow.
*/

DesCipher::DesCipher(const char key[8])
{
  for (int i=0; i<8; ++i) chKey[i] = key[i];
  desExpand(chKey,desKey);
}

//---------------------------------------------------------------------------
/** Copy constructor.
  \param cp The object to copy.

  The stored key is copied as well.
*/

DesCipher::DesCipher(const DesCipher& cp)
{
  for (int i=0; i<8; ++i) chKey[i] = cp.chKey[i];
  for (int i=0; i<64; ++i) desKey[i] = cp.desKey[i];
}

//---------------------------------------------------------------------------
/** Assigment operator.
  \param src The object to assign from.
  \return A reference to this object.

  This object copies its key from \c src as well.
*/

DesCipher& DesCipher::operator=(const DesCipher& src)
{
  for (int i=0; i<8; ++i) chKey[i] = src.chKey[i];
  for (int i=0; i<64; ++i) desKey[i] = src.desKey[i];

  return *this;
}

//---------------------------------------------------------------------------
/** Encrypts an array of 8-bit bytes.
  \param plainTxt The data to be encrypted, must \b not be \c NULL.
  \param len The number of bytes in \c plainTxt that must be encrypted,\n \b must be
  a multiple of 8. (Pad with zeroes if required).
  \param cipherTxt Pointer to a buffer that will receive the encrypted result,\n
  must \b not be \c NULL.\n
  The capacity of the buffer must be at least \c len bytes.
  \return \c true if encryption was successfull,\n
  \c false if \c len is not a multiple of 8.
  \note
  If either \c plainTxt or \c cipherTxt is \c NULL behavior is undefined.\n
  Most likely a program crash will occur.\n
*/

bool DesCipher::encrypt(const char *plainTxt, int len, char *cipherTxt)
{
  DesBlock expPlain, expCipher, locKey;

  if (len&7) return false;

  while (len >= 8) {
    desExpand(plainTxt,expPlain);
    copyBlock(desKey,locKey);
    descrypt(expPlain, locKey, expCipher, true);
    desCompress(expCipher,cipherTxt);

    plainTxt += 8; cipherTxt += 8; len -= 8;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Decrypts an array of 8-bit bytes.
  \param cipherTxt The data to be decrypted, must \b not be \c NULL.
  \param len The number of bytes in \c cipherTxt that must be decrypted,\n
  \b must be a multiple of 8.
  \param plainTxt Pointer to a buffer that will receive the decrypted result,\n
  must \b not be \c NULL.\n
  The capacity of the buffer must be at least \c len bytes.
  \return \c true if decryption was successfull,\n
  \c false if \c len is not a multiple of 8.
  \note
  If either \c cipherTxt or \c plainTxt is \c NULL behavior is undefined.\n
  Most likely a program crash will occur.\n
*/

bool DesCipher::decrypt(const char *cipherTxt, int len, char *plainTxt)
{
 DesBlock expPlain, expCipher, locKey;

 if (len&7) return false;

 while (len >= 8) {
   desExpand(cipherTxt,expCipher);
   copyBlock(desKey,locKey);
   descrypt(expCipher, locKey, expPlain, false);
   desCompress(expPlain,plainTxt);

   cipherTxt += 8; plainTxt += 8; len -= 8;
  }

 return true;
}

//---------------------------------------------------------------------------
/** Encrypts an array of 16-bit Unicode characters.
  \param plainTxt The data to be encrypted, must \b not be \c NULL.
  \param len The number of Unicode characters in \c plainTxt that must be encrypted,\n
  \b must be a multiple of 4. (Pad with zeroes if required).
  \param cipherTxt Pointer to a buffer that will receive the encrypted result,\n
  must \b not be \c NULL.\n
  The capacity of the buffer must be at least \c len Unicode characters.
  \return \c true if encryption was successfull,\n
  \c false if \c len is not a multiple of 4.
  \note
  If either \c plainTxt or \c cipherTxt is \c NULL behavior is undefined.\n
  Most likely a program crash will occur.\n
*/

bool DesCipher::encrypt(const wchar_t *plainTxt, int len, wchar_t *cipherTxt)
{
  const char *chPlainTxt = (const char *)plainTxt;
  char *chCipherTxt      = (char *)cipherTxt;

  return encrypt(chPlainTxt,len*2,chCipherTxt);
}

//---------------------------------------------------------------------------
/** Decrypts an array of 16-bit Unicode characters.
  \param cipherTxt The data to be decrypted, must \b not be \c NULL.
  \param len The number of Unicode characters in \c cipherTxt that must be decrypted,\n
  \b must be a multiple of 4.
  \param plainTxt Pointer to a buffer that will receive the decrypted result,\n
  must \b not be \c NULL.\n
  The capacity of the buffer must be at least \c len Unicode characters.
  \return \c true if decryption was successfull,\n
  \c false if \c len is not a multiple of 4.
  \note
  If either \c cipherTxt or \c plainTxt is \c NULL behavior is undefined.\n
  Most likely a program crash will occur.\n
*/

bool DesCipher::decrypt(const wchar_t *cipherTxt, int len, wchar_t *plainTxt)
{
  const char *chCipherTxt = (const char *)cipherTxt;
  char *chPlainTxt        = (char *)plainTxt;

  return decrypt(chCipherTxt,len*2,chPlainTxt);
}

} // namespace Ino

//---------------------------------------------------------------------------
