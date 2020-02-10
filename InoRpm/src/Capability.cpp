//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Capability (and list) --------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Aug 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Capability.h"

#include "Package.h"

#include "Basics.h"

#include <string.h>

namespace InoRpm
{

using namespace Ino;

//---------------------------------------------------------------------------

Capability::Capability(const char *capName, const char *capVer, int flg)
: Persistable(), pkg(NULL),
  name(dupStr(capName)), version(dupStr(capVer)), flags(flg)
{
}

//---------------------------------------------------------------------------

Capability::Capability(const Capability& cp)
: Persistable(cp),
  pkg(cp.pkg), 
  name(dupStr(cp.name)),
  version(dupStr(cp.version)),
  flags(cp.flags)
{
}

//---------------------------------------------------------------------------

Capability::~Capability()
{
  delete[] name;
  delete[] version;
}

//---------------------------------------------------------------------------

#ifdef _WIN32
bool Capability::getFullString(char *buf, int bufSz) const
{
  return false;
}

bool Capability::getNvr(char *buf, int bufSz) const
{
  return false;
}

#else

//---------------------------------------------------------------------------

bool Capability::getNvr(char *buf, int bufSz) const
{
  if (!buf) return false;

  int reqLen = strlen(name) + 1 + strlen(version) + 1;
  if (reqLen > bufSz) return false;

  strcpy(buf,name);
  strcat(buf,"-");
  strcat(buf,version);

  return true;
}

//---------------------------------------------------------------------------

bool Capability::getFullString(char *buf, int bufSz) const
{
  if (!buf) return false;

  int reqLen = strlen(name) + 4 + strlen(version) + 1;
  if (reqLen > bufSz) return false;

  strcpy(buf,name);
  strcat(buf," ");

  if (flags & RPMSENSE_LESS) strcat(buf,"<");
  else if (flags & RPMSENSE_GREATER)strcat(buf,">");

  if (flags & RPMSENSE_EQUAL) strcat(buf,"=");

  strcat(buf," ");
  strcat(buf,version);

  return true;
}

#endif

//---------------------------------------------------------------------------
// Persistence section

static const char fldPackage[] = "Package";
static const char fldName[]    = "Name";
static const char fldVersion[] = "Version";
static const char fldFlags[]   = "Flags";

//---------------------------------------------------------------------------

void Capability::definePersistentFields(Ino::PersistentWriter& po) const
{
  po.addField(fldPackage,typeid(Package));
  po.addArrayField(fldName,(char *)NULL);
  po.addArrayField(fldVersion,(char *)NULL);
  po.addField(fldFlags,typeid(long));
}

//---------------------------------------------------------------------------

Capability::Capability(Ino::PersistentReader& pi)
: Persistable(),
  pkg((Package *)pi.readObject(fldPackage)),
  name((char *)pi.readValArray(fldName,NULL)),
  version((char *)pi.readValArray(fldVersion,NULL)),
  flags((unsigned int)pi.readInt(fldFlags,0))
{
}

//---------------------------------------------------------------------------

void Capability::writePersistentObject(Ino::PersistentWriter& po) const
{
  po.writeObject(fldPackage,pkg);
  po.writeArray(fldName,name,strlen(name)+1); // Include the trailng zero!!
  po.writeArray(fldVersion,version,strlen(version)+1);
  po.writeInt(fldFlags,flags);
}

} // namespace

//---------------------------------------------------------------------------

