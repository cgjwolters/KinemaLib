//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------------- Transform Matrix Train Manager -----------------------------
//---------------------------------------------------------------------------
//------------------------ Copyright Inofor Hoek Aut BV 2004 ----------------
//---------------------------------------------------------------------------
//------------------------------------------------------ C.Wolters ----------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "TrfTrain.h"

#include "Trf.h"
#include "Exceptions.h"

namespace Ino
{

//---------------------------------------------------------------------------

Trf3Train::Trf3Train(short trfCount, short paramCount)
: trfSz(trfCount), paramSz(paramCount),
  trfLst(new Trf3[trfSz]),
  trfFstDerMat(new Trf3[trfSz*paramSz]),
  trfSecDerMat(new Trf3[trfSz*paramSz])
{
  reset();
}

//---------------------------------------------------------------------------

Trf3Train::Trf3Train(const Trf3Train& cp)
: trfSz(cp.trfSz), paramSz(cp.paramSz),
  trfLst(new Trf3[trfSz]),
  trfFstDerMat(new Trf3[trfSz*paramSz]),
  trfSecDerMat(new Trf3[trfSz*paramSz])
{
  for (int i=0; i<trfSz; i++) {
    trfLst[i] = cp.trfLst[i];
    trfLst[i].isDerivative = false;
  }
  
  long sz = trfSz * paramSz;

  for (int i=0; i<sz; i++) {
    if (cp.trfFstDerMat[i].isDerivative) trfFstDerMat[i] = cp.trfFstDerMat[i];
    else trfFstDerMat[i].isDerivative = false;

    if (cp.trfSecDerMat[i].isDerivative) trfSecDerMat[i] = cp.trfSecDerMat[i];
    else trfSecDerMat[i].isDerivative = false;
  }
}

//---------------------------------------------------------------------------

Trf3Train::~Trf3Train()
{
  delete[] trfLst;
  delete[] trfFstDerMat;
  delete[] trfSecDerMat;
}

//---------------------------------------------------------------------------

Trf3Train& Trf3Train::operator=(const Trf3Train& src)
{
  long sz = trfSz * paramSz;

  if (trfSz != src.trfSz) {
    delete[] trfLst;

    trfSz = src.trfSz;
    trfLst = new Trf3[trfSz];
  }

  for (int i=0; i<trfSz; i++) {
    trfLst[i] = src.trfLst[i];
    trfLst[i].isDerivative = false;
  }
  
  paramSz = src.paramSz;

  if (sz != src.trfSz * src.paramSz) { // Must reallocate
    delete[] trfFstDerMat;
    delete[] trfSecDerMat;

    sz = trfSz * paramSz;

    trfFstDerMat = new Trf3[sz];
    trfSecDerMat = new Trf3[sz];
  }


  for (int i=0; i<sz; i++) {
    if (src.trfFstDerMat[i].isDerivative) trfFstDerMat[i] = src.trfFstDerMat[i];
    else trfFstDerMat[i].isDerivative = false;

    if (src.trfSecDerMat[i].isDerivative) trfSecDerMat[i] = src.trfSecDerMat[i];
    else trfSecDerMat[i].isDerivative = false;
  }

  return *this;
}

//---------------------------------------------------------------------------

void Trf3Train::reset()
{
  for (int i=0; i<trfSz; i++) trfLst[i].init();

  long sz = trfSz * paramSz;

  for (int i=0; i<sz; i++) {
    trfFstDerMat[i].zero();
    trfFstDerMat[i].isDerivative = false;
    trfSecDerMat[i].zero();
    trfSecDerMat[i].isDerivative = false;
  }
}

//---------------------------------------------------------------------------

void Trf3Train::setTrf(short trfIdx, const Trf3& trf)
{
  if (trfIdx < 0 || trfIdx >= trfSz)
               throw IndexOutOfBoundsException("Trf3Train::setTrf");

  trfLst[trfIdx] = trf;
  trfLst[trfIdx].isDerivative = false;
}

//---------------------------------------------------------------------------

void Trf3Train::setTrfFstDer(short trfIdx, short paramIdx, const Trf3& trf)
{
  if (trfIdx < 0 || trfIdx >= trfSz)
            throw IndexOutOfBoundsException("Trf3Train::setTrfFstDer 1");
  if (paramIdx < 0 || paramIdx >= paramSz)
            throw IndexOutOfBoundsException("Trf3Train::setTrfFstDer 2");

  long idx = paramIdx * trfSz + trfIdx;

  trfFstDerMat[idx] = trf;
  trfFstDerMat[idx].isDerivative = true;
}

//---------------------------------------------------------------------------

void Trf3Train::resetTrfFstDer(short trfIdx, short paramIdx)
{
  if (trfIdx < 0 || trfIdx >= trfSz)
            throw IndexOutOfBoundsException("Trf3Train::resetTrfFstDer 1");
  if (paramIdx < 0 || paramIdx >= paramSz)
            throw IndexOutOfBoundsException("Trf3Train::resetTrfFstDer 2");

  long idx = paramIdx * trfSz + trfIdx;

  trfFstDerMat[idx].isDerivative = false;
}

//---------------------------------------------------------------------------

void Trf3Train::setTrfSecDer(short trfIdx, short paramIdx, const Trf3& trf)
{
  if (trfIdx < 0 || trfIdx >= trfSz)
                throw IndexOutOfBoundsException("Trf3Train::setTrfSecDer 1");
  if (paramIdx < 0 || paramIdx >= paramSz)
                throw IndexOutOfBoundsException("Trf3Train::setTrfSecDer 2");

  long idx = paramIdx * trfSz + trfIdx;

  trfSecDerMat[idx] = trf;
  trfSecDerMat[idx].isDerivative = true;
}

//---------------------------------------------------------------------------

void Trf3Train::resetTrfSecDer(short trfIdx, short paramIdx)
{
  if (trfIdx < 0 || trfIdx >= trfSz)
                throw IndexOutOfBoundsException("Trf3Train::resetTrfSecDer");
  if (paramIdx < 0 || paramIdx >= paramSz)
                throw IndexOutOfBoundsException("Trf3Train::resetTrfSecDer");

  long idx = paramIdx * trfSz + trfIdx;

  trfSecDerMat[idx].isDerivative = false;
}

//---------------------------------------------------------------------------

static void addToTrf(Trf3& dst, const Trf3& src)
{
  for (int i=0; i<3; i++) {
    for (int j=0; j<4; j++) dst(i,j) += src(i,j);
  }
}

//---------------------------------------------------------------------------

static void multTrf(Trf3& dst, double fact)
{
  for (int i=0; i<3; i++) {
    for (int j=0; j<4; j++) dst(i,j) *= fact;
  }
}

//---------------------------------------------------------------------------

static void invertFstDer(Trf3& fstDer, const Trf3& invTrf)
{
  Trf3 t(invTrf);

  t *= fstDer; t *= invTrf;

  multTrf(t,-1.0);

  fstDer = t;
  fstDer.isDerivative = true; // To be sure
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void invertSecDer(Trf3& secDer, const Trf3& trf,
                         const Trf3& invTrf, const Trf3& invDer)
{
  Trf3 t1(invDer); t1 *= trf; t1 *= invDer; multTrf(t1,2.0);

  Trf3 t2(invTrf); t2 *= secDer; t2 *= invTrf; multTrf(t2,-1.0);

  secDer = t1; addToTrf(secDer,t2);
  secDer.isDerivative = true; // To be sure
}

//---------------------------------------------------------------------------

void Trf3Train::invert()
{
  short lwb = 0, upb = trfSz-1;

  while (lwb <= upb) {
    Trf3 hghTrf(trfLst[lwb]);
    Trf3 lowTrf(trfLst[upb]);

    if (lwb != upb) lowTrf.invertInto(trfLst[lwb]);

    hghTrf.invertInto(trfLst[upb]);

    for (short parIdx=0; parIdx<paramSz; parIdx++) {
      long lowIdx = parIdx * trfSz + lwb;
      long hghIdx = parIdx * trfSz + upb;

      if (lowIdx != hghIdx) {
        Trf3 lowDerTrf = trfFstDerMat[lowIdx];
        trfFstDerMat[lowIdx] = trfFstDerMat[hghIdx];
        trfFstDerMat[hghIdx] = lowDerTrf;

        lowDerTrf = trfSecDerMat[lowIdx];
        trfSecDerMat[lowIdx] = trfSecDerMat[hghIdx];
        trfSecDerMat[hghIdx] = lowDerTrf;
      }

      if (trfFstDerMat[lowIdx].isDerivative) {
        invertFstDer(trfFstDerMat[lowIdx],trfLst[lwb]);

        if (trfSecDerMat[lowIdx].isDerivative)
          invertSecDer(trfSecDerMat[lowIdx],lowTrf,trfLst[lwb],
                                                     trfFstDerMat[lowIdx]);
      }

      if (trfFstDerMat[hghIdx].isDerivative && lowIdx != hghIdx) {
        invertFstDer(trfFstDerMat[hghIdx],trfLst[upb]);

        if (trfSecDerMat[hghIdx].isDerivative)
          invertSecDer(trfSecDerMat[hghIdx],hghTrf,trfLst[upb],
                                                     trfFstDerMat[hghIdx]);
      }
    }

    lwb++; upb--;
  }
}

//---------------------------------------------------------------------------

void Trf3Train::calcTrfTrain(Trf3& trf) const
{
  trf.isDerivative = false;

  trf.init();

  for (short i=trfSz-1; i>=0; i--) trf *= trfLst[i];

  trf.isDerivative = false;
}

//---------------------------------------------------------------------------

bool Trf3Train::calcTrfFstDerTrain(short paramIdx, Trf3& trf) const
{
  if (paramIdx >= paramSz)
           throw IndexOutOfBoundsException("Trf3Train::calcTrfFstDerTrain");

  trf.zero();
  trf.isDerivative = true;

  Trf3 leftTrf;
  bool notZero = false;

  for (short i=trfSz-1; i>=0; i--) {
    if (notZero) trf *= trfLst[i];

    long idx = paramIdx * trfSz + i;

    if (trfFstDerMat[idx].isDerivative) {
      Trf3 derTrf(leftTrf);

      derTrf *= trfFstDerMat[idx];

      addToTrf(trf,derTrf);

      notZero = true;
    }

    leftTrf *= trfLst[i];
  }

  return notZero;
}

//---------------------------------------------------------------------------

bool Trf3Train::calcTrfSecDerTrain(short paramIdx, Trf3& trf) const
{
  if (paramIdx >= paramSz)
            throw IndexOutOfBoundsException("Trf3Train::calcTrfSecDerTrain");

  trf.zero();
  trf.isDerivative = true;

  Trf3 leftTrf;
  bool notZero = false;

  Trf3 leftDerTrf;
  leftDerTrf.isDerivative = true;
  bool notDerZero = false;

  for (short i=trfSz-1; i>=0; i--) {
    if (notZero) trf *= trfLst[i];

    long idx = paramIdx * trfSz + i;

    if (trfFstDerMat[idx].isDerivative) {
      if (notDerZero) {
        Trf3 derTrf(leftDerTrf);

        derTrf *= trfFstDerMat[idx];

        addToTrf(trf,derTrf);

        notZero = true;

        leftDerTrf *= trfLst[i];
      }

      Trf3 derTrf(leftTrf);
      derTrf *= trfFstDerMat[idx];
      multTrf(derTrf,2.0);

      addToTrf(leftDerTrf,derTrf);
      notDerZero = true;

      if (trfSecDerMat[idx].isDerivative) {
        Trf3 hTrf(leftTrf);

        hTrf *= trfSecDerMat[idx];

        addToTrf(trf,hTrf);

        notZero = true;
      }
    }

    leftTrf *= trfLst[i];
  }

  return notZero;
}

//---------------------------------------------------------------------------

bool Trf3Train::calcTrfTrainUpto(short trfIdx, Trf3 &trf) const
{
  if (trfIdx < 0 || trfIdx > trfSz) return false;

  trf.isDerivative = false;

  trf.init();

  for (short i=trfIdx-1; i>=0; i--) trf *= trfLst[i];

  trf.isDerivative = false;

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
