//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General basic types, constants and functions ----------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2000 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOBASETYPE_INC
#define INOBASETYPE_INC

#ifndef __cplusplus
#error This code is only suitable for C++
#endif

#include <cwchar>

#ifdef __BORLANDC__
#include <_stddef.h>
#endif

#ifdef __GNUC__
#define __int64 long long
#endif

namespace Ino
{
//---------------------------------------------------------------------------
//--- The version of this library -------------------------------------------
//---------------------------------------------------------------------------

extern void inoLibVersion(unsigned char& major, unsigned char& minor,
                                                  unsigned char& release);

//---------------------------------------------------------------------------
//------ Some useful constants ----------------------------------------------
//---------------------------------------------------------------------------

extern const double NumAccuracy;
extern const double InchInMm;

//---------------------------------------------------------------------------
//------- Some useful generic functions -------------------------------------
//---------------------------------------------------------------------------

template <class T> inline T sqr(T v) { return v*v; }
template <class T> inline T abs(T v) { return v < 0 ? -v : v; }

//---------------------------------------------------------------------------
//------ Some character manipulations ---------------------------------------
//---------------------------------------------------------------------------

extern void trim(char *s);
extern void trim(wchar_t *s);

extern void toUpper(char *s);
extern void toUpper(wchar_t *s);

extern void toLower(char *s);
extern void toLower(wchar_t *s);

extern char *dupStr(const char *s);
extern wchar_t *dupStr(const wchar_t *s);

extern wchar_t *dupChar2WChar(const char *s);
extern void char2WChar(const char *s, wchar_t *ws);

extern char *dupWChar2Char(const wchar_t *s);
extern void WChar2Char(const wchar_t *ws, char *s);

extern long hashCode(const char *s);
extern long hashCode(const wchar_t *s);

extern const char *readField(const char *s, char *buf, bool trimFld=true);
extern const wchar_t *readField(const wchar_t *s, wchar_t *buf, bool trimFld=true);

extern bool toInt(const char *s, long& intVal);
extern bool toInt(const wchar_t *s, long& intVal);

extern bool toDouble(const char *s, double& dblVal);
extern bool toDouble(const wchar_t *s, double& dblVal);

extern const char    *fromInt(long val);
extern const wchar_t *fromIntW(long val);

extern const char    *fromDouble(double val, int digits=-1);
extern const wchar_t *fromDoubleW(double val, int digits=-1);

extern double round(double val, double delta);

extern int compareStr(const char *s1, const char *s2);
extern int compareStr(const wchar_t *s1, const wchar_t *s2);

extern int compareNcStr(const char *s1, const char *s2);
extern int compareNcStr(const wchar_t *s1, const wchar_t *s2);

extern int compareAlphaNum(const char *s1, const char *s2);
extern int compareAlphaNum(const wchar_t *s1, const wchar_t *s2);

extern int compareNcAlphaNum(const char *s1, const char *s2);
extern int compareNcAlphaNum(const wchar_t *s1, const wchar_t *s2);

extern double toDegrees(double val);
extern double toRadians(double val);

extern long currentTime();

//---------------------------------------------------------------------------
//------ Some filename manipulations ----------------------------------------
//---------------------------------------------------------------------------

extern bool fileDateTime(char *s, size_t cap);
extern bool fileDateTime(wchar_t *s, size_t cap);

//---------------------------------------------------------------------------

class ProgressReporter
{
  long reportInc, maxProg, maxSc;
  long curProg, curSc;
  bool abort;

  ProgressReporter(const ProgressReporter& cp);
  ProgressReporter& operator=(const ProgressReporter& src);

public:
  ProgressReporter(long maxProgress, long reportIncrement=4096,
                                                      long maxScale=100);

  long getReportInc() const { return reportInc; }

  void reset(long maxProgress, long maxScale=100);

  bool setProgress(int newProgress);
  bool incProgress(int progInc=1);

  virtual bool progressReport(int progScale) = 0;
  bool mustAbort() const { return abort; }
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
