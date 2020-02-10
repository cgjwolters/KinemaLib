//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- C-Interface to License Codeword Encoder ---------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "EncCwLib.h"

#include "WNagC.h"
#include "CNagW.h"

//---------------------------------------------------------------------------
/** \file EncCwLib.h
  C language interface to the Prodim codeword encoder library.

  This version now has <em>hex</em> licensekey strings instead of the old
  base64 strings.\n
  Be aware that all string arguments must now be longer!\n
  \n
  The functions in the DLL no longer have decorated names.\n
  \n
  Link against this EncCw library and additionally the Microsoft
  library iphlpapi.lib.\n
  \n
  See the documentaion of function \ref encCw to get started.

  \author C. Wolters
  \date April 2006
*/

//---------------------------------------------------------------------------
/** \mainpage EncCw LicenseKey Encode library.
   This C language library can be used to genereate a license key.\n
   \n
   This version now has <em>hex</em> licensekey strings instead of the old
   base64 strings.\n
   Be aware that all string arguments must now be longer!\n
   \n
   The functions in the DLL no longer have decorated names.\n
   \n
   Include header file EncCwLib.h and link against library EncCw.lib.\n
   In addition Microsoft's library Iphlpapi.lib must also possibly be linked
   against in order to be able to get the MAC address of netwerk interface
   cards.\n
   \n
   The library is intended for use in a \b multi-thread environment.
   \n
   See the documentation of function \ref encCw to get started.

   \author C. Wolters
   \date April 2006
*/

//---------------------------------------------------------------------------

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved)
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

//---------------------------------------------------------------------------
/** Returns the current day number.

    To generate, for axample, a codeword that will expire in 14 days,
    add 14 to this number and supply that value to function \ref encCw. 
    \return The number of today.

    This value is the number of seconds since Jan. 1 1970 divided by 84600,\n
    (i.e. the number of days since Jan 1 1970).
*/

ENCCWLIB_API long getToday()
{
  return (long)(_time64(NULL)/86400);
}

//---------------------------------------------------------------------------
/** Returns a mangled version of the current date as reported by the
   local computer.

   Use this function to generate a data string to be stored in the
   computers registry as a precaution against users that set the
   clock back.\n

   \param tmNow This buffer will receive the mangled date string, must
   be at least 13 bytes long. A trailing zero will be written to the buffer.
   \return \c true if the operation succeeded, \c false if and only if
   parameter <tt>tmNow == NULL</tt>.
*/

ENCCWLIB_API bool getTmNow(char tmNow[13])
{
  return AW::CNagW::getTmNow(tmNow);
}

//---------------------------------------------------------------------------
/** Returns the machine id of the local computer.

   The machine id is a mangled version of the MAC-address of the first
   network interface card found in the computer.\n
   The mangling ensures that the string only contains readable ASCII
   characters.\n
   \n
   The machine id is needed to generate a new codeword using function
   \ref encCw. 

   \param mchId The string that will hold the machineid, must be at least
   17 bytes long. A trailing zero byte is written as well.
   \return \c true if the operation succeeded, returns \c false if and
   only if <tt>id == NULL</tt>

   \note
   If the local computers MAC-address cannot be found then a
   generic value is returned instead.
*/

ENCCWLIB_API bool getMachineId(char mchId[17])
{
  return AW::CNagW::getMachineId(mchId);
}

//---------------------------------------------------------------------------
/** Generates a new codeword (licensekey).

    \param appId The application identifier\n
    Currently:
    \li 0: ProlinerCalc (Qt version)
    \li 1: LeapFrog application (by Nils Haeck)
    \li 2: ProlinerUnfold (by Nils Haeck)
    \param mchId The mangled machine id of the target computer.\n
    See \ref getMachineId().
    \param limitDay The day the new codeword will expire.\n
    Use for example <tt>\ref getToday()+14</tt> to generate a
    codeword that will expire in 14 days.
    \param features The (16) feature bits to store in the codeword.
    \param maxMajor The maximum valid major version number (0..15).
    \param maxMinor The maximum valid minor version number (0..15).
    \param cw The buffer to receive the new codeword, must not be
    \c NULL.\n
    The buffer must have a capacity of at least 12 bytes.
    \return \c true if a codeword has been stored in the buffer
    pointed to by parameter \c cw.\n
    \c false if parameter <tt>cw == NULL</tt>.

    \note
    Only the lower 4 bits of parameters \c maxMajor and \c maxMinor are
    stored in the codeword.
*/

ENCCWLIB_API bool encCw(unsigned char appId, const char mchId[17],
                        long limitDay, unsigned short features,
                        unsigned char maxMajor, unsigned char maxMinor,
                        char cw[17])
{
  if (!cw) return false;

  AW::WNagC wNag(appId,mchId,limitDay*86400,features,maxMajor,maxMinor);

  const char *cwStr = wNag.getCodeWord();
  if (!cwStr) return false;

  for (int i=0; i<17; ++i) cw[i] = cwStr[i];

  return true;
}

//---------------------------------------------------------------------------
