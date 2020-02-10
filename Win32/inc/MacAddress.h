//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Retrieve Windows Pc's MAC address --------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV NOV 2005 ---------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef AW_MAC_ADDRESS_INC
#define AW_MAC_ADDRESS_INC

namespace AW
{

//-------------------------------------------------------------------------
// Denotes an Ethernet Interface (Address)
// Can only be obtained from a MacAdddressList

class MacAddress
{
  __int64 macAddress;
  mutable char macAddressStr[17];

  const char *ipAddress;
  const char *description;

  MacAddress(__int64 mac, const char *ip, const char *desc);
  MacAddress(const MacAddress& cp);
  MacAddress& operator=(const MacAddress& src);

  ~MacAddress();

public:
  __int64 getMacAddress() const { return macAddress; }

  const char *getHexEncryptedMacAddressString() const;
  const char *getIpAddress() const { return ipAddress; }
  const char *getDescription() { return description; }
  static bool decodeHexDecryptedMacStr(const char macStr[17], __int64& macAddr);

  static const __int64 generic;
  friend class MacAddressList;
};

//-------------------------------------------------------------------------
// Denotes the Windows machines Ethernet Interfaces.

class MacAddressList
{
  MacAddress **devLst;
  int sz;

  MacAddress generic;

  MacAddressList(const MacAddressList& cp);
  MacAddressList& operator=(const MacAddressList& src);

public:
  MacAddressList(bool noVmWare = true);
  ~MacAddressList();

  int size() const { return sz; }

  const MacAddress& getDevice(int idx) const;

  const MacAddress& getGeneric() const { return generic; }
};

} //namespace

//---------------------------------------------------------------------------
#endif
