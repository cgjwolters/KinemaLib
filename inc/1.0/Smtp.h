//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Class for Smtp mail sending ---------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim International BV Aug 2007 ------------------------
//---------------------------------------------------------------------------
//------- C. Wolters  -------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef SMTP_INC
#define SMTP_INC

#include "Writer.h"
#include "ZipOut.h"

#include <QObject>
#include <QTcpSocket>

//---------------------------------------------------------------------------

class QEventLoop;
class QTimer;
class QStringList;

struct Gsasl;
struct Gsasl_session;

namespace Ino {

struct Recipient;
class Writer;
class ZipOut;

class Smtp : public QObject, private Writer
{
  Q_OBJECT

public:
  enum Error { ErrOk=0, ErrBusy, ErrNotAscii,
               ErrMalformed, ErrAddressingIncomplete, ErrInvalidServer,
               ErrNotConnected, ErrCantConnect, ErrWrongAuth,
               ErrConnectionLost, ErrNotAllowed, ErrTimeOut,
               ErrCantSendMsg, ErrAborted};

private:
  int connectTimeOut, readWriteTimeOut;

  QString niceFrom, emailFrom;
  QString niceReplyTo, emailReplyTo;
  QList<Recipient>& recipientList;
  QString emailSubject;
  QString server, login, passwd;

  QTcpSocket& sock;
  QEventLoop& evtLoop;
  QTimer& timer;

  Gsasl *saslCtx;

  ZipOut *zipWrt;
  Writer *attWrt;
  bool txtHdrWritten;

  char inBuf[1500];
  int bufSz;

  Error lastError;

  FILE *fd;

  void logInput(const char *logMsg);
  void logOutput(const char *logMsg);
  void logMsg(const QString& logMsg);

  void cleanup();
  bool readLineSub();
  bool writeData(const char *buf, int sz);
  bool flushData();

  virtual bool isClosed() const;
  virtual bool isAborted() const { return false; }
  virtual long getErrorCode() const { return 0; };
  virtual bool isBuffered() const { return true; };

  bool readLine();
  int msgCode();
  static int msgCode(const QString& msg);
  virtual bool write(const char *buf, int sz);
  virtual bool flush();

  bool writeMsg(const QString& msg);

  void addingZipFile(const char *zipFile, const char *file);

  bool setLastError(Error err);
  bool checkConnectionData();
  void quitFromServer();
  QString fromDomain() const;
  bool readEhloResponse(QStringList& answLst);

  static int saslCallBack(Gsasl *ctx, Gsasl_session *sctx, int prop);

  bool hasAuthMethod(const QStringList& answLst,
                                            const QString& method) const;
  bool hasKnownAuthMethod(QStringList& answLst) const;
  bool authenticate(const QStringList& answLst, const QString& method);

  bool smtpConnect(bool& withoutAuth);

  bool exchangeMsg(const QString& msg, int expectCode);
  void checkAttFinished();
  bool writeHeaders();
  bool writeTxtHeader();

  Smtp(const Smtp& cp);             // No Copying
  Smtp& operator=(const Smtp& src); // No Assignment

private slots:
  void sockHostFound();
  void sockConnected();
  void sockDisconnected();
  void sockError(QAbstractSocket::SocketError socketError);

  void sockTimeOut();

  void sockReadyRead();
  void sockBytesWritten();

signals:
  void progress(QString msg);
  void finished(bool success);
  
  void smtpLog(QString serverMsg);

public:
  Smtp(QObject *parent=NULL);
  virtual ~Smtp();

  Error getLastError() const;

  bool isBusy() const;
  bool isConnected() const;

  bool clear();

  bool setFrom(const QString& niceName, const QString& email);
  bool setReplyTo(const QString& niceName, const QString& email);
  bool addRecipient(const QString& niceName, const QString& email);
  bool setSubject(const QString& subject);
  bool setServer(const QString& smtpServer, const QString& loginName,
                                                const QString& password);

  bool setTimeOuts(int connectTimeOut, int readWriteTimeOut);

  bool connect();

  bool writeMsgLine(const QString& msgLine);

  ZipOut *attachZip(const QString& name);
  Writer *attach(const QString& name);

  bool disconnect();

  void abort();

  friend class SmtpConn;
  friend class SmtpZipWriter;
  friend class SmtpAttachmentWriter;
};

} // namespace

//---------------------------------------------------------------------------
#endif
