//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General File Manipulation Object ----------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "AnsiFile.h"
#include "Basics.h"

#include <wchar.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <windows.h>
#include <shlobj.h>

//---------------------------------------------------------------------------

static bool containsSlash(const char *p)
{
  if (!p) return false;

  while (*p) {
    if (*p == '/' || *p == '\\') return true;

    ++p;
  }

  return false;
}

//---------------------------------------------------------------------------

void replaceSlashes(char *p)
{
  if (!p) return;

  while (*p) {
    if (*p == '\\') *p = '/';
    p++;
  }
}

//---------------------------------------------------------------------------

int removeTrailingSlashes(char *p)
{
  if (!p) return 0;

  char *t = p;

  while (*t) ++t;

  while (t > p) {
    --t;
    if (*t == '\\' || *t == '/') *t = '\0';
    else {
      ++t;
      break;
    }
  }

  return t-p;
}

//---------------------------------------------------------------------------

namespace Ino
{

using namespace std;

//---------------------------------------------------------------------------
/** \addtogroup file_io File System Classes
 @{
*/

//---------------------------------------------------------------------------
/** \file AnsiFile.h
  Defines class Ino::AnsiFile, a class for file information and manipulation.

  \see Unifile.h
  \author C. Wolters
  \date Nov 2005
*/

//---------------------------------------------------------------------------
/** \class AnsiFileFilter
   A virtual file selection filter.

   Derive a class from this class and implement method accept() to define
   a filter of your own.\n
   \n
   See the \c findXXX methods of class AnsiFile for details.
   \see AnsiFile
   \author C. Wolters
   \date Nov 2005
*/

//---------------------------------------------------------------------------
/** \class AnsiFile
  Denotes a file or directory in the file system.
  All path strings are supplied as ASCII characters.\n
  Regardless of the operating system both forwardslashes and backslashes
  are valid filename separators.

  \note
  All pathnames are treated as \b case-sensitive in principle, however
  on MS Windows methods that access the underlying filesystem will behave
  as MS Windows does.\n
  This means that methods such as setName()
  and the equality operator are case-sensitive, because they only operate
  on the path held by this object.
  
  \see UniFile
  \author C. Wolters
  \date Nov 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** \fn AnsiFileFilter::~AnsiFileFilter()
   Destructor.
*/

//---------------------------------------------------------------------------
/** \fn bool AnsiFileFilter::accept(const AnsiFile& fileOrDir) const
   File or directory search filter.

   This method is called from the \c findXXX() methods of AnsiFile for
   each matching file or directory. This method gets the chance to
   decide ultimately whether the file or directory is to be included in the
   search result.

   \param fileOrDir The file or directory to decide upon.
   \return \c true if the file or directory is to be included in the
   search result,\n
   \c false if it is to be excluded.
*/

//---------------------------------------------------------------------------

void AnsiFile::init() const
{
  if (pathStr && pathStrSz > 0) return;

  char dirName[1500] = "";

  int len = GetCurrentDirectoryA(1500,dirName);
  if (len < 1) return;

  pathStrSz = len;

  if (!pathStr) {
    pathStrCap = pathStrSz+32;
    pathStr = new char[pathStrCap];
  }
  else if (pathStrCap < pathStrSz+1) {
    pathStrCap = pathStrSz+32;
    pathStr = (char *)realloc(pathStr,pathStrCap*sizeof(char));
  }
  
  strcpy(pathStr,dirName);
  replaceSlashes(pathStr);
  pathStrSz = removeTrailingSlashes(pathStr);
}

//---------------------------------------------------------------------------
/** Default constructor.
  This file will have an empty path.
  \note
  Many of the methods of this class will convert an empty path
  to a path containing the <em>current directory</em>.
*/

AnsiFile::AnsiFile()
: fndHdl(-1), fndMode(FindAll), findInfo(), srchFilter(NULL),
  pathStr(NULL),pathStrSz(0), pathStrCap(0),
  fileLst(NULL), fileLstCap(0)
{
}

//---------------------------------------------------------------------------
/** Copy constructor.
   Any find status is \b not copied, see the \c findXXX() methods.
   \param cp the object to make a copy from.
*/

AnsiFile::AnsiFile(const AnsiFile& cp)
: fndHdl(-1), fndMode(FindAll), findInfo(), srchFilter(NULL),
  pathStr(dupStr(cp.pathStr)),
  pathStrSz(cp.pathStrSz), pathStrCap(pathStrSz+1),
  fileLst(NULL), fileLstCap(0)
{
}

//---------------------------------------------------------------------------
/** Destructor.
*/

AnsiFile::~AnsiFile()
{
  cancelFind();

  if (pathStr) delete[] pathStr;
  if (fileLst) delete[] fileLst;
}

//---------------------------------------------------------------------------
/** Constructor, initializes on the path supplied.
  \param path The path of the file or directory to initialize this
  AnsiFile on, may be \c NULL.

  The path supplied may be either \e relative or \e absolute.\n
  A relative path is evalutated with respect to the current directory.\n
  See also getAbsoluteFile().
*/

AnsiFile::AnsiFile(const char *path)
: fndHdl(-1), fndMode(FindAll), findInfo(), srchFilter(NULL), pathStr(NULL),
  pathStrSz(0), pathStrCap(0),
  fileLst(NULL), fileLstCap(0)
{
  if (path) {
    pathStrSz = strlen(path);
    pathStrCap = pathStrSz+32;

    pathStr = new char[pathStrCap];
    strcpy(pathStr,path);

    replaceSlashes(pathStr);
    pathStrSz = removeTrailingSlashes(pathStr);
  }
}

//---------------------------------------------------------------------------
/** Constructor, initializes on a file in a directory.
  \param parent The parent directory of the file.
  \param path The name of the file or directory.
*/

AnsiFile::AnsiFile(const AnsiFile& parent, const char *path)
: fndHdl(-1), fndMode(FindAll), findInfo(), srchFilter(NULL),
  pathStr(dupStr(parent.pathStr)),
  pathStrSz(parent.pathStrSz), pathStrCap(pathStrSz+1),
  fileLst(NULL), fileLstCap(0)
{
  operator+=(AnsiFile(path));
}

//---------------------------------------------------------------------------
/** Assignment operator.
   \param src the object to make a copy from.
   \return A reference to this object.
   \note
   Any find operation in progress is \link cancelFind() cancelled\endlink
   first.\n
   The find status of parameter \c src is \b not copied,
   see the \c findXXX() methods for details.
*/

AnsiFile& AnsiFile::operator=(const AnsiFile& src)
{
  if (&src == this) return *this;

  cancelFind();

  if (pathStr) delete[] pathStr;

  pathStr = NULL;
  pathStrSz = 0;
  pathStrCap = 0;

  if (src.pathStr) {
    pathStrSz = strlen(src.pathStr);
    pathStrCap = pathStrSz + 32;

    pathStr = new char[pathStrCap];
    strcpy(pathStr,src.pathStr);
  }

  return *this;
}

//---------------------------------------------------------------------------
/** \fn AnsiFile::operator const char*() const
  Conversion operator, equivalent to getPath().
  \return A pointer to the pathstring, may be \c NULL (if there is no valid
  current directory).

  \note
  If the path is empty it is first initialized to the current directory.
*/

//---------------------------------------------------------------------------
/** Concatenates a relative path to this Ansifile.
   
   \param wf The path to concatenate.
   \return A reference to this object.
   The path of this AnsiFile and of parameter \c wf are simply
   concatenated, removing double embedded slash characters.\n
   This is done even if parameter \c wf denotes an absolute path.\n
   \n
   If the path in parameter \c wf is empty, this is a no-op.\n
   Else if the path in this AnsiFile is empty it is initialized
   to the current directory.
   \note
   Any find operation in progress is \ref cancelFind() "cancelled" first.
*/

AnsiFile& AnsiFile::operator+=(const AnsiFile& wf)
{
  if (&wf == this) return *this;

  cancelFind();

  if (!wf.pathStr || wf.pathStrSz < 1) return *this;

  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr) {
    pathStr = dupStr(wf.pathStr);
    pathStrSz = wf.pathStrSz;
    pathStrCap = pathStrSz+1;

    return *this;
  }

