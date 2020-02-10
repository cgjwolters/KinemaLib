//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Creates a zip formatted output stream -----------------------------
//------- (Files cannot be NOT compressed (yet)) ----------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV SEP 2007 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "ZipOut.h"

#include "Exceptions.h"

#include "zlib.h"

#include <time.h>
#include <cstring>
#include <cstdlib>

namespace Ino {

//---------------------------------------------------------------------------

class ZipFile
{
  char *path;

  void dosDateTime();

  ZipFile(const ZipFile& cp);
  ZipFile& operator=(const ZipFile& src);

public:
  long startPos;
  long rawSz, sz, crc;
  short dosDate, dosTime;
  bool compressed;
  bool written;

  ZipFile(long curPos, const char* filePath, bool compressing);
  ~ZipFile();

  const char *getPath() const { return path; }
};

//---------------------------------------------------------------------------

class ZipFileList
{
  ZipFile **lst;
  int cap,sz;

  void ensureCapacity(int newCap);

  ZipFileList(const ZipFileList& cp);             // No copying
  ZipFileList& operator=(const ZipFileList& src); // No assignment

public:
  ZipFileList();
  ~ZipFileList();

  int size() const { return sz; }

  ZipFile& append(long curPos, const char* path, bool compressing);

  ZipFile& operator[](int idx);
};

//---------------------------------------------------------------------------

class ZipWriter : public Writer
{
  Writer& wrt;

  ZipWriter(const ZipWriter& cp);
  ZipWriter& operator=(const ZipWriter& src);

public:
  ZipWriter(Writer& writer);

  Writer& getWriter() { return wrt; }

  virtual bool isClosed() const { return wrt.isClosed(); }
  virtual bool isAborted() const { return wrt.isAborted(); }
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return wrt.isBuffered(); }

  virtual bool write(const char *buf, int sz);
  virtual bool flush() { return wrt.flush(); }
};

//---------------------------------------------------------------------------
//------- ZipFile Methods ----------------------------------------------------
//---------------------------------------------------------------------------

void ZipFile::dosDateTime()
{
  time_t now = time(NULL);

  struct tm *tmS = localtime(&now);

  dosDate  =  tmS->tm_mday & 0x1f;
  dosDate |= ((tmS->tm_mon+1) & 0x0f) << 5;
  dosDate |= ((tmS->tm_year+80) & 0x7f) << 9;

  dosTime  = (tmS->tm_sec/2) & 0x1f;
  dosTime |= (tmS->tm_min & 0x3f) << 5;
  dosTime |= (tmS->tm_hour & 0x1f) << 11;
}

//---------------------------------------------------------------------------

ZipFile::ZipFile(long curPos, const char* filePath, bool compressing)
: path(dupStr(filePath)), startPos(curPos), rawSz(0), sz(0), crc(0),
  dosDate(0), dosTime(0), compressed(compressing), written(false)
{
  if (!path) return;

  char *p = path;

  while (*p) {
    if (*p == '\\') *p = '/';
    ++p;
  }

  int sLen = strlen(path);
  if (sLen > 1 && path[1] == ':') memmove(path,path+2,sLen-1);

  sLen = strlen(path);
  if (sLen > 0 && path[0] == '/') memmove(path,path+1,sLen);

  dosDateTime();

  crc = crc32(0,Z_NULL,0);
}

//---------------------------------------------------------------------------

ZipFile::~ZipFile()
{
  delete[] path;
}

//---------------------------------------------------------------------------
//------- ZipFileList Methods ------------------------------------------------
//---------------------------------------------------------------------------

void ZipFileList::ensureCapacity(int newCap)
{
  if (cap >= newCap) return;

  cap = newCap + 256;

  lst = (ZipFile **)realloc(lst,cap*sizeof(ZipFile *));
}

//---------------------------------------------------------------------------

ZipFileList::ZipFileList()
:  lst(NULL), cap(0), sz(0)
{
}

//---------------------------------------------------------------------------

ZipFileList::~ZipFileList()
{
  for (int i=0; i<sz; ++i) delete lst[i];

  delete[] lst;
}

//---------------------------------------------------------------------------

