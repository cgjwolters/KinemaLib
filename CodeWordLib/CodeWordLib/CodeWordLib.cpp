//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C-Interface to License Codeword Decoder ---------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// CodeWordLib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CodeWordLib.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#include "CodeWordLib.h"
#include "CNagW.h"

//---------------------------------------------------------------------------
/** \file CodeWordLib.h
  C language interface to the Prodim codeword decode library.

  This version now has <em>hex</em> licensekey strings instead of the old
  base64 strings.\n
  Be aware that all string arguments must now be longer!\n
  \n
  This version (1.08) now supports \e permanent license keys.\n
  See \ref decCwIsPermanent() for details.\n
  \n
  The functions in the DLL no longer have decorated names.\n
  \n
  Link against this DecCw library and additionally the Microsoft
  library iphlpapi.lib.\n
  \n
  See the documentaion of function
  \ref decCwInit to get started.

  \author C. Wolters
  \date Mar 2007
  \version 1.0.8
*/

//---------------------------------------------------------------------------

/** \mainpage DecCw Decode LicenseKey library.
   This C language library can be used to protect applications with
   a license key.\n
   \n
   This is version 1.0.8 which now has hex license key strings instead of the
   old base64 strings.\n
   A new licensekey program has been implemented to support the new format.\n
   \n
   This version now supports \e permanent license keys.\n
   See \ref decCwIsPermanent() for details.\n
   \n
   The functions in the DLL no longer have decorated names.\n
   \n
   Include header file CodeWordLib.h and link against library CodeWordLib.lib.\n
   In addition Microsoft's library Iphlpapi.lib must also \b possibly be linked
   against in order to be able to get the MAC address of netwerk interface
   cards.\n
   \n
   The library is intended for use in a \b multi-thread environment.
   \n
   See the documentaion of function
   \ref decCwInit to get started.

   \author C. Wolters
   \date Mar 2007
   \version 1.0.8
*/

//---------------------------------------------------------------------------

static AW::CNagW *cNagW = NULL;

//---------------------------------------------------------------------------
/** Initializes a codeword decode structure.

    This is the first function to call.\n
    \param appId The application identifier\n
    Currently:
    \li 0: ProlinerCalc (Qt version)
    \li 1: LeapFrog application (by Nils Haeck)
    \li 2: ProlinerUnfold (by Nils Haeck)
    \param cw The codeword to decode, must be exactly 16 bytes long
    (with an additional trailing zero byte).
    \param tm An encrypted date string, must be exactly 12 bytes long
    (with an additional trailing zero byte).\n
    This parameter may be \c NULL, but see the note below.
    \param major The major version number of the application.
    \param minor The minor version number of the application.
    \return \c true if the codeword could be decrypted, see
    \ref decCwIsValid().
    The codeword is (DES) decrypted using the MAC address of the target
    computer as a key.\n
    \n
    Once decrypted the codeword contains:
    \li The application Id (1 byte)
    \li The ultimate valid date (2 bytes)
    \li 16 feature bits, that be used to selectively enable program
    features.
    \li The maximum legal major version number (4 bits)
    \li The maximum legal minor version number (4 bits)

    \note
    The purpose of parameter \c tm is to fend off users that set the
    clock of their computer back.\n
    Call function \ref getTmNow on every
    invocation of the application and write the resulting string to
    the registry.\n
    Then on the next invocation pass the stored string in parameter
    \c tm. This function will then use the date stored in \c tm or
    the current date, whichever is later.

    \note Do not forget to call \ref decCwFree() once you are done!
*/

CODEWORDLIB_API bool decCwInit(unsigned char appId, const char cw[17],
                               const char tm[13], unsigned char major,
                               unsigned char minor)
{
  if (cNagW) delete cNagW;

  cNagW = new AW::CNagW(appId,cw,tm,major,minor);

  return decCwIsValid();
}

//---------------------------------------------------------------------------
/** Checks to see if the supplied codeword could be decrypted.

    A codeword being valid does \b NOT imply that the application
    should be allowed to run!
    \return \c true if the codeword is valid,\c false otherwise.\n
    Returns \c false if \ref decCwInit was not first called.

    A codeword is valid if:
    \li It can be decrypted properly, i.e. the computers MAC-address
    has the expected value.
    \li The decrypted appId is equal to the value supplied to
    function \ref decCwInit.
*/

