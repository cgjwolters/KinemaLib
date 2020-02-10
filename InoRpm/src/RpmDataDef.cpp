//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- InoRpm Persistence Data Definitions  ------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hek Aut BV Oct 2009 ------------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "RpmDataDef.h"

#include "Capability.h"
#include "CapabilityList.h"
#include "FileList.h"
#include "Package.h"
#include "PackageList.h"

//---------------------------------------------------------------------------

namespace InoRpm
{

using namespace Ino;

//---------------------------------------------------------------------------

static const long ifamMagic = 0x2382f23a;
static const long imagic    = 0x4fc929cb;

//---------------------------------------------------------------------------

RpmDataDef::RpmDataDef()
: PersistentTypeDef(ifamMagic,imagic,1,0)
{
  add(new PersistentType<Capability>("nl.inofor.inorpm.Capability"));
  add(new PersistentType<CapabilityList>("nl.inofor.inorpm.CapabilityList"));
  add(new PersistentType<FileString>("nl.inofor.inorpm.FileString"));
  add(new PersistentType<FileString>("nl.inofor.inorpm.FileList"));
  add(new PersistentType<FileString>("nl.inofor.inorpm.Package"));
  add(new PersistentType<FileString>("nl.inofor.inorpm.PackageList"));
}

} // namespace

//---------------------------------------------------------------------------