  int minCap = pathStrSz + wf.pathStrSz + 2;

  if (minCap > pathStrCap) {
    pathStrCap = minCap+32;
    pathStr = (char *)realloc(pathStr,minCap*sizeof(char));
  }

  if (pathStrSz > 0 && pathStr[pathStrSz-1] != '/' &&
                                           pathStr[pathStrSz-1] != '\\') {
    pathStr[pathStrSz++] = '/';
    pathStr[pathStrSz] = 0;
  }

  strcat(pathStr+pathStrSz,wf.pathStr);
  pathStrSz += wf.pathStrSz;

  return *this;
}

//---------------------------------------------------------------------------
/** Compare operator.
  \param file The file to compare with.
  \return \c true if:
  \li 1. This AnsiFile and \c file both have an empty path.
  \li 2. The paths of \c this and \c file are identical (\b not ignoring
  case).
  \li 3. \c getAbsoluteFile(*this) == getAbsoluteFile(file) returns \c true.
  \c false otherwise.

  \note
  If either this AnsiFile or \c file have an empty path, they are first
  initialized to the current directory.
*/

bool AnsiFile::operator==(const AnsiFile& file) const
{
  if (&file == this) return true;

  if (!pathStr) init();
  if (!file.pathStr) file.init();

  if (!pathStr) return file.pathStr == NULL;
  else if (!file.pathStr) return false;

  if (!strcmp(pathStr,file.pathStr)) return true;

  AnsiFile af1, af2;
  if (!getAbsoluteFile(af1) || !file.getAbsoluteFile(af2)) return false;

  if (!af1.pathStr) return false;
  else {
    if (!af2.pathStr) return false;

    return !strcmp(af1.pathStr,af2.pathStr);
  }
}

//---------------------------------------------------------------------------
/** Checks if the file or directory exists.
  \return \c true If the file or directory that this AnsiFile represents exists,\n
  \c false otherwise.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::exists() const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  return _access(pathStr,00) == 0; 
}

//---------------------------------------------------------------------------
/** Checks if the file or directory can be read.
  \return \c true if the file or directory that this AnsiFile represents
  can be read,\n
  \c false otherwise.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::canRead() const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  return _access(pathStr,04) == 0; 
}

//---------------------------------------------------------------------------
/** Checks if the file or directory is writeable.
  \return \c true if the file or directory that this AnsiFile represents
  can be written to,\n
  \c false otherwise.
  \note
  If the path is empty it is first initialized to the current directory.
  \note
  This method is not reliable when applied to a directory!
*/

bool AnsiFile::canWrite() const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  return _access(pathStr,02) == 0; 
}

