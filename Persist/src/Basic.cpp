//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Exceptions.h"
#include "Type.h"
#include "Writer.h"

namespace InoPersist
{

using namespace Ino;

//---------------------------------------------------------------------------

const char* Basic::getHashName()
{
#ifndef WIN32
  return getInfo().name();
#else
  return getInfo().raw_name();
#endif
}

//---------------------------------------------------------------------------

int Basic::getDataSize() const
{
  switch (dataType) {
    case Boolean:    return 1;
    case WChar:      return 2;
    case Byte:       return 1;
    case Short:      return 2;
    case Integer:    return 4;
    case Long:       return 8;
    case Float:      return 4;
    case Double:     return 8;
    case LongDouble: throw OperationNotSupportedException(
                       "Type::getDataSize: long double not yet suppported");
    case String:     return 4;

    default:         throw IllegalStateException("Type::getDataSize");
  }
}

//---------------------------------------------------------------------------

const type_info& Basic::getInfo() const
{
  switch (dataType) {
    case Boolean:    return typeid(bool);
    case WChar:      return typeid(wchar_t);
    case Byte:       return typeid(char);
    case Short:      return typeid(short);
    case Integer:    return typeid(long);
    case Long:       return typeid(__int64);
    case Float:      return typeid(float);
    case Double:     return typeid(double);
//    case LongDouble: return typeid(bool);
    case LongDouble: throw IllegalStateException("Long Double not yet supported");
    case String:     return typeid(wchar_t *);

    default:         throw IllegalStateException("Basic::getInfo");
  }
}

//---------------------------------------------------------------------------

void Basic::write(DataWriter& dWrt) const
{
  dWrt.writeByte(Record_Type,"Basic::write");

  dWrt.writeShort((short)dataType,"Basic::write");
}

} // namespace InoPersist

//---------------------------------------------------------------------------
