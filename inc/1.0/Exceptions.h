//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Standard Exceptions -----------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Jul 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOEXCEPTIONS_INC
#define INOEXCEPTIONS_INC

#include <stdexcept>

namespace Ino
{

using namespace std;

//---------------------------------------------------------------------------
// All exceptions below are derived from logic_error
// (directly or indirectly), except OutOfmemoryException.
//---------------------------------------------------------------------------

class OutOfMemoryException : public runtime_error
{
  public:
    OutOfMemoryException(const string& s) : runtime_error(s) {}
};

//---------------------------------------------------------------------------

// Exceptions that are programming errors

class NullPointerException : public invalid_argument
{
  public:
    NullPointerException(const string& s) : invalid_argument(s) {}
};

//---------------------------------------------------------------------------

class IndexOutOfBoundsException : public out_of_range
{
  public:
    explicit IndexOutOfBoundsException(const string& s) : out_of_range(s) {}
};

//---------------------------------------------------------------------------

class IllegalArgumentException : public invalid_argument
{
  public:
    explicit IllegalArgumentException(const string& s) : invalid_argument(s) {}
};

//---------------------------------------------------------------------------

class IllegalStateException : public domain_error
{
  public:
    explicit IllegalStateException(const string& s) : domain_error(s) {}
};

//---------------------------------------------------------------------------

class OperationNotSupportedException : public logic_error
{
  public:
    explicit OperationNotSupportedException(const string& s) : logic_error(s) {}
};

//---------------------------------------------------------------------------

class WrongTypeException : public logic_error
{
  public:
    explicit WrongTypeException(const string& s) : logic_error(s) {}
};

//---------------------------------------------------------------------------

// Application specific exceptions
// Mostly user induced
// Are normally caught and handled'

class NoSuchElementException : public logic_error
{
  public:
    explicit NoSuchElementException(const string& s) : logic_error(s) {}
};
 
//---------------------------------------------------------------------------

class DuplicateNameException : public logic_error
{
  public:
    explicit DuplicateNameException(const string& s) : logic_error(s) {}
};
 
//---------------------------------------------------------------------------

class InterruptedException : public logic_error
{
  public:
    explicit InterruptedException(const string& s) : logic_error(s) {}
}; 

//---------------------------------------------------------------------------

class IOException : public logic_error
{
  public:
    explicit IOException(const string& s) : logic_error(s) {}
}; 

//---------------------------------------------------------------------------

class IllegalFormatException : public IOException
{
  public:
    explicit IllegalFormatException(const string& s) : IOException(s) {}
};

//---------------------------------------------------------------------------

class NumberFormatException : public IOException
{
  public:
    explicit NumberFormatException(const string& s) : IOException(s) {}
};

//---------------------------------------------------------------------------

class FileFormatException : public IOException
{
  public:
    explicit FileFormatException(const string& s) : IOException(s) {}
};
 
//---------------------------------------------------------------------------

class StreamCorruptedException : public IOException
{
  public:
    explicit StreamCorruptedException(const string& s) : IOException(s) {}
};
 
//---------------------------------------------------------------------------

class StreamClosedException : public IOException
{
  public:
    explicit StreamClosedException(const string& s) : IOException(s) {}
};
 
//---------------------------------------------------------------------------

class StreamAbortedException : public IOException
{
  public:
    explicit StreamAbortedException(const string& s) : IOException(s) {}
};

//---------------------------------------------------------------------------

class FileNotFoundException : public IOException
{
  public:
    explicit FileNotFoundException(const string& s) : IOException(s) {}
};

//---------------------------------------------------------------------------

class AccessDeniedException : public IOException
{
  public:
    explicit AccessDeniedException(const string& s) : IOException(s) {}
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