//---------------------------------------------------------------------------
/** Checks if the file or directory can be deleted.
  \return \c true if the file or directory that this AnsiFile represents
  can be delete,\n
  \c false otherwise.
  \note
  This method does not check if the directory is empty, only the access rights
  are checked.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::canRemove() const
{
  AnsiFile parent;
  if (!getParent(parent)) return false;

  return canWrite() && parent.canWrite();
}

//---------------------------------------------------------------------------
/** Checks if this object denotes a file.
  \return \c true if this object denotes an existing file,\n
  \c false if this object denotes a directory (or link) or if the path does
  not denote an existing file or directory (use method
  \ref exists() const "exists" first).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::isFile() const
{
  if (!pathStr || pathStrSz < 1) init();

  struct _stat st;
  if (_stat(pathStr,&st)) return false;

  return (st.st_mode & _S_IFREG) != 0;
}

//---------------------------------------------------------------------------
/** Checks if this object denotes a directory.
  \return \c true if this object denotes an existing directory,\n
  \c false if this object denotes a file (or link) or if the path does
  not denote an existing file or directory (use method
  \ref exists() const "exists" first).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::isDirectory() const
{
  if (!pathStr || pathStrSz < 1) init();

  struct _stat st;
  if (_stat(pathStr,&st)) return false;

  return (st.st_mode & _S_IFDIR) != 0;
}

//---------------------------------------------------------------------------
/** Checks if this object represents a root directory.
  \return \c true if this object represents a root directory.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::isRootDir() const
{
  if (!pathStr || pathStrSz < 1) init();

  AnsiFile absFile;
  getAbsoluteFile(absFile);

  if (!absFile.pathStr || absFile.pathStrSz < 1) return false;

  char dir[_MAX_DIR];

  _splitpath(absFile.pathStr,NULL,dir,NULL,NULL);

  if (!strcmp(dir,"/") || !strcmp(dir,"\\")) return true;

  return false;
}

//---------------------------------------------------------------------------
/** Checks if this objects path has an extension.
  \return \c true if the path of this object has an extension, this occurs
  if the filename part contains a dot followed by one or more characters.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::hasExtension() const
{
  if (!pathStr || pathStrSz < 1) init();

  string ext;
  getExtension(ext);

  return ext.length() > 0;
}

//---------------------------------------------------------------------------
/** Checks if this objects path is a \e relative path.
  \return \c true if the path of this object is a relative path, i.e.
  does not begin with a slash (or backslash).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::isRelative() const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return true;

  char dir[_MAX_DIR];

  _splitpath(pathStr,NULL,dir,NULL,NULL);

  if (strlen(dir) < 1) return true;

  if (dir[0] == '/' || dir[0] == '\\') return false;
  
  return true;
}

//---------------------------------------------------------------------------
/** Gets the size (in bytes) of the file represented by this object.
  \return The filesize or -1 of the file does not exist.\n
  The size of a directory is (currently) left undefined (to be documented).
  \note
  If the path is empty it is first initialized to the current directory.
*/

__int64 AnsiFile::getSize() const
{
  if (!pathStr || pathStrSz < 1) init();

  struct _stat st;
  if (_stat(pathStr,&st)) return -1;

  return st.st_size;
}

//---------------------------------------------------------------------------
/** Gets creation date of the file/directory represented by this object.
  \return The creation date of the file/directory or -1 if the file or
  directory does not exist.\n
  The value returned is expressed as the number of seconds since
  midnight January 1 1970.
  \note
  If the path is empty it is first initialized to the current directory.
*/

long AnsiFile::getCreationDate() const
{
  if (!pathStr || pathStrSz < 1) init();

#ifdef _WIN32
  struct _stat32 st;
  if (_stat32(pathStr,&st)) return -1;
#else
  struct _stat st;
  if (_stat(pathStr,&st)) return -1;
#endif

  return st.st_ctime;
}

//---------------------------------------------------------------------------
/** Gets modified date of the file/directory represented by this object.
  \return The date of last modification of the file/directory or -1 if
  the file or directory does not exist.\n
  The value returned is expressed as the number of seconds since
  midnight January 1 1970.
  \note
  If the path is empty it is first initialized to the current directory.
  \note
  The date returned is meaningless for FAT filesystems.
*/

long AnsiFile::getModifiedDate() const
{
  if (!pathStr || pathStrSz < 1) init();

#ifdef _WIN32
  struct _stat32 st;
  if (_stat32(pathStr,&st)) return -1;
#else
  struct _stat st;
  if (_stat(pathStr,&st)) return -1;
#endif

  return st.st_mtime;
}

//---------------------------------------------------------------------------
/** Returns the filename (title + extension) part from the path
  held by this object.

  This method operates on the path only, the actual file or directory does
  not have to exist.

  \param fileName Receives the filename, will be empty if the file or
  directory does not exist.
  \note
  If the path is empty it is first initialized to the current directory.
*/

void AnsiFile::getName(string& fileName) const
{
  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr || pathStrSz < 1) fileName = "";
  else {
    char fTitle[_MAX_FNAME] = "";
    char fExt[_MAX_EXT] = "";
    _splitpath(pathStr,NULL,NULL,fTitle,fExt);

    fileName = fTitle;
    if (strlen(fExt) > 0) fileName += fExt;
  }
}

//---------------------------------------------------------------------------
/** Returns the title (filename without extension) of the file/directory
  represented by this object.

  This method operates on the path only, the actual file or directory does
  not have to exist.

  \param title Receives the title (without trailing dot), will be empty if
  the file or directory does not exist.
  \note
  If the path is empty it is first initialized to the current directory.
*/

void AnsiFile::getFileTitle(string& title) const
{
  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr || pathStrSz < 1) title = "";
  else {
    char fTitle[_MAX_FNAME] = "";
    _splitpath(pathStr,NULL,NULL,fTitle,NULL);

    title = fTitle;
  }
}

//---------------------------------------------------------------------------
/** Returns the filename extension of the file/directory
  represented by this object.

  This method operates on the path only, the actual file or directory does
  not have to exist.

  \param extension Receives the externsion (with a leading dot), will be empty if
  the file or directory does not exist.
  \note
  If the path is empty it is first initialized to the current directory.
*/

void AnsiFile::getExtension(string& extension) const
{
  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr || pathStrSz < 1) extension = "";
  else {
    char fExt[_MAX_EXT] = "";
    _splitpath(pathStr,NULL,NULL,NULL,fExt);

    extension = fExt;
  }
}

