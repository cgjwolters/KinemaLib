//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- License Codeword Encoder ------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef AW_WNAGC_INC
#define AW_WNAGC_INC

#include <time.h>

namespace AW
{

//-------------------------------------------------------------------------

class WNagC
{
  __int64 intMac;
  bool valid;
  char lcw[17];

  WNagC(const WNagC& cp);
  WNagC& operator=(const WNagC& src);

public:
  WNagC(unsigned char appId, const char mac[17], time_t limitTime,
        unsigned short feats, unsigned char maxMajor,
        unsigned char maxMinor);

  const char *getCodeWord() const;

  bool macIsGeneric() const;
};

} // namespace

//---------------------------------------------------------------------------
#endif
