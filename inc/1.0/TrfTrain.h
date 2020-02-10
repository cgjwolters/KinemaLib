//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------------- Transform Matrix Train Manager -----------------------------
//---------------------------------------------------------------------------
//------------------------ Copyright Inofor Hoek Aut BV 2004 ----------------
//---------------------------------------------------------------------------
//------------------------------------------------------ C.Wolters ----------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOTRFTRAIN_INC
#define INOTRFTRAIN_INC

//---------------------------------------------------------------------------

namespace Ino {

class Trf3;

class Trf3Train
{
  short trfSz;
  short paramSz;

  Trf3 *trfLst;
  Trf3 *trfFstDerMat; // Matrix of Trf3 pointers
  Trf3 *trfSecDerMat; // Idem

public:
  Trf3Train(short trfCount, short paramCount);
  Trf3Train(const Trf3Train& cp);
  ~Trf3Train();

  Trf3Train& operator=(const Trf3Train& src);

  short getTrfCount() const { return trfSz; }
  short getParamCount() const { return paramSz; }

  void reset();


  void setTrf(short trfIdx, const Trf3& trf);
  
  void setTrfFstDer(short trfIdx, short paramIdx, const Trf3& trf);
  void resetTrfFstDer(short trfIdx, short paramIdx);

  void setTrfSecDer(short trfIdx, short paramIdx, const Trf3& trf);
  void resetTrfSecDer(short trfIdx, short paramIdx);

  void invert();

  void calcTrfTrain(Trf3& trf) const;
  bool calcTrfFstDerTrain(short paramIdx, Trf3& trf) const;
  bool calcTrfSecDerTrain(short paramIdx, Trf3& trf) const;

  bool calcTrfTrainUpto(short trfIdx, Trf3 &trf) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