//---------------------------------------------------------------------------
/** Changes the filename part in the path held by this object.

  \attention
  The actual file/directory is \c not renamed and does not have to exist!\n
  See method \ref moveTo(const AnsiFile& newAnsiFile) "moveTo" for that.

  \param  newName The new filename (title+extension).
  \return \c true if the operation was successfull,\n
  \c false if:
  \li \c newName is \c NULL.
  \li \c newName is empty.
  \li \c newName contains illegal characters (notably a slash or backslash).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::setName(const char *newName)
{
  cancelFind();

  if (!newName || strlen(newName) < 1) return false;

  if (containsSlash(newName)) return false;

  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr) {
    pathStr = dupStr(newName);
    
    if (pathStr) {
      pathStrSz = strlen(pathStr);
      pathStrCap = pathStrSz+1;
    }
    else {
      pathStrSz  = 0;
      pathStrCap = 0;
    }
  }
  else {
    char newTitle[_MAX_FNAME], newExt[_MAX_FNAME];
    _splitpath(newName,NULL,NULL,newTitle,newExt);
    

    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    char fTitle[_MAX_FNAME], fExt[_MAX_EXT];

    _splitpath(pathStr,drive,dir,fTitle,fExt);

    char newPath[1500] = "";

    _makepath(newPath,drive,dir,newTitle,newExt);

    pathStrSz = strlen(newPath);

    if (pathStrSz >= pathStrCap-1) {
      pathStrCap = pathStrSz + 32;
      pathStr = (char *)realloc(pathStr,pathStrCap*sizeof(char));
    }
    
    strcpy(pathStr,newPath);
    pathStrSz = removeTrailingSlashes(pathStr);
  }

  return true;
}

//---------------------------------------------------------------------------
/** Changes the title (filename without extension) in the path held by
  this object.

  \attention
  The actual file/directory is \c not renamed and does not have to exist!\n
  See method \ref moveTo(const AnsiFile& newAnsiFile) "moveTo" for that.

  \param  newTitle The new title (filename minus extension).
  \return \c true if the operation was successfull,\n
  \c false if:
  \li \c newTitle is \c NULL.
  \li \c newTitle is empty.
  \li \c newTitle contains illegal characters (notably a slash or backslash).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::setFileTitle(const char *newTitle)
{
  cancelFind();

  if (!newTitle || strlen(newTitle) < 1) return false;

  if (containsSlash(newTitle)) return false;

  if (!pathStr || pathStrSz < 1) init();

  if (!pathStr) {
    pathStr = dupStr(newTitle);

    if (pathStr) {
      pathStrSz = strlen(pathStr);
      pathStrCap = pathStrSz+1;
    }
    else {
      pathStrSz = 0;
      pathStrCap = 0;
    }
  }
  else {
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fExt[_MAX_EXT];

    _splitpath(pathStr,drive,dir,NULL,fExt);

    char newPath[1500] = "";

    _makepath(newPath,drive,dir,newTitle,fExt);

    pathStrSz = strlen(newPath);

    if (pathStrSz >= pathStrCap-1) {
      pathStrCap = pathStrSz + 32;
      pathStr = (char *)realloc(pathStr,pathStrCap*sizeof(char));
    }
    
    strcpy(pathStr,newPath);
  }

  return true;
}

//---------------------------------------------------------------------------
/** Changes the extension of the filename in the path held by
  this object.

  \attention
  The actual file/directory is \c not renamed and does not have to exist!\n
  See method \ref moveTo(const AnsiFile& newAnsiFile) "moveTo" for that.

  \param newExt The new extension of the filename, may be \c NULL
  or empty (in which case the extension is removed).
  \return \c true if the operation was successfull,\n
  \c false if:
  \li \c newTitle contains illegal characters (notably a slash or backslash).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::setExtension(const char *newExt)
{
  cancelFind();

  if (containsSlash(newExt)) return false;

  if (!pathStr || pathStrSz < 1) init();

  char drive[_MAX_DRIVE], dir[_MAX_DIR], fTitle[_MAX_FNAME];

  _splitpath(pathStr,drive,dir,fTitle,NULL);

  char newPath[1500] = "";
  _makepath(newPath,drive,dir,fTitle,newExt);

  pathStrSz = strlen(newPath);

  if (pathStrSz >= pathStrCap-1) {
    pathStrCap = pathStrSz + 32;
    pathStr = (char *)realloc(pathStr,pathStrCap*sizeof(char));
  }
  
  strcpy(pathStr,newPath);

  return true;
}

//---------------------------------------------------------------------------
/** Moves or renames a file or directory.
  \param newAnsiFile The object holding the new name/path of the file/directory
  represented by this object.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li The original file or directory does not exist.
  \li If \c newAnsiFile contains an emtpy path.
  \li The operating system refuses to perform the operation.
  \note
   Any find operation in progress is \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory,\n
  so <b>beware, you might be renaming the current directory!!!</b>
*/

bool AnsiFile::moveTo(const AnsiFile& newAnsiFile)
{
  cancelFind();

  if (!exists() || !newAnsiFile.pathStr ||
                                   newAnsiFile.pathStrSz < 1) return false;

  return MoveFileA(pathStr,newAnsiFile.pathStr) == TRUE;
}

