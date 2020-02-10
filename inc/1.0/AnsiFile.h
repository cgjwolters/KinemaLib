//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General File Manipulation Object ----------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2000 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INO_ANSI_FILE_INC
#define INO_ANSI_FILE_INC

#include <string>

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

class AnsiFile;

//---------------------------------------------------------------------------

class AnsiFileFilter
{
public:
  virtual ~AnsiFileFilter() {}

  virtual bool accept(const AnsiFile& fileOrDir) const { &fileOrDir; return true; }
};

//---------------------------------------------------------------------------

class AnsiFile
{
  enum FindMode { FindAll, FindFiles, FindDirs };

  mutable intptr_t fndHdl;
  mutable FindMode fndMode;
  mutable struct _finddata_t findInfo;
  mutable const AnsiFileFilter *srchFilter;

  mutable char *pathStr;
  mutable int pathStrSz, pathStrCap;

  mutable AnsiFile *fileLst;
  mutable int fileLstCap;

  void init() const;
  bool findFirstAny(const char *spec) const;

public:
  AnsiFile();
  AnsiFile(const AnsiFile& cp);

  AnsiFile(const char *path);
  AnsiFile(const AnsiFile& parent, const char *path);
  ~AnsiFile();

  AnsiFile& operator=(const AnsiFile& src);

  AnsiFile& operator+=(const AnsiFile& wf);
  operator const char*() const { return pathStr; }

  bool operator==(const AnsiFile& file) const;

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

  void getName(string& fileName) const;
  void getFileTitle(string& title) const;
  void getExtension(string& extension) const;

  bool setName(const char *newName);
  bool setFileTitle(const char *newTitle);
  bool setExtension(const char *newExt);

  bool moveTo(const AnsiFile& newAnsiFile);
  bool copyTo(const AnsiFile& newAnsiFile);

  bool setAsCurrentDir();
  bool createDir(bool recursive=false);
  bool createFile(bool recursive=false);

  bool remove();

  void getPath(string& path);
  const char *getPath() const;

  bool getParent();
  bool getParent(AnsiFile& parent) const;

  bool getAbsoluteFile(AnsiFile& af) const;

  bool findFirstDir(AnsiFile& dir, const char *spec=NULL,
                                  AnsiFileFilter *filter=NULL) const;
  bool findNextDir(AnsiFile& dir) const;

  bool findFirstFile(AnsiFile& file, const char *spec=NULL,
                                  AnsiFileFilter *filter=NULL) const;
  bool findNextFile(AnsiFile& file) const;

  bool findFirst(AnsiFile& fileOrDir, const char *spec=NULL,
                                  AnsiFileFilter *filter=NULL) const;
  bool findNext(AnsiFile& fileOrDir) const;

  void cancelFind() const;

  AnsiFile *getFileList(int& listSz, bool sort=false,
                        const char *spec=NULL, AnsiFileFilter *filter=NULL) const;
  AnsiFile *getDirList(int& listSz, bool sort=false,
                       const char *spec=NULL,  AnsiFileFilter *filter=NULL) const;
  AnsiFile *getList(int& listSz, bool sort=false,
                        const char *spec=NULL, AnsiFileFilter *filter=NULL) const;

  static bool getHomeDir(AnsiFile& homeDir);
  static bool getExeDir(AnsiFile& exeDir);
  static bool getTmpDir(AnsiFile& tmpDir);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
