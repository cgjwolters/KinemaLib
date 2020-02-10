//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Retrieve Windows Pc's MAC address ---------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "StdAfx.h"

#include "Windows.h"
#include "MacAddress.h"
#include "Hex.h"
#include "DesCipher.h"

#include <stdlib.h>
#include <string.h>
#include <Iphlpapi.h>

namespace AW
{

//---------------------------------------------------------------------------

const __int64 MacAddress::generic = 0xFE0123456987;

//---------------------------------------------------------------------------

static void toUpper(char *s)
{
  if (!s) return;

  while (*s) {
    *s = (char)toupper(*s);
    ++s;
  }
}

//---------------------------------------------------------------------------

static char *dupStr(const char *s)
{
  if (!s) return NULL;

  char *buf = new char[strlen(s)+1];

  strcpy(buf,s);

  return buf;
}

static void cpKey(__int64 ma, char key[8])
{
  for (int i=5; i>=0; --i) {
    key[i] = (char)(ma & 0xff);
    ma >>= 8;
  }

  key[6] = 'a';
  key[7] = 'k';
}


//---------------------------------------------------------------------------

MacAddress::MacAddress(__int64 mac, const char *ip, const char *desc)
: macAddress(mac), ipAddress(dupStr(ip)), description(dupStr(desc))
{
  macAddressStr[0] = '\0';
}

//---------------------------------------------------------------------------

MacAddress::~MacAddress()
{
  if (ipAddress) delete[] ipAddress;
  if (description) delete[] description;
}

//---------------------------------------------------------------------------

const char *MacAddress::getHexEncryptedMacAddressString() const
{
  __int64 adr = macAddress;

  char rawAdr[8];
  
  cpKey(adr,rawAdr);

  AW::DesCipher des("H(7l1@8q");

  char encCw[8];
  des.encrypt(rawAdr,8,encCw);
  
  Hex hex;
  int codeLen = 0;

  const char *s = hex.encode(encCw,8,codeLen);
  if (!s) return NULL;

  strcpy(macAddressStr,s);

  return macAddressStr;
}

//---------------------------------------------------------------------------

bool MacAddress::decodeHexDecryptedMacStr(const char macStr[17], __int64& macAddr)
{
  if (!macStr || strlen(macStr) != 16) return false;

  macAddr = 0;

  Hex hex;
  int decodeLen = 0;
  const char *decStr = hex.decode(macStr,16,decodeLen);

  if (decodeLen != 8) return false;

  AW::DesCipher des("H(7l1@8q");
  
  char decDes[8];
  des.decrypt(decStr,8,decDes);
  
  for (int i=0; i<6; ++i) {
    macAddr <<= 8;
    macAddr |= (decDes[i] & 0xff);
  }

  if (decDes[6] != 'a') return false;
  if (decDes[7] != 'k') return false;

  return true;
}

//---------------------------------------------------------------------------

MacAddressList::MacAddressList(bool noVmWare)
: devLst(NULL), sz(0), generic(MacAddress::generic,"0.0.0.0","Generic")
{
  IP_ADAPTER_INFO adapterInfo1[1];
  ULONG bufLen = 0;

  // Inquire required size of buffer first (by setting bufLen to zero);
  DWORD ret = GetAdaptersInfo(adapterInfo1,&bufLen);
  if (ret != ERROR_BUFFER_OVERFLOW) return;

  // Allocate required size
  char *infoBuf = new char[bufLen];
  IP_ADAPTER_INFO *adapterInfo = (IP_ADAPTER_INFO *)infoBuf;

  // Get info
  ret = GetAdaptersInfo(adapterInfo,&bufLen);
  if (ret != ERROR_SUCCESS) return;

  IP_ADAPTER_INFO *curAdapter = adapterInfo;

  while (curAdapter) {
    if (curAdapter->Type == MIB_IF_TYPE_ETHERNET && curAdapter->AddressLength >= 6) {
      char upcDesc[2048] = "";
      strncpy(upcDesc,curAdapter->Description,2047);
      upcDesc[2047] = '\0';

      toUpper(upcDesc);

      if (!noVmWare || !strstr(upcDesc,"VMWARE")) ++sz;
    }
    curAdapter = curAdapter->Next;
  }

  devLst = new MacAddress*[sz];
  sz = 0;

  curAdapter = adapterInfo;

  while (curAdapter) {
    if (curAdapter->Type == MIB_IF_TYPE_ETHERNET && curAdapter->AddressLength >= 6) {
      char upcDesc[2048] = "";
      strncpy(upcDesc,curAdapter->Description,2047);
      upcDesc[2047] = '\0';

      toUpper(upcDesc);

      if (!noVmWare || !strstr(upcDesc,"VMWARE")) {
        __int64 macAdr = 0;

        for (int i=0; i<6; ++i) {
          macAdr <<= 8;
          macAdr |= curAdapter->Address[i];
        }

        const char *ipStr = curAdapter->IpAddressList.IpAddress.String;

        devLst[sz++] = new MacAddress(macAdr,ipStr,curAdapter->Description);
      }
    }

    curAdapter = curAdapter->Next;
  }

  delete[] infoBuf;
}

//---------------------------------------------------------------------------

MacAddressList::~MacAddressList()
{
  if (devLst) {
    for (int i=0; i<sz; ++i) {
      if (devLst[i]) delete devLst[i];
    }

    delete[] devLst;
  }
}

//---------------------------------------------------------------------------

const MacAddress& MacAddressList::getDevice(int idx) const
{
  if (!devLst || idx < 0 || idx >= sz) idx = 0;

  return *devLst[idx];
}

} // namespace

//---------------------------------------------------------------------------