//---------------------------------------------------------------------------
/** Copies a file.
  \param newAnsiFile The object holding the destination name/path of the file
  to be copied.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li The original file does not exist or is a directory.
  \li If \c newAnsiFile contains an emtpy path.
  \li The operating system refuses to perform the operation.
  \note
   Any find operation in progress is \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::copyTo(const AnsiFile& newAnsiFile)
{
  if (!exists() || !newAnsiFile.pathStr ||
                                   newAnsiFile.pathStrSz < 1) return false;

  return CopyFileA(pathStr,newAnsiFile.pathStr,TRUE) == TRUE;
}

//---------------------------------------------------------------------------
/** Changes the <em>current directory</em>.
  Makes the directory represented by this object the currrent directory
  for this process.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li This is not a directory.
  \li There is no read access to the directory.
  \li The operating system refuses to perform the operation.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::setAsCurrentDir()
{
  if (!isDirectory() || !canRead()) return false;

  return SetCurrentDirectoryA(pathStr) == TRUE;
}

//---------------------------------------------------------------------------
/** Creates a new directory.
  Creates the directory defined by the path held by this object.
  \param recursive If \c true parent directories of the target directory
  are created as well where necessary.
  \return \c true if the operation was successfull or if the directory already
  exists,\n
  \c false if:
  \li A \b file with the same path already exists.
  \li The path denotes an existing \ref isRootDir() const "root directory".
  \li \c recursive is \c false and the parent of the target directory does not
  exist or is not a directory.
  \li The operating system refuses to perform the operation (perhaps due to
  access restrictions).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::createDir(bool recursive)
{
  if (exists()) return isDirectory();
  if (isRootDir()) return false;

  if (recursive) {
    AnsiFile parent;
    if (!getParent(parent) || !parent.createDir(true)) return false;
  }
  
  return CreateDirectoryA(pathStr,NULL) == TRUE;
}
//---------------------------------------------------------------------------
/** Creates a new file.
  Creates the file defined by the path held by this object.
  \param recursive If \c true parent directories of the target file
  are created as well where necessary.
  \return \c true if the operation was successfull or if the file already
  exists,\n
  \c false if:
  \li A \b directory with the same path already exists.
  \li \c recursive is \c false and the parent of the target directory does not
  exist or is not a directory.
  \li The operating system refuses to perform the operation (perhaps due to
  access restrictions).
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::createFile(bool recursive)
{
  if (exists()) return isFile();

  if (recursive) {
    AnsiFile parent;
    if (!getParent(parent) || !parent.createDir(true)) return false;
  }
  
  HANDLE hdl = CreateFileA(pathStr,GENERIC_WRITE,0,NULL,
                                 OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

  if (hdl == INVALID_HANDLE_VALUE) return false;

  CloseHandle(hdl);

  return true;
}

//---------------------------------------------------------------------------
/** Deletes a file.
  Deletes the file defined by the path held by this object.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li The file does not exist or (for a directory) is not empty.
  \li The operating system refuses to perform the operation (perhaps due to
  access restrictions).
  \note
  Any find operation in progress is \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be trying to remove the current directory!</b>
*/

bool AnsiFile::remove()
{
  cancelFind();

  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  return ::remove(pathStr) == 0;
}

//---------------------------------------------------------------------------
/** Gets a copy of the path string that is held by this object.
  \param path The string that will receive a copy of the path.
  \note
  If the path is empty it is first initialized to the current directory.
*/

void AnsiFile::getPath(string& path)
{
  if (!pathStr || pathStrSz < 1) init();

  if (pathStr) path = pathStr;
  else path = "";
}

//---------------------------------------------------------------------------
/** Returns a pointer to the path string that is held by this object.

  \return A pointer to the pathstring, may be \c NULL (if there is no valid
  current directory).

  \note
  If the path is empty it is first initialized to the current directory.
*/

const char *AnsiFile::getPath() const
{
  if (!pathStr || pathStrSz < 1) init();

  return pathStr;
}

//---------------------------------------------------------------------------
/** Makes this object represent the parent of the file/directory it
  represent until now.
  \return \c true if the operation is successfull,
  \c false if:
  \li The path held by this object is invalid.
  \li The file/directory this object represents does not exist.
  \li This object represents the \ref isRootDir() const "root directory".
  \note
  Any find operation in progress is \ref cancelFind() "cancelled" first.
  \note
  If the path is initially empty it is first initialized to the
  current directory.
*/

bool AnsiFile::getParent()
{
  return getParent(*this);
}

//---------------------------------------------------------------------------
/** Returns the parent of this object.
  \param parent The object that will become the parent of this object.\n
  If \c false is returned object \c parent is left unmodified.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li The path held by this object is invalid.
  \li The file/directory this object represents does not exist.
  \li This object represents the \ref isRootDir() const "root directory".
  \note
  Any find operation in progress on object \c parent is
  \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::getParent(AnsiFile& parent) const
{
  parent.cancelFind();

  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  char drive[_MAX_DRIVE], dir[_MAX_DIR], fTitle[_MAX_FNAME];
  _splitpath(pathStr,drive,dir,fTitle,NULL);

  int dirLen = strlen(dir);

  char newPath[1500] = "";

  if (!strcmp(dir,"/") || !strcmp(dir,"\\")) {
    if (strlen(fTitle) < 1) return false;

    strcpy(newPath,drive);
    strcat(newPath,"/");
  }
  else if (dirLen < 1) {
    AnsiFile absFile;
    getAbsoluteFile(absFile);
    return absFile.getParent(parent);
  }
  else {  
    _makepath(newPath,drive,dir,NULL,NULL);
    if (strlen(newPath) < 1) return false;

    int len = strlen(newPath);
    if (len > 1 && newPath[len-1] == '/' ||
                                newPath[len-1] == '\\') newPath[len-1] = '\0';
  }

  parent.pathStrSz = strlen(newPath);

  if (parent.pathStrSz >= parent.pathStrCap-1) {
    parent.pathStrCap = parent.pathStrSz + 32;
    parent.pathStr = (char *)realloc(parent.pathStr,parent.pathStrCap*sizeof(char));
  }
  
  strcpy(parent.pathStr,newPath);

  return true;
}

//---------------------------------------------------------------------------
/** Gets an absolute path representation of this object.
  \param af The object that will hold the absolute representation.
  \return \c true if the operation was successfull,\n
  \c false if:
  \li The path held by this object is invalid.
  \li The file/directory this object represents does not exist.
  \note
  Any find operation in progress on object \c af is
  \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory.
*/

