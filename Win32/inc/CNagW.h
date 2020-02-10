//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- License Codeword Decoder -----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef AW_CNAGW_INC
#define AW_CNAGW_INC

namespace AW
{

//-------------------------------------------------------------------------

class CNagW
{
public:
  enum { Permanent = 0x7FFFFFFF };

private:
  bool valid;
  bool regFromUser;
  long curTime;
  char scw[17];
  char lcw[8];
  char mchId[17];
  mutable char tmNow[13];
  unsigned char majVer, minVer;

  CNagW(const CNagW& cp);
  CNagW& operator=(const CNagW& src);

public:
  CNagW(unsigned char appId, const char cw[17], const char tm[13] = 0,
                        unsigned char major=1, unsigned char minor=0);

  ~CNagW();
  bool isValid() const;
  bool expired() const;

  unsigned short getFeats() const;
  long getDaysLeft() const;
  bool isPermanent() const;

  unsigned char getMajor() const;
  unsigned char getMinor() const;

  const char *getMachineId() const { return mchId; }
  unsigned char getAppId() const;
  const char *getTmNow() const;

  static bool getMachineId(char id[17]);
  static bool getTmNow(char tmNow[13]);
};

} // namespace

//---------------------------------------------------------------------------
#endif
