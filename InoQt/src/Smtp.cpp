//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Class for Smtp mail sending ---------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim International BV Aug 2007 ------------------------
//---------------------------------------------------------------------------
//------- C. Wolters  -------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Smtp.h"

#include "ZipOut.h"

#include "SmtpZipWriter.h"
#include "SmtpAttachmentWriter.h"

#ifndef _WIN32
#include "gsasl.h"
#endif

#include <QList>
#include <QEventLoop>
#include <QTimer>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>

namespace Ino {

//---------------------------------------------------------------------------

#ifdef _WIN32
static bool saslLibInit = false;
#endif

//---------------------------------------------------------------------------

static const QString boundTxt("INOFOR_SMTP_CLIENT_MESSAGE_BOUNDARY");

static const QString authMethodList[] =
  { "DIGEST-MD5","CRAM-MD5","NTLM","PLAIN","LOGIN" };

//---------------------------------------------------------------------------

struct Recipient
{
  QString niceName;
  QString email;

  Recipient(const QString& nice, const QString emailAddr);
};

//---------------------------------------------------------------------------

Recipient::Recipient(const QString& nice, const QString emailAddr)
: niceName(nice), email(emailAddr)
{
}

//---------------------------------------------------------------------------

static bool isAllAscii(const QString& s)
{
  int sz = s.size();

  for (int i=0; i<sz; ++i) {
    if (s[i] > 127) return false;

    if (s[i] < ' ') {
      if (s[i] == '\r' || s[i] == '\n' || s[i] == '\t') continue;

      return false;
    }
  }

  return true;
}

//---------------------------------------------------------------------------

static bool isValidEmailAddress(const QString& s)
{
  int sz = s.size();
  if (sz < 3) return false;

  int atIdx = s.indexOf('@');
  if (atIdx < 1 || atIdx >= sz-1 ) return false;

  return true;
}

//---------------------------------------------------------------------------
// Smtps Methods

void Smtp::logInput(const char *msg)
{
  if (!msg) return;

  QString s("<< %1");
  s = s.arg(msg);
  s.remove('\n');
  s.remove('\r');

  emit smtpLog(s);
}

//---------------------------------------------------------------------------

void Smtp::logOutput(const char *msg)
{
  if (!msg) return;

  QString qMsg(msg);
  qMsg.remove('\r');

  int sz = qMsg.size();

  if (sz > 0 && qMsg[sz-1] == '\n') qMsg.remove(sz-1,1);

  QStringList sl = qMsg.split('\n');

  QString s(">> %1");
  sz = sl.size();

  for (int i=0; i<sz; ++i) emit smtpLog(s.arg(sl[i]));
}

//---------------------------------------------------------------------------

void Smtp::logMsg(const QString& msg)
{
  if (msg.isEmpty()) return;

  QString s("-- %1");
  s = s.arg(msg);
  s.remove('\n');
  s.remove('\r');

  emit smtpLog(s);
}

//---------------------------------------------------------------------------

void Smtp::cleanup()
{
  txtHdrWritten = false;

  delete zipWrt; zipWrt = NULL;
  delete attWrt; attWrt = NULL;
}

//---------------------------------------------------------------------------

bool Smtp::isBusy() const
{
  return sock.state() != QAbstractSocket::UnconnectedState;
}

//---------------------------------------------------------------------------

bool Smtp::isConnected() const
{
  return sock.state() == QAbstractSocket::ConnectedState;
}

//---------------------------------------------------------------------------

bool Smtp::isClosed() const
{
  return !isBusy();
}

//---------------------------------------------------------------------------

void Smtp::sockReadyRead()
{
  if (sock.state() != QAbstractSocket::ConnectedState) return;
  
  if (evtLoop.isRunning() && sock.canReadLine()) evtLoop.exit();
}

//---------------------------------------------------------------------------

bool Smtp::readLineSub()
{
  bufSz = 0;
  if (lastError != ErrOk) return false;

  timer.start(readWriteTimeOut);

  while (!sock.canReadLine()) {
    if (sock.state() != QAbstractSocket::ConnectedState) {
      timer.stop();
      return false;
    }
    evtLoop.exec(); // Wait for data to come in
  }

  timer.stop();

  bufSz = sock.readLine(inBuf,sizeof(inBuf));

  if (bufSz < 0) {
    bufSz = 0;
    return false;
  }

  if (bufSz > 1 && inBuf[bufSz-1] == '\n') bufSz--;
  if (bufSz > 1 && inBuf[bufSz-1] == '\r') bufSz--;

  inBuf[bufSz] = '\0';

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::readLine()
{
  bufSz = 0;
  if (lastError != ErrOk) return false;

  if (readLineSub()) {
    logInput(inBuf);
    return true;
  }

  if (lastError == ErrOk) {
    lastError = ErrConnectionLost;
    logMsg(tr("Cant read from server."));
    emit progress(tr("Connection lost."));
  }

  sock.abort();
  cleanup();

  emit finished(false);

  return false;    
}

//---------------------------------------------------------------------------

int Smtp::msgCode()
{
  if (bufSz < 1) return 0;

  return atoi(inBuf);
}

//---------------------------------------------------------------------------

int Smtp::msgCode(const QString& msg)
{
  QString nr = msg.left(3);
  return nr.toInt();
}

//---------------------------------------------------------------------------

void Smtp::sockBytesWritten()
{
  if (sock.state() != QAbstractSocket::ConnectedState) return;
  
  if (evtLoop.isRunning() && sock.bytesToWrite() <= 2048) evtLoop.exit();
}

//---------------------------------------------------------------------------

bool Smtp::writeData(const char *buf, int sz)
{
  if (lastError != ErrOk) return false;
  if (!buf || sz < 1) return true;

  int wSz = sock.write(buf,sz);

  if (wSz < 0) return false;

  if (sock.bytesToWrite() <= 2048) return true;

  timer.start(readWriteTimeOut);

  while (sock.bytesToWrite() > 2048) {
    if (sock.state() != QAbstractSocket::ConnectedState) {
      timer.stop();
      return false;
    }
    evtLoop.exec(); // Wait for data to drain
  }

  timer.stop();

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::write(const char *buf, int sz)
{
  if (lastError != ErrOk) return false;

  if (writeData(buf,sz)) return true;

  if (lastError == ErrOk) {
    lastError = ErrConnectionLost;
    logMsg(tr("Cant write to server."));
    emit progress(tr("Connection lost."));
  }

  sock.abort();
  cleanup();

  emit finished(false);

  return false;    
}

//---------------------------------------------------------------------------

bool Smtp::flushData()
{
  timer.start(readWriteTimeOut);

  while (sock.bytesToWrite() > 0) {
    if (sock.state() != QAbstractSocket::ConnectedState) {
      timer.stop();
      return false;
    }
    evtLoop.exec();
  }

  timer.stop();

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::flush()
{
  if (flushData()) return true;

  if (lastError == ErrOk) {
    lastError = ErrConnectionLost;
    logMsg(tr("Cant write to server."));
    emit progress(tr("Connection lost."));
  }

  sock.abort();
  cleanup();

  emit finished(false);

  return false;
}

//---------------------------------------------------------------------------

bool Smtp::writeMsg(const QString& msg)
{
  return write(msg.toStdString().c_str(),msg.size());
}

//---------------------------------------------------------------------------

void Smtp::addingZipFile(const char *zipFile, const char *file)
{
  if (!zipFile || !file) return;

  QString msg = tr("  Adding file: %1 to %2.").arg(file).arg(zipFile); 

  emit progress(msg);
}

//---------------------------------------------------------------------------

bool Smtp::setLastError(Error err)
{
  lastError = err;

  return err == ErrOk;
}

//---------------------------------------------------------------------------

Smtp::Smtp(QObject *parent)
: QObject(parent), Writer(0),
  connectTimeOut(60000), readWriteTimeOut(60000),
  niceFrom(), emailFrom(), niceReplyTo(), emailReplyTo(),
  recipientList(*new QList<Recipient>()),
  emailSubject(),server(), login(), passwd(),
  sock(*new QTcpSocket(this)),
  evtLoop(*new QEventLoop(this)),
  timer(*new QTimer(this)),saslCtx(NULL),
  zipWrt(NULL), attWrt(NULL), txtHdrWritten(false),
  bufSz(0), lastError(ErrOk)
{
  QObject::connect(&sock,SIGNAL(hostFound()),SLOT(sockHostFound()));
  QObject::connect(&sock,SIGNAL(connected()),SLOT(sockConnected()));
  QObject::connect(&sock,SIGNAL(disconnected()),SLOT(sockDisconnected()));
  QObject::connect(&sock,SIGNAL(error(QAbstractSocket::SocketError)),
                           SLOT(sockError(QAbstractSocket::SocketError)));
  QObject::connect(&sock,SIGNAL(readyRead()),SLOT(sockReadyRead()));
  QObject::connect(&sock,SIGNAL(bytesWritten(qint64)),
                           SLOT(sockBytesWritten()));

  timer.setSingleShot(true);
  QObject::connect(&timer,SIGNAL(timeout()),SLOT(sockTimeOut()));

#ifndef _WIN32
  if (gsasl_init(&saslCtx) == GSASL_OK) {
    gsasl_callback_hook_set (saslCtx,this);
    gsasl_callback_set(saslCtx,(Gsasl_callback_function)saslCallBack);
  }
#endif
}

//---------------------------------------------------------------------------

Smtp::~Smtp()
{
  cleanup();

  delete &recipientList;

#ifndef _WIN32
  gsasl_done(saslCtx);
#endif
}

//---------------------------------------------------------------------------

bool Smtp::clear()
{
  if (isBusy()) return setLastError(ErrBusy);

  niceFrom      = "";
  emailFrom     = "";
  niceReplyTo   = "";
  emailReplyTo  = "";
  emailSubject  = "";
  server        = "";
  login         = "";
  passwd        = "";

  recipientList.clear();

  delete zipWrt;
  zipWrt         = NULL;

  delete attWrt;
  attWrt         = NULL;

  txtHdrWritten = false;
  bufSz          = 0;
  lastError      = ErrOk;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::setFrom(const QString& niceName, const QString& email)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (!niceName.isEmpty() && !isAllAscii(niceName))
    return setLastError(ErrNotAscii);

  if (email.isEmpty()) return setLastError(ErrMalformed);
  if (!isAllAscii(email)) return setLastError(ErrNotAscii);
  if (!isValidEmailAddress(email)) return setLastError(ErrMalformed);

  niceFrom  = niceName;
  emailFrom = email;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::setReplyTo(const QString& niceName, const QString& email)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (!niceName.isEmpty() && !isAllAscii(niceName))
    return setLastError(ErrNotAscii);

  if (email.isEmpty()) return setLastError(ErrMalformed);
  if (!isAllAscii(email)) return setLastError(ErrNotAscii);
  if (!isValidEmailAddress(email)) return setLastError(ErrMalformed);

  niceReplyTo  = niceName;
  emailReplyTo = email;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::addRecipient(const QString& niceName, const QString& email)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (!niceName.isEmpty() && !isAllAscii(niceName))
    return setLastError(ErrNotAscii);

  if (email.isEmpty()) return setLastError(ErrMalformed);
  if (!isAllAscii(email)) return setLastError(ErrNotAscii);
  if (!isValidEmailAddress(email)) return setLastError(ErrMalformed);

  recipientList.append(Recipient(niceName,email));

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::setSubject(const QString& subject)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (!isAllAscii(subject)) return setLastError(ErrNotAscii);

  emailSubject = subject;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::setServer(const QString& smtpServer, const QString& loginName,
                                                    const QString& password)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (smtpServer.isEmpty() || !isAllAscii(smtpServer))
    return setLastError(ErrInvalidServer);

  server = smtpServer;
  login  = loginName;
  passwd = password;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::setTimeOuts(int connectTimeOutMs, int readWriteTimeOutMs)
{
  if (isBusy()) return setLastError(ErrBusy);

  if (connectTimeOutMs < 100) connectTimeOutMs = 100;
  if (readWriteTimeOutMs < 100) readWriteTimeOutMs = 100;

  connectTimeOut   = connectTimeOutMs;
  readWriteTimeOut = readWriteTimeOutMs;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::checkConnectionData()
{
  if (emailFrom.isEmpty() || server.isEmpty() ||
    recipientList.size() < 1) return setLastError(ErrAddressingIncomplete);

  return true;
}

//---------------------------------------------------------------------------

void Smtp::sockHostFound()
{
  logMsg(tr("Connecting to: %1.").arg(server));
  emit progress(tr("Connecting ..."));
}

//---------------------------------------------------------------------------

void Smtp::sockConnected()
{
  if (evtLoop.isRunning()) evtLoop.exit();
}

//---------------------------------------------------------------------------

void Smtp::sockDisconnected()
{
  if (evtLoop.isRunning()) evtLoop.exit();
}

//---------------------------------------------------------------------------

void Smtp::sockError(QAbstractSocket::SocketError socketError)
{
  if (lastError != ErrOk) {
    if (evtLoop.isRunning()) evtLoop.exit();
    return;
  }

  lastError = ErrCantConnect;
  QString msg;

  switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
      msg = tr("No stmp server at: %1").arg(server);
      break;

    case QAbstractSocket::RemoteHostClosedError:
      lastError = ErrConnectionLost;
      msg       = tr("Connection lost.").arg(server);
      break;

    case QAbstractSocket::HostNotFoundError:
      msg = tr("Host %1 not found").arg(server);
      break;


    case QAbstractSocket::SocketTimeoutError:
      msg = tr("Connection attempt has timed out.");
      break;

    case QAbstractSocket::NetworkError:
      msg = tr("The network is not available or a network error has occurred.");
      break;

    case QAbstractSocket::DatagramTooLargeError:
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::AddressInUseError:
    case QAbstractSocket::SocketAddressNotAvailableError:
    case QAbstractSocket::UnsupportedSocketOperationError:
    case QAbstractSocket::UnknownSocketError:
    default:
      msg = tr("Cannot connect to %1 due to an internal error.").arg(server);
      break;
  }

  logMsg(msg);
  emit progress(msg);
  emit finished(false);

  if (evtLoop.isRunning()) evtLoop.exit();
}

//---------------------------------------------------------------------------

void Smtp::sockTimeOut()
{
  if (sock.state() == QAbstractSocket::ConnectedState) {
    lastError = ErrTimeOut;

    logMsg(tr("Timeout error."));
    emit progress(tr("Timeout error."));
  }
  else if (sock.state() == QAbstractSocket::ClosingState) {
  }
  else {
    lastError = ErrCantConnect;

    logMsg(tr("Connection attempt has timed out."));
    emit progress(tr("Cannot connect (time out)."));
  }

  sock.abort();
  emit finished(false);

  if (evtLoop.isRunning()) evtLoop.exit();
}

//---------------------------------------------------------------------------

void Smtp::quitFromServer()
{
  logMsg(tr("Quitting"));

  char msg[] = "QUIT\r\n";
  if (write(msg,strlen(msg))) {
    logOutput(msg);
    readLine();
  }

  if (sock.state() == QAbstractSocket::ConnectedState) {
    timer.start(readWriteTimeOut);
    sock.close();
    if (sock.state() != QAbstractSocket::UnconnectedState) evtLoop.exec();
    timer.stop();
  }
  
  if (lastError == ErrOk) emit finished(false);
}

//---------------------------------------------------------------------------

QString Smtp::fromDomain() const
{
  int idx = emailFrom.indexOf('@');
  if (idx < 0) return "";

  idx++;

  return emailFrom.right(emailFrom.size()-idx);
}

//---------------------------------------------------------------------------

bool Smtp::readEhloResponse(QStringList& answLst)
{
  for (;;) {
    if (!readLine()) return false;

    if (msgCode() != 250) {
      lastError = ErrCantConnect;
      emit progress(tr("Dont understand response from server, aborting."));
      quitFromServer();
      return false;
    }
    
    answLst.append(inBuf);

    if (bufSz < 4 || inBuf[3] != '-') return true;
  }
}

//---------------------------------------------------------------------------

bool Smtp::hasAuthMethod(const QStringList& answLst,
                                            const QString& method) const
{
#ifdef _WIN32
  return false;
#else
  for (int i=0; i<answLst.size(); ++i) {
    if (answLst[i].indexOf("AUTH",Qt::CaseInsensitive) < 0) continue;

    return answLst[i].contains(method,Qt::CaseInsensitive) &&
                         gsasl_client_support_p(saslCtx,method.toStdString().c_str());
  }

  return false;
#endif
}

//---------------------------------------------------------------------------

bool Smtp::hasKnownAuthMethod(QStringList& answLst) const
{
  int sz = sizeof(authMethodList)/sizeof(QString);

  for (int i=0; i<sz; ++i) {
   if (hasAuthMethod(answLst,authMethodList[i])) return true;
  }

  return false;
}

//---------------------------------------------------------------------------

int Smtp::saslCallBack(Gsasl *ctx, Gsasl_session *sctx, int prop)
{
#ifdef _WIN32
  return -1;
#else
  if (!sctx) return GSASL_NO_CALLBACK;

  Smtp *smtp = (Smtp *)gsasl_callback_hook_get(ctx);
  if (!smtp) return GSASL_NO_CALLBACK;

  Gsasl_property gProp = (Gsasl_property)prop;

  switch (gProp) {
    case GSASL_AUTHID:
      gsasl_property_set(sctx,gProp,smtp->login.toStdString().c_str());
      return GSASL_OK;

    case GSASL_AUTHZID:
      gsasl_property_set(sctx,gProp,"");
      return GSASL_OK;

    case GSASL_PASSWORD:
      gsasl_property_set(sctx,gProp,smtp->passwd.toStdString().c_str());
      return GSASL_OK;

    case GSASL_ANONYMOUS_TOKEN:
      gsasl_property_set(sctx,gProp,smtp->emailFrom.toStdString().c_str());
      return GSASL_OK;

    case GSASL_SERVICE:
      gsasl_property_set(sctx,gProp,"smtp");
      return GSASL_OK;

    case GSASL_HOSTNAME:
      gsasl_property_set(sctx,gProp,"Proliner");
      return GSASL_OK;

    case GSASL_REALM:
      gsasl_property_set(sctx,gProp,"");
      return GSASL_OK;

    case GSASL_GSSAPI_DISPLAY_NAME:
    case GSASL_PASSCODE:
    case GSASL_SUGGESTED_PIN:
    case GSASL_PIN:
    default:
      return GSASL_NO_CALLBACK;
  }

  return GSASL_OK;
#endif
}

//---------------------------------------------------------------------------

bool Smtp::authenticate(const QStringList& answLst, const QString& method)
{
#ifdef _WIN32
  return false;
#else
  if (!hasAuthMethod(answLst,method)) return false;

  QString auth = "AUTH %1\r\n";
  auth = auth.arg(method);

  logOutput(auth.toStdString().c_str());

  if (!write(auth.toStdString().c_str(),auth.size()) || !readLine()) return false;

  if (bufSz < 3 || msgCode() != 334) return false;
  
  Gsasl_session *session;

  if (gsasl_client_start(saslCtx,method.toStdString().c_str(),&session) != GSASL_OK)
                                                               return false;
  int rc = GSASL_OK;

  do {
    char *p = NULL;
    const char *b = inBuf+3;
    if (bufSz > 3 && *b == ' ') b++;

    rc = gsasl_step64(session,b,&p);

    if (rc == GSASL_NEEDS_MORE || rc == GSASL_OK) {
      logOutput(p);
      if (!write(p,strlen(p)) || !write("\r\n",2) || !readLine() || bufSz < 3) {
        free(p);
        gsasl_finish(session);
        return false;
      }

      free(p);
    }
  }
  while (rc == GSASL_NEEDS_MORE);

  gsasl_finish(session);

  if (rc == GSASL_OK && msgCode() == 235) {
    QString msg = "%1 authentication succeeded.";
    logMsg(msg.arg(method));
    return true;
  }
  else {
    if (write("*\r\n",3)) readLine();
    QString msg = "%1 authentication failed.";
    logMsg(msg.arg(method));
    return false;
  }
#endif
}

//---------------------------------------------------------------------------

bool Smtp::smtpConnect(bool& withoutAuth)
{
  withoutAuth = false;

  logMsg(tr("Logging in to: %1.").arg(server));
  emit progress(tr("Logging in ..."));

  if (!readLine()) return false;
  if (bufSz < 1 && !readLine()) return false;

  int code = msgCode();

  if (code < 1) { // Cant understand: give up
    lastError = ErrCantConnect;
    sock.abort();
    logMsg(tr("Funny initial server msg, aborting."));
    emit progress(tr("Dont understand server, aborting."));
    emit finished(false);

    return false;
  }

  if (code != 220) { // Server wont allow
    lastError = ErrCantConnect;

    logMsg(tr("Connection refused."));
    emit progress(tr("Connection refused (code %1), aborting.").arg(code));
    quitFromServer();

    return false;
  }

  // Send EHLO command
  QString domFrom = fromDomain();
  if (domFrom.isEmpty()) domFrom = "Me";

  QString msg = "EHLO %1\r\n";
  msg = msg.arg(domFrom);

  if (!write(msg.toStdString().c_str(),msg.size())) return false;
  logOutput(msg.toStdString().c_str());

  QStringList answLst;
  if (!readEhloResponse(answLst)) return false;

  if (answLst.size() < 1) {
    if (lastError == ErrOk) lastError = ErrCantConnect;

    logMsg(tr("Cant read/understand EHLO response."));
    emit progress(tr("Dont understand server %1, aborting.").arg(server));
    quitFromServer();

    return false;
  }

  if (login.isEmpty() || !hasKnownAuthMethod(answLst)) {
    withoutAuth = true;
    return true; // Try without logging in.
  }

  int sz = sizeof(authMethodList)/sizeof(QString);
  for (int i=0; i<sz; ++i) {
    if (authenticate(answLst,authMethodList[i])) return true;
  }

  lastError = ErrCantConnect;

  logMsg(tr("Failed to authenticate, giving up."));
  emit progress(tr("Cannot login with server, giving up."));

  quitFromServer();

  return false;
}

//---------------------------------------------------------------------------

bool Smtp::exchangeMsg(const QString& msg, int expectCode)
{
  logOutput(msg.toStdString().c_str());

  if (!write(msg.toStdString().c_str(),msg.size()) || !readLine()) return false;

  if (msgCode() != expectCode) {
    lastError = ErrNotAllowed;
    emit progress(tr("Cannot send mail data."));

    quitFromServer();

    return false;    
  }

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::writeHeaders()
{
  QString fromMsg("%1 <%2>");
  fromMsg=fromMsg.arg(niceFrom).arg(emailFrom);

  QString toMsg;
  for (int i=0; i<recipientList.size(); ++i) {
    if (i > 0) toMsg += ',';

    QString rec("%1 <%2>");
    toMsg += rec.arg(recipientList[i].niceName).arg(recipientList[i].email);
  }

  QDateTime dt = QDateTime::currentDateTime();
  QString dateMsg = dt.toString("d MMM yyyy hh:mm");

  QString header("From: %1\r\nTo: %2\r\nDate: %3\r\nSubject: %4\r\n"
                 "X-Mailer: Inofor Smtp Client\r\n"
                 "MIME-Version: 1.0\r\n"
                 "Content-Type: multipart/mixed;\r\n boundary=\"%5\"\r\n");
  
  header = header.arg(fromMsg).arg(toMsg).arg(dateMsg)
                              .arg(emailSubject).arg(boundTxt);
  int sz = header.size();

  logOutput(header.toStdString().c_str());

  if (!write(header.toStdString().c_str(),sz)) return false;

  return true;
}

//---------------------------------------------------------------------------

bool Smtp::connect()
{
  if (isBusy()) return setLastError(ErrBusy);
  if (!checkConnectionData()) return false;

  txtHdrWritten = false;

  lastError = ErrOk;

  timer.start(connectTimeOut);

  logMsg(tr("Looking up: %1.").arg(server));
  emit progress(tr("Looking up: %1.").arg(server));

  sock.connectToHost(server,25);

  evtLoop.exec();
  timer.stop();

  if (lastError != ErrOk) {
    sock.abort();
    emit finished(false);
    return false;
  }

  // The socket is connected, start authentication on the smtp server:
  
  bool withoutAuth = false;
  if (!smtpConnect(withoutAuth)) return false;

  // Emit the MAIL FROM, RCPT TO and DATA commands

  if (withoutAuth)
       emit progress(tr("Trying without login."));
  else emit progress(tr("Login succeeded."));

  QString msg("MAIL FROM: <%1>\r\n");
  if (!exchangeMsg(msg.arg(emailFrom),250)) return false;

  for (int i=0; i<recipientList.size(); ++i) {
    msg = "RCPT TO: <%1>\r\n";
    if (!exchangeMsg(msg.arg(recipientList[i].email),250)) return false;
  }

  msg = "DATA\r\n";
  if (!exchangeMsg(msg,354)) return false;

  // Write the mailheaders:

  if (!writeHeaders()) return false;

  return true;
}

//---------------------------------------------------------------------------

void Smtp::checkAttFinished()
{
  if (zipWrt) {
    zipWrt->close();
    delete zipWrt;
    zipWrt = NULL;
  }

  if (attWrt) {
    attWrt->close();
    delete attWrt;
    attWrt = NULL;
  }
}

//---------------------------------------------------------------------------

bool Smtp::writeTxtHeader()
{
  if (txtHdrWritten) return true;

  txtHdrWritten = true;

  QString bnd("\r\n--"); bnd += boundTxt; bnd += "\r\n";
  if (!write(bnd.toStdString().c_str(),bnd.size())) return false;
  logOutput(bnd.toStdString().c_str());

  char msg[] = "Content-Type: text/plain;\r\n charset=\"iso-8859-1\"\r\n"
               "Content-Transfer-Encoding: quoted-printable\r\n\r\n";

  bool ok = write(msg,strlen(msg));
  if (ok) logOutput(msg);

  return ok;
}

//---------------------------------------------------------------------------

bool Smtp::writeMsgLine(const QString& msgLine)
{
  if (!isConnected()) return setLastError(ErrNotConnected);
  if (!isAllAscii(msgLine)) return setLastError(ErrNotAscii);

  lastError = ErrOk;

  checkAttFinished();

  if (!txtHdrWritten && !writeTxtHeader()) return false;

  QString msg(msgLine);
  msg.replace("\r\n.","\r\n..");

  if (!msg.endsWith("\r\n")) msg.append("\r\n");

  return write(msg.toStdString().c_str(),msg.size());    
}

//---------------------------------------------------------------------------

ZipOut *Smtp::attachZip(const QString& name)
{
  if (!isConnected()) {
    lastError = ErrNotConnected;
    return NULL;
  }

  lastError = ErrOk;
  txtHdrWritten = false;

  checkAttFinished();

  QFileInfo fi(name);
  QString zipName = fi.fileName();

  emit progress(tr("Attaching zip file: %1.").arg(zipName));

  zipWrt = new SmtpZipWriter(*this,zipName.toStdString().c_str(), boundTxt.toStdString().c_str());

  return zipWrt;
}

//---------------------------------------------------------------------------

Writer *Smtp::attach(const QString& name)
{
  if (!isConnected()) {
    lastError = ErrNotConnected;
    return NULL;
  }

  txtHdrWritten = false;
  lastError = ErrOk;

  checkAttFinished();

  QFileInfo fi(name);
  QString attName = fi.fileName();

  emit progress(tr("Attaching file: %1.").arg(attName));

  attWrt = new SmtpAttachmentWriter(*this,attName.toStdString().c_str(),
                                                         boundTxt.toStdString().c_str());
  return attWrt;
}

//---------------------------------------------------------------------------

bool Smtp::disconnect()
{
  if (!isConnected()) {
    if (lastError == ErrOk) return setLastError(ErrNotConnected);
    return false;
  }

  checkAttFinished();
  txtHdrWritten = false;
  lastError = ErrOk;

  QString bnd = "\r\n--"; bnd += boundTxt; bnd += "--\r\n.\r\n";
  if (!write(bnd.toStdString().c_str(),bnd.size())) return false;
  logOutput(bnd.toStdString().c_str());

  if (!readLine()) return false;

  bool success = msgCode() == 250;

  if (!success) progress(tr("Server wont accept email"));

  emit progress(tr("Disconnecting ..."));

  char msg[] = "QUIT\r\n";
  if (write(msg,strlen(msg))) {
    logOutput(msg);
    readLine();
  }

  if (sock.state() == QAbstractSocket::ConnectedState) {
    timer.start(readWriteTimeOut);
    sock.close();
    if (sock.state() != QAbstractSocket::UnconnectedState) evtLoop.exec();
    timer.stop();
  }
  
  logMsg(tr("Disconnected."));
  logMsg("\n");

  emit progress("\n");

  if (success)
       emit progress(tr("Email has been sent."));
  else emit progress(tr("Email has NOT been sent."));

  emit finished(success);

  if (!success) lastError = ErrCantSendMsg;

  return success;
}

//---------------------------------------------------------------------------

void Smtp::abort()
{
  if (!isBusy()) return;

  logMsg(tr("Aborting connection."));
  emit progress(tr("Aborting email transfer."));

  lastError = ErrAborted;
  timer.stop();
  sock.abort();
  if (evtLoop.isRunning()) evtLoop.exit();
}

} // namespace

//---------------------------------------------------------------------------