bool AnsiFile::getAbsoluteFile(AnsiFile& af) const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  char aPath[1500] = "";

  if (!_fullpath(aPath,pathStr,1499)) return false;

  af.cancelFind();

  af.pathStrSz = strlen(aPath);

  if (af.pathStrSz >= af.pathStrCap-1) {
    af.pathStrCap = af.pathStrSz+32;
    af.pathStr = (char *)realloc(af.pathStr,af.pathStrCap*sizeof(char));
  }

  strcpy(af.pathStr,aPath);

  return true;
}

//---------------------------------------------------------------------------

bool AnsiFile::findFirstAny(const char *spec) const
{
  if (!pathStr || pathStrSz < 1) init();
  if (!pathStr || pathStrSz < 1) return false;

  char wildCard[] = "*";
  const char *wldPtr = wildCard;

  if (spec && strlen(spec) > 0) wldPtr = spec;

  char fndBuf[1500] = "";
  strcpy(fndBuf,pathStr);
  strcat(fndBuf,"/");
  strcat(fndBuf,wldPtr);

  fndHdl = _findfirst(fndBuf,&findInfo);

  return fndHdl >= 0;
}

//---------------------------------------------------------------------------
/** Starts a wildcard search for directories in the directory denoted by this
  object.
  The search is \b not recursive, only this directory is searched.\n
  \n
  The search position is stored, so that if \c true is returned, method
  \ref findNextDir(AnsiFile& dir) const "findNextDir()" may be called to
  find more matching directories.
  \param dir The object that will hold the first matching directory, if any.\n
  Left unmodified if no matching directory can be found.
  \param spec May contain a \e wildcard specification for the directory or
  directories to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  The strings "." and ".." are not legal.\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return \c true if at least one matching directory was found, parameter
  \c dir will hold the result,\n
  \c false if:
  \li This object does not represent an existing directory.
  \li Access rights prohibit searching the directory.
  \li No matching directory could be found.
  \note
  The directories denoted by  "." or ".." will never be returned.
  \note
  Any find operation in progress on this object and object \c dir is
  \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you may be searching the current directory</b>.
*/

bool AnsiFile::findFirstDir(AnsiFile& dir, const char *spec,
                                            AnsiFileFilter *filter) const
{
  cancelFind();
  fndMode = FindDirs;

  if (!findFirstAny(spec)) return false;

  srchFilter = filter;

  if ((findInfo.attrib & _A_SUBDIR) != 0 &&
          strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
    AnsiFile f(*this,findInfo.name);

    if (!srchFilter || srchFilter->accept(f)) {
      dir = f;
      return true;
    }
  }

  return findNextDir(dir);
}

//---------------------------------------------------------------------------
/** Continues a wildcard search for directories started with method
  findFirstDir().

  \param dir The object that will hold the next matching directory, if any.\n
  Left unmodified if no matching directory can be found.
  \return \c true if another matching directory was found,\n
  \c false if:
  \li Method findFirstDir() was not first called or that method
  returned \c false.
  \li Another matching directory was not found.
*/

bool AnsiFile::findNextDir(AnsiFile& dir) const
{
  if (fndHdl < 0 || fndMode != FindDirs) return false;

  while (_findnext(fndHdl,&findInfo) == 0) {
    if ((findInfo.attrib & _A_SUBDIR) != 0 &&
           strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
      AnsiFile f(*this,findInfo.name);

      if (!srchFilter || srchFilter->accept(f)) {
        dir = f;
        return true;
      }
    }
  }

  cancelFind();

  return false;
}

//---------------------------------------------------------------------------
/** Starts a wildcard search for files in the directory denoted by this
  object.
  The search is \b not recursive, only this directory is searched.\n
  \n
  The search position is stored, so that if \c true is returned, method
  \ref findNextFile(AnsiFile& file) const "findNextFile()" may be called to
  find more matching files.
  \param file The object that will hold the first matching file, if any.\n
  Left unmodified if no matching file can be found.
  \param spec May contain a \e wildcard specification for the file or
  files to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return \c true if at least one matching file was found, parameter
  \c file will hold the result,\n
  \c false if:
  \li This object does not represent an existing directory.
  \li Access rights prohibit searching the directory.
  \li No matching file could be found.
  
  \note
  Any find operation in progress on this object and object \c file is
  \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be searching the current directory</b>.
*/

bool AnsiFile::findFirstFile(AnsiFile& file, const char *spec,
                                             AnsiFileFilter *filter) const
{
  cancelFind();

  fndMode = FindFiles;

  if (!findFirstAny(spec)) return false;

  srchFilter = filter;

  if ((findInfo.attrib & _A_SUBDIR) == 0 &&
          strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
    AnsiFile f(*this,findInfo.name);

    if (!srchFilter || srchFilter->accept(f)) {
      file = f;
      return true;
    }
  }

  return findNextFile(file);
}

//---------------------------------------------------------------------------
/** Continues a wildcard search for files started with method
  findFirstFile().

  \param file The object that will hold the next matching file, if any.\n
  Left unmodified if no matching file can be found.
  \return \c true if another matching file was found,\n
  \c false if:
  \li Method findFirstFile() was not first called or that method
  returned \c false.
  \li Another matching file was not found.
*/

bool AnsiFile::findNextFile(AnsiFile& file) const
{
  if (fndHdl < 0 || fndMode != FindFiles) return false;

  while (_findnext(fndHdl,&findInfo) == 0) {
    if ((findInfo.attrib & _A_SUBDIR) == 0 &&
           strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
      AnsiFile f(*this,findInfo.name);

      if (!srchFilter || srchFilter->accept(f)) {
        file = f;
        return true;
      }
    }
  }

  cancelFind();

  return false;
}

