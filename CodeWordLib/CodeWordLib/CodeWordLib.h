//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C-Interface to License Codeword Decoder ---------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifdef CODEWORDLIB_EXPORTS
#define CODEWORDLIB_API __declspec(dllexport)
#else
#define CODEWORDLIB_API __declspec(dllimport)
#endif

//---------------------------------------------------------------------------

extern "C" {

CODEWORDLIB_API bool decCwInit(unsigned char appId, const char cw[17],
                               const char tm[13],
                               unsigned char major, unsigned char minor);

CODEWORDLIB_API bool decCwIsValid();
CODEWORDLIB_API bool decCwExpired();

CODEWORDLIB_API unsigned short decCwGetFeats();
CODEWORDLIB_API bool decCwIsPermanent();
CODEWORDLIB_API long decCwGetDaysLeft();

CODEWORDLIB_API unsigned char decCwGetMajor();
CODEWORDLIB_API unsigned char decCwGetMinor();

CODEWORDLIB_API bool getMachineId(char id[17]);
CODEWORDLIB_API bool getTmNow(char tmNow[13]);

CODEWORDLIB_API void decCwFree();

} // extern "C"

//---------------------------------------------------------------------------