CODEWORDLIB_API bool decCwIsValid()
{
  if (!cNagW) return false;

  return cNagW->isValid();
}

//---------------------------------------------------------------------------
/** Checks to see if the supplied codeword has expired.

    <b>Use this function to test if the application can be allowed to run</b>.\n
    \n
    A codeword is expired if the current date is past the date stored in
    the codeword or if the application supplied a higher major/minor version
    number to \ref decCwInit than is stored in the codeword.

    \return \c true if the codeword is not \ref decCwIsValid() "valid"
    or has expired, \c false otherwise.
*/

CODEWORDLIB_API bool decCwExpired()
{
  if (!cNagW) return false;

  return cNagW->expired();
}

//---------------------------------------------------------------------------
/** Returns the <em>feature bits</em> contained in the codeword.

    \return the feature bits or an undefined value if the codeword is not
    \ref decCwIsValid() "valid".
*/

CODEWORDLIB_API unsigned short decCwGetFeats()
{
  if (!cNagW) return 0;

  return cNagW->getFeats();
}

//---------------------------------------------------------------------------
/** Returns whether the licenseley is permanent.

   \note Always call this function before calling decCwGetDaysLeft().

   \return \c true if the codeword never expires.
*/

CODEWORDLIB_API bool decCwIsPermanent()
{
  if (!cNagW) return false;

  return cNagW->isPermanent();
}

//---------------------------------------------------------------------------
/** Returns the number of days left until the codeword expires.

    \note Always call function decCwIsPermanent() before calling
    this function.

    \return the number of days until the codeword expires or zero
    if the codeword is not \ref decCwIsValid() "valid".\n
    \n
    If decCwIsPermanent() returns \c true then this function will
    return the value <tt>0x7FFFFFFF</tt>.
*/

CODEWORDLIB_API long decCwGetDaysLeft()
{
  if (!cNagW) return 0;

  return cNagW->getDaysLeft();
}

//---------------------------------------------------------------------------
/** Returns the ultimate major version number as stored in the codeword.

    \return the stored major version number or an undefined value
    if the codeword is not \ref decCwIsValid() "valid".
*/

CODEWORDLIB_API unsigned char decCwGetMajor()
{
  if (!cNagW) return 0;

  return cNagW->getMajor();
}

//---------------------------------------------------------------------------
/** Returns the ultimate minor version number as stored in the codeword.

    \return the stored minor version number or an undefined value
    if the codeword is not \ref decCwIsValid() "valid".
*/

CODEWORDLIB_API unsigned char decCwGetMinor()
{
  if (!cNagW) return 0;

  return cNagW->getMinor();
}

//---------------------------------------------------------------------------
/** Returns the machine id of the local computer.

   The machine id is a mangled version of the MAC-address of the first
   network interface card found in the computer.\n
   The mangling ensures that the string only contains readable ASCII
   characters.\n
   \n
   The machine id is needed to generate a new codeword using the
   special codeword generating program. 

   \param id The string that will hold the machineid, must be at least
   17 bytes long. A trailing zero byte is written as well.
   \return \c true if the operation succeeded, returns \c false if and
   only if <tt>id == NULL</tt>

   \note
   If the local computers MAC-address cannot be found then a
   generic value is returned instead.
*/

CODEWORDLIB_API bool getMachineId(char id[17])
{
  return AW::CNagW::getMachineId(id);
}

//---------------------------------------------------------------------------
/** Returns a mangled version of the current date as reported by the
   local computer.

   Use this function to generate a data string on each invocation of the
   application and store the returned string in the computers registry.\n
   \n
   On a next invocation of the application get the string from the registry
   and supply it to function \ref decCwInit.\n
   This will fend off users that have set their computers clock back.

   \param tmNow This buffer will receive the mangled date string, must
   be at least 13 bytes long. A trailing zero will be written to the buffer.
   \return \c true if the operation succeeded, \c false if and only if
   parameter <tt>tmNow == NULL</tt>.
*/

CODEWORDLIB_API bool getTmNow(char tmNow[13])
{
  return AW::CNagW::getTmNow(tmNow);
}

//---------------------------------------------------------------------------
/** Frees all data allocated by this module.

    Do not forget to call, once you are done!
*/

CODEWORDLIB_API void decCwFree()
{
  if (cNagW) delete cNagW;
  cNagW = NULL;
}

//---------------------------------------------------------------------------