ZipFile& ZipFileList::append(long curPos, const char* path, bool compressing)
{
  ensureCapacity(sz+1);

  lst[sz++] = new ZipFile(curPos,path,compressing);

  return *lst[sz-1];
}

//---------------------------------------------------------------------------

ZipFile& ZipFileList::operator[](int idx)
{
  if (idx < 0 || idx >= sz)
             throw IndexOutOfBoundsException("ZipFileList::operator[]");

  return *lst[idx];
}

//---------------------------------------------------------------------------
//------- ZipWriter Methods -------------------------------------------------
//------- Mainly serves as a byte counter -----------------------------------
//---------------------------------------------------------------------------

ZipWriter::ZipWriter(Writer& writer)
: Writer(NULL), wrt(writer)
{
}

//---------------------------------------------------------------------------

bool ZipWriter::write(const char *buf, int sz)
{
  if (!buf || sz < 0) return false;

  if (!wrt.write(buf,sz)) return false;

  bytesWritten += sz;
  bytesInc     += sz;

  return true;
}

//---------------------------------------------------------------------------
//------- ZipOut Methods ----------------------------------------------------
//---------------------------------------------------------------------------

ZipOut::ZipOut(Writer& writer)
: Writer(NULL), wrt(*new ZipWriter(writer)),
  bWrt(*new ByteArrayWriter(100*1024,0)),
  cWrt(*new CompressedWriter(bWrt,4096)),
  dWrt(*new DataWriter(wrt)),
  fileLst(*new ZipFileList),
  closed(false)
{
  dWrt.writeLittleEndian();
}

//---------------------------------------------------------------------------

ZipOut::~ZipOut()
{
  close();
  
  delete &fileLst;
  delete &dWrt;
  delete &bWrt;
  delete &cWrt;
  delete &wrt;
}

//---------------------------------------------------------------------------

Writer& ZipOut::getWriter()
{
  return wrt.getWriter();
}

//---------------------------------------------------------------------------

bool ZipOut::isClosed() const
{
  return closed || wrt.isClosed();
}

//---------------------------------------------------------------------------

bool ZipOut::isAborted() const
{
  return wrt.isAborted();
}

//---------------------------------------------------------------------------

long ZipOut::getErrorCode() const
{
  return wrt.getErrorCode();
}

//---------------------------------------------------------------------------

bool ZipOut::isBuffered() const
{
  return wrt.isBuffered();
}

//---------------------------------------------------------------------------

bool ZipOut::writeLastFile()
{
  int sz = fileLst.size();
  if (sz < 1) return true;

  ZipFile& zf = fileLst[sz-1];
  if (zf.written) return true;

  if (zf.compressed) cWrt.flush();

  char *dataBuf = bWrt.getBuffer();

  if (zf.compressed) {
    cWrt.flush();
    
    zf.rawSz = cWrt.getRawBytesWritten();
    zf.sz    = cWrt.getBytesWritten();

    // Remove zlib header and trailer

    if (zf.sz >= 2) {
      if (dataBuf[1] & 0x20) {
        zf.sz -= 10;
        dataBuf += 6;
      }
      else {
        zf.sz -= 6;
        dataBuf += 2;
      }
    }

    if (zf.sz < 0) {
      zf.sz    = 0;
      zf.rawSz = 0;
    }
  }
  else {
    zf.sz    = bWrt.getSize();
    zf.rawSz = zf.sz;
  }

  zf.written = true;

  if (!dWrt.writeInt(0x04034b50)) return false; // File hdr Signature
  if (!dWrt.writeShort(20)) return false;       // Minimum required version
  if (!dWrt.writeShort(0)) return false;        // General purpose flags

  if (zf.compressed) {
    if (!dWrt.writeShort(8)) return false;        // Compression method (none)
  }
  else if (!dWrt.writeShort(0)) return false;

  if (!dWrt.writeShort(zf.dosTime)) return false; // File modification time
  if (!dWrt.writeShort(zf.dosDate)) return false; // File modification date

  if (!dWrt.writeInt(zf.crc)) return false;   // Crc
  if (!dWrt.writeInt(zf.sz)) return false;   // Compressed size
  if (!dWrt.writeInt(zf.rawSz)) return false;   // Uncompressed size

  size_t pathLen = 0;
  if (zf.getPath()) pathLen = strlen(zf.getPath());
  if (!dWrt.writeShort((short)pathLen)) return false;   // Path name length

  if (!dWrt.writeShort(0)) return false;         // Extra field length

  if (pathLen > 0) {
    if (!dWrt.write(zf.getPath(),pathLen)) return false; // Path name
  }

  // Write the file data:
  return wrt.write(dataBuf,zf.sz);
}

