//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General File Manipulation Object ----------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2000 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INO_UNI_FILE_INC
#define INO_UNI_FILE_INC

#include <string>
#include <wchar.h>

#ifdef __GNUC__
#include <sys/io.h>
#else
#include <io.h>
#endif

#ifndef _WIN32
#define intptr_t long
#endif

namespace Ino
{

using namespace std;

class UniFile;

//---------------------------------------------------------------------------

class UniFileFilter
{
public:
  virtual ~UniFileFilter() {}

  virtual bool accept(const UniFile& fileOrDir) const { &fileOrDir; return true; }
};

//---------------------------------------------------------------------------

class UniFile
{
  enum FindMode { FindAll, FindFiles, FindDirs };

  mutable intptr_t fndHdl;
  mutable FindMode fndMode;
  mutable struct _wfinddata_t findInfo;
  mutable const UniFileFilter *srchFilter;

  mutable wchar_t *pathStr;
  mutable int pathStrSz, pathStrCap;

  mutable UniFile *fileLst;
  mutable int fileLstCap;

  void init() const;
  bool findFirstAny(const wchar_t *spec) const;

public:
  UniFile();
  UniFile(const UniFile& cp);

  UniFile(const wchar_t *path);
  UniFile(const UniFile& parent, const wchar_t *path);
  ~UniFile();

  UniFile& operator=(const UniFile& src);

  UniFile& operator+=(const UniFile& wf);
  operator const wchar_t*() const { return pathStr; }

  bool operator==(const UniFile& file) const;

  bool exists() const;
  bool canRead() const;
  bool canWrite() const;
  bool canRemove() const;

  bool isFile() const;
  bool isDirectory() const;
  bool isRootDir() const;
  bool hasExtension() const;

  bool isRelative() const;

  __int64 getSize() const;

  long getCreationDate() const;
  long getModifiedDate() const;

  void getName(wstring& fileName) const;
  void getFileTitle(wstring& title) const;
  void getExtension(wstring& extension) const;

  bool setName(const wchar_t *newName);
  bool setFileTitle(const wchar_t *newTitle);
  bool setExtension(const wchar_t *newExt);

  bool moveTo(const UniFile& newUniFile);
  bool copyTo(const UniFile& newUniFile);

  bool setAsCurrentDir();
  bool createDir(bool recursive=false);
  bool createFile(bool recursive=false);

  bool remove();

  void getPath(wstring& path);
  const wchar_t *getPath() const;

  bool getParent();
  bool getParent(UniFile& parent) const;

  bool getAbsoluteFile(UniFile& af) const;

  bool findFirstDir(UniFile& dir, const wchar_t *spec=NULL,
                                          UniFileFilter *filter=NULL) const;
  bool findNextDir(UniFile& dir) const;

  bool findFirstFile(UniFile& file, const wchar_t *spec=NULL,
                                          UniFileFilter *filter=NULL) const;
  bool findNextFile(UniFile& file) const;

  bool findFirst(UniFile& fileOrDir, const wchar_t *spec=NULL,
                                          UniFileFilter *filter=NULL) const;
  bool findNext(UniFile& fileOrDir) const;

  void cancelFind() const;

  UniFile *getFileList(int& listSz, bool sort=false,
                       const wchar_t *spec=NULL, UniFileFilter *filter=NULL) const;
  UniFile *getDirList(int& listSz, bool sort=false,
                       const wchar_t *spec=NULL, UniFileFilter *filter=NULL) const;
  UniFile *getList(int& listSz, bool sort=false,
                        const wchar_t *spec=NULL,UniFileFilter *filter=NULL) const;

  static bool getHomeDir(UniFile& homeDir);
  static bool getExeDir(UniFile& exeDir);
  static bool getTmpDir(UniFile& tmpDir);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