//---------------------------------------------------------------------------
/** Starts a wildcard search for <em>files or directories</em> in the
  directory denoted by this object.
  The search is \b not recursive, only this directory is searched.\n
  \n
  The search position is stored so, that if \c true is returned, method
  \ref findNext(AnsiFile& file) const "findNext()" may be called to
  find more matching files or directories.
  \param fileOrDir The object that will hold the first matching file or directory,
  if any.\n
  Left unmodified if no matching file or directory can be found.
  \param spec May contain a \e wildcard specification for the file or
  directory to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return \c true if at least one matching file was found, parameter
  \c fileOrDir will hold the result,\n
  \c false if:
  \li This object does not represent an existing directory.
  \li Access rights prohibit searching the directory.
  \li No matching file or directory could be found.
  
  \note
  The directories denoted by  "." or ".." will never be returned.
  \note
  Any find operation in progress on this object and object \c fileOrDir is
  \ref cancelFind() "cancelled" first.
  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be searching the current directory</b>.
*/

bool AnsiFile::findFirst(AnsiFile& fileOrDir, const char *spec,
                                              AnsiFileFilter *filter) const
{
  cancelFind();

  fndMode = FindAll;

  srchFilter = filter;

  if (!findFirstAny(spec)) return false;

  if (strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
    AnsiFile f(*this,findInfo.name);

    if (!srchFilter || srchFilter->accept(f)) {
      fileOrDir = f;
      return true;
    }
  }

  return findNext(fileOrDir);
}

//---------------------------------------------------------------------------
/** Continues a wildcard search for files or directories started with method
  findFirst().

  \param fileOrDir The object that will hold the next matching file or directory,
  if any.\n
  Left unmodified if no matching file or directory can be found.
  \return \c true if another matching file or directory was found,\n
  \c false if:
  \li Method findFirst() was not first called or that method returned \c false.
  \li Another matching file or directory was not found.
*/

bool AnsiFile::findNext(AnsiFile& fileOrDir) const
{
  if (fndHdl < 0 || fndMode != FindAll) return false;

  while (_findnext(fndHdl,&findInfo) == 0) {
    if (strcmp(findInfo.name,".") && strcmp(findInfo.name,"..") ) {
      AnsiFile f(*this,findInfo.name);

      if (!srchFilter || srchFilter->accept(f)) {
        fileOrDir = f;
        return true;
      }
    }
  }

  cancelFind();

  return false;
}

//---------------------------------------------------------------------------
/** Stops a find in progress on this object.

  See the \c findFirstXXX() methods for details.
*/

void AnsiFile::cancelFind() const
{
  srchFilter = NULL;

  if (fndHdl >= 0) _findclose(fndHdl);
  fndHdl = -1;

  fndMode = FindAll;
}

//---------------------------------------------------------------------------

static int cmpFiles(const void *vf1, const void *vf2)
{
  if (!vf1) {
    if (!vf2) return 0;
    else return -1;
  }
  else if (!vf2) return 1;

  const AnsiFile *uf1 = (const AnsiFile *)vf1;
  const AnsiFile *uf2 = (const AnsiFile *)vf2;

  return compareAlphaNum(uf1->getPath(),uf2->getPath());
}

//---------------------------------------------------------------------------
/** Searches the directory denoted by this object for files.

  This method the search result in an internal list and returns
  a pointer to that list.\n

  This is a convenience method, based on methods findFirstFile() and
  findNextFile() where the result is optionally sorted and then
  stored in an internal list.\n
  \n
  
  \param listSz Will hold the number of files found.
  \param sort If \c true the search result will be sorted based
  on function \ref compareAlphaNum(const char *s1, const char *s2)
  "compareAlphaNum".
  \param spec May contain a \e wildcard specification for the file
  to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return A pointer to the internal list of found files if
  <tt>listSz > 0</tt>.\n
  The pointer will be invalid if <tt>listSz < 1</tt>!!\n
  The returned pointer is only valid until the next call to this method
  or method getDirList() or getList().\n

  \attention
  Do \b NOT \c delete the list returned by this method!!

  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be searching the current directory</b>.
  \note
  This method leaves other search operations in progress intact.
*/

AnsiFile *AnsiFile::getFileList(int& listSz, bool sort, const char *spec,
                                                 AnsiFileFilter *filter) const
{
  listSz = 0;

  const AnsiFileFilter *oldFilter = srchFilter;

  intptr_t oldFndHdl = fndHdl;
  fndHdl = -1;
  srchFilter = NULL;

  AnsiFile f;
  if (findFirstFile(f,spec,filter)) {
    listSz++;
    while (findNextFile(f)) listSz++;
  }

  if (listSz > fileLstCap) {
    if (fileLst) delete[] fileLst;

    fileLstCap = listSz + 64;
    fileLst = new AnsiFile[fileLstCap];
  }

  if (listSz > 0) {
    fileLst = new AnsiFile[listSz];

    int fCnt = 0;
    findFirstFile(fileLst[fCnt++],spec,filter);

    while (fCnt < listSz && findNextFile(fileLst[fCnt++]));

    listSz = fCnt;

    if (sort && listSz > 1) qsort(fileLst,listSz,sizeof(AnsiFile),cmpFiles);
  }

  fndHdl = oldFndHdl;
  srchFilter = oldFilter;

  return fileLst;
}

//---------------------------------------------------------------------------
/** Searches the directory denoted by this object for directories.

  This method the search result in an internal list and returns
  a pointer to that list.\n

  This is a convenience method, based on methods findFirstDir() and
  findNextDir() where the result is optionally sorted and then stored
  in an internal list.

  \param listSz Will hold the number of directories found.
  \param sort If \c true the search result will be sorted based
  on function \ref compareAlphaNum(const char *s1, const char *s2)
  "compareAlphaNum".
  \param spec May contain a \e wildcard specification for the directory
  to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return A pointer to the internal list of found directories if
  <tt>listSz > 0</tt>.\n
  The pointer will be invalid if <tt>listSz < 1</tt>!!\n
  The returned pointer is only valid until the next call to this method
  or method getFileList() or getList().\n

  \attention
  Do \b NOT \c delete the list returned by this method!!

  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be searching the current directory</b>.
  \note
  The directories denoted by  "." or ".." will never be returned.
  \note
  This method leaves other search operations in progress intact.
*/

