/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* --------------- Standard Int * Double Array Utilities ---------------- */
/* ---------------------------------------------------------------------- */
/* ------------- Copyright Inofor Hoek Aut BV 1997, C. Wolters ---------- */
/* ---------------------------------------------------------------------- */


#include "Base_Arr.h"

#include <cstring>
#include <cstdlib>
#include <cmath>

namespace Ino
{

// using namespace std;

/* ---------------------------------------------------------------------- */
/* ------- IB_Int_Arr --------------------------------------------------- */
/* ---------------------------------------------------------------------- */

IB_Int_Arr::IB_Int_Arr(unsigned int sz)
 : arr(NULL), asz(0)
{
  if (sz > 0) {
    arr = new int[sz];
    if (arr) asz = sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

IB_Int_Arr::~IB_Int_Arr()
{
  if (arr) delete[] arr;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Int_Arr::Size(unsigned int new_sz)
{
  if (new_sz > asz || new_sz < asz/2) {
    int *hold = arr; arr = NULL;

    if (new_sz) {
      arr = new int[new_sz];

      unsigned int cpsz = asz; if (new_sz < asz) cpsz = new_sz;

      if (hold) memcpy(arr,hold,cpsz*sizeof(int));
    }

    if (hold) delete[] hold;

    asz = new_sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- IB_Dbl_Arr ------------------------------------------------------ */
/* ---------------------------------------------------------------------- */

IB_Dbl_Arr::IB_Dbl_Arr(unsigned int sz)
  : arr(NULL), asz(0)
{
  if (sz > 0) {
     arr = new double[sz];
     if (arr) asz = sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

IB_Dbl_Arr::~IB_Dbl_Arr()
{
  if (arr) delete[] arr;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Dbl_Arr::Size(unsigned int new_sz)
{
  if (new_sz > asz || new_sz < asz/2) {
    double *hold = arr; arr = NULL;

    if (new_sz) {
      arr = new double[new_sz];

      unsigned int cpsz = asz; if (new_sz < asz) cpsz = new_sz;

      if (hold) memcpy(arr,hold,cpsz*sizeof(double));
    }

    if (hold) delete[] hold;

    asz = new_sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- IB_Str80_Arr ------------------------------------------------- */
/* ---------------------------------------------------------------------- */

IB_Str80_Arr::IB_Str80_Arr(unsigned int sz)
  : arr(NULL), asz(0)
{
  if (sz > 0) {
    arr = new String[sz];
    if (arr) asz = sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

IB_Str80_Arr::~IB_Str80_Arr()
{
  if (arr) delete[] arr;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Str80_Arr::Size(unsigned int new_sz)
{
  if (new_sz > asz || new_sz < asz/2) {
    String *hold = arr; arr = NULL;

    if (new_sz) {
      arr = new char[new_sz][81];

      unsigned int cpsz = asz; if (new_sz < asz) cpsz = new_sz;

      if (hold) memcpy(arr,hold,cpsz*sizeof(String));
    }

    if (hold) delete[] hold;

    asz = new_sz;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Integer Array Utilities -------------------------------------- */
/* ---------------------------------------------------------------------- */

static int intcmpasc(const void *i1, const void *i2)
{
  const int *ii1 = (const int *)i1;
  const int *ii2 = (const int *)i2;

       if (*ii1 < *ii2) return -1;
  else if (*ii1 > *ii2) return  1;

  return 0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Sort_Asc(int arr[], unsigned int sz)
{
  if (arr && sz > 0) qsort(arr, sz, sizeof(int), intcmpasc);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst(const int arr[], unsigned int sz, int val)
{
  unsigned int idx = IB_B_Search_Fst_Ge(arr,sz,val);

  if (idx < sz && arr[idx] != val) return sz;

  return idx;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Ge(const int arr[], unsigned int sz, int val)
{
  if (!arr) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (arr[idx] < val) lwb = idx+1;
    else                upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Gt(const int arr[], unsigned int sz, int val)
{
  if (!arr) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (arr[idx] <= val) lwb = idx+1;
    else                 upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert_Asc(int arr[], unsigned int& sz, int newval)
{
  if (!arr) return false;

  unsigned int inspos = IB_B_Search_Fst_Ge(arr,sz,newval);

  if (inspos < sz) {
    if (arr[inspos] == newval) return false;
    memmove(arr+inspos+1,arr+inspos,(sz-inspos)*sizeof(int));
  }

  arr[inspos] = newval;

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert(int arr[], unsigned int& sz, unsigned int inspos, int newval)
{
  if (!arr || inspos > sz) return false;

  if (inspos < sz) memmove(arr+inspos+1,arr+inspos,(sz-inspos)*sizeof(int));

  arr[inspos] = newval;

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Remove(int arr[], unsigned int &sz, unsigned int delpos)
{
  if (!arr || delpos >= sz) return false;

  memmove(arr+delpos,arr+delpos+1,(sz-delpos-1)*sizeof(int));

  sz--;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Reverse(int arr[], unsigned int sz)
{
  if (!arr) return;

  unsigned int lwb = 0, upb = sz-1;

  while (lwb < upb) {
    int hold = arr[lwb]; arr[lwb] = arr[upb]; arr[upb] = hold;
    lwb++;
    upb--;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- Double Array Utilities --------------------------------------- */
/* ---------------------------------------------------------------------- */

static int dblcmpasc(const void *d1, const void *d2)
{
  const double *dd1 = (const double *)d1;
  const double *dd2 = (const double *)d2;

       if (*dd1 < *dd2) return -1;
  else if (*dd1 > *dd2) return  1;

  return 0;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Sort_Asc(double arr[], unsigned int sz)
{
  if (arr && sz > 0) qsort(arr, sz, sizeof(double), dblcmpasc);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst(const double arr[], unsigned int sz,
                                                double val, double tol)
{
  unsigned int idx = IB_B_Search_Fst_Ge(arr,sz,val,tol);

  if (idx < sz && fabs(arr[idx]-val) > tol) return sz;

  return idx;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Ge(const double arr[], unsigned int sz,
                                                 double val, double tol)
{
  if (!arr) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (arr[idx] < val-tol) lwb = idx+1;
    else                    upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Gt(const double arr[], unsigned int sz,
                                                 double val, double tol)
{
  if (!arr) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (arr[idx] < val+tol) lwb = idx+1;
    else                    upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert_Asc(double arr[], unsigned int& sz, double newval, double tol)
{
  if (!arr) return false;

  unsigned int inspos = IB_B_Search_Fst_Ge(arr,sz,newval,tol);

  if (inspos < sz) {
    if (fabs(arr[inspos] - newval) < tol) return false;

    memmove(arr+inspos+1,arr+inspos,(sz-inspos)*sizeof(double));
  }

  arr[inspos] = newval;

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert(double arr[], unsigned int& sz, unsigned int inspos, double newval)
{
  if (!arr || inspos > sz) return false;

  if (inspos < sz) memmove(arr+inspos+1,arr+inspos,
                                               (sz-inspos)*sizeof(double));

  arr[inspos] = newval;

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Remove(double arr[], unsigned int &sz, unsigned int delpos)
{
  if (!arr || delpos >= sz) return false;

  memmove(arr+delpos,arr+delpos+1,(sz-delpos-1)*sizeof(double));

  sz--;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Reverse(double arr[], unsigned int sz)
{
  if (!arr) return;

  unsigned int lwb = 0, upb = sz-1;

  while (lwb < upb) {
    double hold = arr[lwb]; arr[lwb] = arr[upb]; arr[upb] = hold;
    lwb++;
    upb--;
  }
}

/* ---------------------------------------------------------------------- */
/* ------- String80 Array Utilities ------------------------------------- */
/* ---------------------------------------------------------------------- */

#ifdef NEVER
void IB_Sort_Asc(char arr[][81], unsigned int sz)
{
  if (arr && sz > 0) qsort(arr, sz, sizeof(double), strcmp);
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst(const char arr[][81], unsigned int sz, const char *s)
{
  unsigned int idx = IB_B_Search_Fst_Ge(arr,sz,s);

  if (idx < sz && !strcmp(arr[idx],s)) return sz;

  return idx;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Ge(const char arr[][81], unsigned int sz,
                                                          const char *s)
{
  if (!arr || !s) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (strcmp(arr[idx],s) < 0) lwb = idx+1;
    else                        upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

unsigned int IB_B_Search_Fst_Gt(const char arr[][81], unsigned int sz,
                                                          const char *s)
{
  if (!arr || !s) return sz;

  int idx, lwb = 0, upb = sz-1;

  while (lwb <= upb) {
    idx = (lwb+upb)/2;

    if (strcmp(arr[idx],s) <= 0) lwb = idx+1;
    else                         upb = idx-1;
  }

  return ++upb;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert_Asc(char arr[][81], unsigned int& sz, const char *ins)
{
  if (!arr || !ins || strlen(ins) > 80) return false;

  unsigned int inspos = IB_B_Search_Fst_Ge(arr,sz,ins);

  if (inspos < sz) {
    if (!strcmp(arr[inspos],ins)) return false;

    memmove(arr+inspos+1,arr+inspos,(sz-inspos)*sizeof(char[81]));
  }

  strcpy(arr[inspos],ins);

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Insert(char arr[][81], unsigned int& sz, unsigned int inspos,
                                                       const char *ins)
{
  if (!arr || inspos > sz || !ins || strlen(ins) > 80) return false;

  if (inspos < sz) memmove(arr+inspos+1,arr+inspos,
                                        (sz-inspos)*sizeof(char[81]));

  strcpy(arr[inspos],ins);

  sz++;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

bool IB_Remove(char arr[][81], unsigned int &sz, unsigned int delpos)
{
  if (!arr || delpos >= sz) return false;

  memmove(arr+delpos,arr+delpos+1,(sz-delpos-1)*sizeof(char[81]));

  sz--;

  return true;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void IB_Reverse(char arr[][81], unsigned int sz)
{
  if (!arr) return;

  unsigned int lwb = 0, upb = sz-1;

  while (lwb < upb) {
    char hold[81];
    strcpy(hold,arr[lwb]);
    strcpy(arr[lwb],arr[upb]);
    strcpy(arr[upb],hold);
    lwb++;
    upb--;
  }
}

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