//---------------------------------------------------------------------------

bool ZipOut::addFile(const char *path, bool compressed)
{
  if (isClosed()) return false;

  if (!writeLastFile()) return false;

  bWrt.setSize(0);
  cWrt.resetBytesWritten();

  int curPos = wrt.getBytesWritten();
  fileLst.append(curPos,path,compressed);

  return true;
}

//---------------------------------------------------------------------------

bool ZipOut::write(const char *buf, int sz)
{
  if (isClosed()) return false;

  int fSz = fileLst.size();
  if (fSz < 1) return false;

  ZipFile& zf = fileLst[fSz-1];
  zf.crc = crc32(zf.crc,(const Bytef *)buf,sz);

  if (zf.compressed)
       return cWrt.write(buf,sz);
  else return bWrt.write(buf,sz);
}

//---------------------------------------------------------------------------

bool ZipOut::writeDirectory()
{
  int startPos = wrt.getBytesWritten();

  int fSz = fileLst.size();

  for (int i=0; i<fSz; ++i) {
    ZipFile& zf = fileLst[i];

    if (!dWrt.writeInt(0x02014b50)) return false; // Hdr Signature
    if (!dWrt.writeShort(0)) return false;        // Version made by
    if (!dWrt.writeShort(20)) return false;       // Minimum required version
    if (!dWrt.writeShort(0)) return false;        // General purpose flags

    if (zf.compressed) {
      if (!dWrt.writeShort(8)) return false;        // Compression method (none)
    }
    else if (!dWrt.writeShort(0)) return false;

    if (!dWrt.writeShort(zf.dosTime)) return false; // File modification time
    if (!dWrt.writeShort(zf.dosDate)) return false; // File modification date

    if (!dWrt.writeInt(zf.crc)) return false;
    if (!dWrt.writeInt(zf.sz)) return false;
    if (!dWrt.writeInt(zf.rawSz)) return false;

    size_t pathLen = 0;
    if (zf.getPath()) pathLen = strlen(zf.getPath());
    if (!dWrt.writeShort((short)pathLen)) return false;   // Path name length

    if (!dWrt.writeShort(0)) return false;         // Extra field length
    if (!dWrt.writeShort(0)) return false;         // File comment length
    if (!dWrt.writeShort(0)) return false;         // Disk number start
    if (!dWrt.writeShort(0)) return false;         // Internal file attributes
    if (!dWrt.writeInt(0))   return false;         // External file attributes
    if (!dWrt.writeInt(zf.startPos)) return false; // Local header offset

    if (pathLen > 0) {
      if (!dWrt.write(zf.getPath(),pathLen)) return false; // Path name
    }
  }

  int curPos = wrt.getBytesWritten();

  if (!dWrt.writeInt(0x06054b50)) return false; // End of central dir signature
  if (!dWrt.writeShort(0)) return false;        // Number of this disk
  if (!dWrt.writeShort(0)) return false;        // Number of the disk with the centrel dir
  if (!dWrt.writeShort((short)fSz)) return false;      // Number of files in the dir on this disk
  if (!dWrt.writeShort((short)fSz)) return false;      // Number of files in the dir

  if (!dWrt.writeInt(curPos-startPos)) return false; // Size of central dir

  if (!dWrt.writeInt(startPos)) return false;   // Startpos of central dir
  if (!dWrt.writeShort(0)) return false;        // Comment length

  return true;
}

//---------------------------------------------------------------------------

bool ZipOut::close()
{
  if (closed) return true;
  closed = true;

  bool ok = writeLastFile() && writeDirectory();

  wrt.flush();

  return ok;
}

} // namespace Ino

//---------------------------------------------------------------------------