AnsiFile *AnsiFile::getDirList(int& listSz, bool sort, const char *spec,
                                                AnsiFileFilter *filter) const
{
  listSz = 0;

  const AnsiFileFilter *oldFilter = srchFilter;

  intptr_t oldFndHdl = fndHdl;
  fndHdl = -1;
  srchFilter = NULL;

  AnsiFile f;
  if (findFirstDir(f,spec,filter)) {
    listSz++;
    while (findNextDir(f)) listSz++;
  }

  if (listSz > fileLstCap) {
    if (fileLst) delete[] fileLst;

    fileLstCap = listSz + 64;
    fileLst = new AnsiFile[fileLstCap];
  }

  if (listSz > 0) {
    fileLst = new AnsiFile[listSz];

    int fCnt = 0;
    findFirstDir(fileLst[fCnt++],spec,filter);

    while (fCnt < listSz && findNextDir(fileLst[fCnt++]));

    listSz = fCnt;
    
    if (sort && listSz > 1) qsort(fileLst,listSz,sizeof(AnsiFile),cmpFiles);
  }

  fndHdl = oldFndHdl;
  srchFilter = oldFilter;

  return fileLst;
}

//---------------------------------------------------------------------------
/** Searches the directory denoted by this object for files or directories.

  This method the search result in an internal list and returns
  a pointer to that list.\n

  This is a convenience method, based on methods findFirst() and
  findNext() where the result is optionally sorted and then stored in
  an internal list.

  \param listSz Will hold the number of files and directories found.
  \param sort If \c true the search result will be sorted based
  on function \ref compareAlphaNum(const char *s1, const char *s2)
  "compareAlphaNum".
  \param spec May contain a \e wildcard specification for the file or
  directory to search for.\n
  The only valid wildcard character is the asterisk (*).\n
  If \c NULL the string "*" is assumed.
  \param filter An optional additional user defined filter, may be \c NULL.
  \return A pointer to the internal list of found files and directories if
  <tt>listSz > 0</tt>.\n
  The pointer will be invalid if <tt>listSz < 1</tt>!!\n
  The returned pointer is only valid until the next call to this method
  or method getFileList() or getDirList().\n

  \attention
  Do \b NOT \c delete the list returned by this method!!

  \note
  If the path is empty it is first initialized to the current directory,\n
  <b>so beware, you might be searching the current directory</b>.
  \note
  The directories denoted by  "." or ".." will never be returned.
  \note
  This method leaves other search operations in progress intact.
*/

AnsiFile *AnsiFile::getList(int& listSz, bool sort, const char *spec,
                                                 AnsiFileFilter *filter) const
{
  listSz = 0;

  const AnsiFileFilter *oldFilter = srchFilter;

  intptr_t oldFndHdl = fndHdl;
  fndHdl = -1;
  srchFilter = NULL;

  AnsiFile f;
  if (findFirst(f,spec,filter)) {
    listSz++;
    while (findNext(f)) listSz++;
  }

  if (listSz > fileLstCap) {
    if (fileLst) delete[] fileLst;

    fileLstCap = listSz + 64;
    fileLst = new AnsiFile[fileLstCap];
  }

  if (listSz > 0) {
    fileLst = new AnsiFile[listSz];

    int fCnt = 0;
    findFirst(fileLst[fCnt++],spec,filter);

    while (fCnt < listSz && findNext(fileLst[fCnt++]));

    listSz = fCnt;
    
    if (sort && listSz > 1) qsort(fileLst,listSz,sizeof(AnsiFile),cmpFiles);
  }

  fndHdl = oldFndHdl;
  srchFilter = oldFilter;

  return fileLst;
}

//---------------------------------------------------------------------------
/** Returns the users home directory.
  \param homeDir Will receive the home directory if successfull, left
  unmodified otherwise.
  \return \c true if the operation was successfull,
  \c false otherwise.
*/

bool AnsiFile::getHomeDir(AnsiFile& homeDir)
{
  char pathBuf[1500] = "";
  unsigned long len = GetEnvironmentVariableA("HOMEPATH",pathBuf,1499);

  if (len < 1 || len >= 1499) {
    // Try another way

    if (SHGetFolderPathA(NULL,CSIDL_PERSONAL,NULL,
                                  SHGFP_TYPE_CURRENT,pathBuf)) return false;
  }

  homeDir = AnsiFile(pathBuf);

  return true;
}

//---------------------------------------------------------------------------
/** Returns the directory that holds the currently running executable.
  \param exeDir Will receive the directory of the executable if successfull,
  left unmodified otherwise.
  \return \c true if the operation was successfull, \c false otherwise.
*/

bool AnsiFile::getExeDir(AnsiFile& exeDir)
{
  char exeBuf[1500] = "";

  unsigned long len = GetModuleFileNameA(NULL,exeBuf,1499);
  if (len < 1 || len >=1499) return false;

  AnsiFile exeFile(exeBuf);
  return exeDir.getParent(exeFile);
}


//---------------------------------------------------------------------------
/** Gets the systems temporary directory.
  \param tmpDir Receives the temporary directory if successfull, left
  unmodified otherwise.
  \return \c true if successfull.
*/

bool AnsiFile::getTmpDir(AnsiFile& tmpDir)
{
  char pathBuf[1500] = "";
  unsigned long len = GetEnvironmentVariableA("TEMP",pathBuf,1499);

  if (len < 1 || len >= 1499)
                 len = GetEnvironmentVariableA("TMP",pathBuf,1499);

  if (len < 1 || len >= 1499) return false;

  tmpDir = AnsiFile(pathBuf);

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
