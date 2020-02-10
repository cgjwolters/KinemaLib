//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- DES Encryption/Decryption -----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INO_DES_INC
#define INO_DES_INC

#include <wchar.h>

namespace Ino
{

//---------------------------------------------------------------------------

class DesCipher
{
  char chKey[8];
  unsigned char desKey[64];

public:
  DesCipher(const char key[8]);
  DesCipher(const DesCipher& cp);

  DesCipher& operator=(const DesCipher& src);

  // len must be multiple of 8:
  bool encrypt(const char *plainTxt, int len, char *cipherTxt);
  bool decrypt(const char *cipherTxt, int len, char *plaintTxt);

  // len must be multiple of 4:
  bool encrypt(const wchar_t *plainTxt, int len, wchar_t *cipherTxt);
  bool decrypt(const wchar_t *cipherTxt, int len, wchar_t *plaintTxt);
};

} // namespace Ino


//---------------------------------------------------------------------------
#endif
