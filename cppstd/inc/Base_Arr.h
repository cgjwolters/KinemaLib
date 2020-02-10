/* ---------------------------------------------------------------------- */
/* --------------- Standard Int * Double Array Utilities ---------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* --------------- Copyright Inofor Hoek Aut BV 1997, C. Wolters -------- */
/* ---------------------------------------------------------------------- */

#ifndef BASE_ARR_INC
#define BASE_ARR_INC

#include "Basics.h"

namespace Ino
{

/* ---------------------------------------------------------------------- */
/* ------- Variable Size Array Class for Ints --------------------------- */
/* ---------------------------------------------------------------------- */

class IB_Int_Arr
{
  int *arr;
  unsigned int asz;

  IB_Int_Arr(const IB_Int_Arr& cp);              // No Copying
  IB_Int_Arr& operator=(const IB_Int_Arr& src);  // No Assignment

 public:
  IB_Int_Arr() : arr(NULL), asz(0) {}
  IB_Int_Arr(unsigned int sz);
  ~IB_Int_Arr();

  unsigned int Size() const { return asz; }
  void Size(unsigned int new_sz);

  operator const int *() const { return arr; }
  operator       int *()       { return arr; }

  const int& operator[](int idx) const { return arr[idx]; }
        int& operator[](int idx)       { return arr[idx]; }

  const int* operator+(int idx) const { return arr + idx; }
        int* operator+(int idx)       { return arr + idx; }
};

/* ---------------------------------------------------------------------- */
/* ------- Variable Size Array Class for Doubles ------------------------ */
/* ---------------------------------------------------------------------- */

class IB_Dbl_Arr
{
  double *arr;
  unsigned int asz;

  IB_Dbl_Arr(const IB_Dbl_Arr& cp);              // No Copying
  IB_Dbl_Arr& operator=(const IB_Dbl_Arr& src);  // No Assignment

 public:
  IB_Dbl_Arr() : arr(NULL), asz(0) {}
  IB_Dbl_Arr(unsigned int sz);
  ~IB_Dbl_Arr();

  unsigned int Size() const { return asz; }
  void Size(unsigned int new_sz);

  operator const double *() const { return arr; }
  operator       double *()       { return arr; }

  const double& operator[](int idx) const { return arr[idx]; }
        double& operator[](int idx)       { return arr[idx]; }

  const double* operator+(int idx) const { return arr + idx; }
        double* operator+(int idx)       { return arr + idx; }
};

/* ---------------------------------------------------------------------- */
/* ------- Variable Size Array Class for String[80] --------------------- */
/* ---------------------------------------------------------------------- */

class IB_Str80_Arr
{
 public:
  typedef char String[81];

 private:
  String *arr;
  unsigned int asz;

  IB_Str80_Arr(const IB_Str80_Arr& cp);              // No Copying
  IB_Str80_Arr& operator=(const IB_Str80_Arr& src);  // No Assignment

 public:
  IB_Str80_Arr() : arr(NULL), asz(0) {}
  IB_Str80_Arr(unsigned int sz);
  ~IB_Str80_Arr();

  unsigned int Size() const { return asz; }
  void Size(unsigned int new_sz);

  operator const String *() const { return arr; }
  operator       String *()       { return arr; }

  const char * operator[](int idx) const { return arr[idx]; }
        char * operator[](int idx)       { return arr[idx]; }

  const String * operator+(int idx) const { return arr + idx; }
        String * operator+(int idx)       { return arr + idx; }
};

/* ---------------------------------------------------------------------- */
/* ------- Integer Array Utilities -------------------------------------- */
/* ---------------------------------------------------------------------- */

extern void IB_Sort_Asc(int arr[], unsigned int sz);

extern unsigned int IB_B_Search_Fst(const int arr[], unsigned int sz, int val);
extern unsigned int IB_B_Search_Fst_Ge(const int arr[], unsigned int sz, int val);
extern unsigned int IB_B_Search_Fst_Gt(const int arr[], unsigned int sz, int val);

extern bool IB_Insert_Asc(int arr[], unsigned int& sz, int newval);
extern bool IB_Insert(int arr[], unsigned int& sz, unsigned int inspos, int newval);

extern bool IB_Remove(int arr[], unsigned int &sz, unsigned int delpos);

extern void IB_Reverse(int arr[], unsigned int sz);

/* ---------------------------------------------------------------------- */
/* ------- Double Array Utilities --------------------------------------- */
/* ---------------------------------------------------------------------- */

extern void IB_Sort_Asc(double arr[], unsigned int sz);

extern unsigned int IB_B_Search_Fst(const double arr[], unsigned int sz,
                                                  double val, double tol);
extern unsigned int IB_B_Search_Fst_Ge(const double arr[], unsigned int sz,
                                                  double val, double tol);
extern unsigned int IB_B_Search_Fst_Gt(const double arr[], unsigned int sz,
                                                  double val, double tol);

extern bool IB_Insert_Asc(double arr[], unsigned int& sz, double newval, double tol);
extern bool IB_Insert(double arr[], unsigned int& sz, unsigned int inspos, double newval);

extern bool IB_Remove(double arr[], unsigned int &sz, unsigned int delpos);

extern void IB_Reverse(double arr[], unsigned int sz);

/* ---------------------------------------------------------------------- */
/* ------- String80 Array Utilities ------------------------------------- */
/* ---------------------------------------------------------------------- */

extern void IB_Sort_Asc(char arr[][81], unsigned int sz);

extern unsigned int IB_B_Search_Fst(const char arr[][81], unsigned int sz, const char *s);
extern unsigned int IB_B_Search_Fst_Ge(const char arr[][81], unsigned int sz, const char *s);
extern unsigned int IB_B_Search_Fst_Gt(const double arr[][81], unsigned int sz, const char *s);

extern bool IB_Insert_Asc(char arr[][81], unsigned int& sz, const char *ins);
extern bool IB_Insert(char arr[][81], unsigned int& sz, unsigned int inspos, const char *ins);

extern bool IB_Remove(char arr[][81], unsigned int &sz, unsigned int delpos);

extern void IB_Reverse(char arr[][81], unsigned int sz);

} // namespace Ino

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
#endif
