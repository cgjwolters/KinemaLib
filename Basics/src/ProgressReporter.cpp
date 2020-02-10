//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General Progress Reporter -----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Jul 2005 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup inolib InoforLibs
@{
*/

/** \addtogroup general_functions General Functions */
/** \addtogroup templates Templates */
/** \addtogroup exceptions Exception Classes */
/** \addtogroup file_io File System Classes */
/** \addtogroup stream_io Stream I/O */
/** \addtogroup encoding Encoding/Decoding */
/** \addtogroup geometry 2D/3D Geometry */
/** \addtogroup mswindows MS Windows Specific */

/**
@}
*/

//---------------------------------------------------------------------------
/** \addtogroup stream_io Stream I/O
@{
*/

/** \addtogroup stream_writer Stream Writers */
/** \addtogroup stream_reader Stream Readers */
/** \addtogroup persistence Persistence */

//---------------------------------------------------------------------------
/** \class ProgressReporter
    Reports the progress a Reader or a Writer makes.

    This pure virtual class can for instance be used to show progress in a
    progress bar, while a file is being read, written or downloaded.\n
    \n
    Derive a class from this class and implement method
    progressReport(int progScale). Then pass a pointer to an instance of
    that class to one of the Reader or Writer derived classes.\n
    The reader or writer will then report its progress to this class by
    calling method \ref setProgress(int progress) "setProgress"
    at regular intervals.\n
    That indirectly causes your implementation of method
    \ref progressReport(int progScale) "progressReport" to be called.\n
    \n
    It is possible to abort a read or write operation prematurely (such as
    when a user hits the Cancel button) by returning \c false from
    your implementation of \ref progressReport(int progScale)
    "progressReport". That will tell the Reader or Writer to stop reading
    or writing and return -1 from the \ref Reader::read(char *, int)
    "read" or \ref Writer::write(const char *, int) "write" method.

    \author C. Wolters
    \date Jul 2005
*/

/**
@}
*/

//---------------------------------------------------------------------------
/** Constructor.
   \param maxProgress The progress value (made by a Reader or Writer) that
   indicates completion. (e.g. the size of a file to read or write).\n
   If less than one, the value is silently set to one.
   \param reportIncrement The minimum progress made by a Reader or Writer
   between successive calls to method \ref progressReport(int progScale)
   "progressReport".\n
   If less than one, the value is silently set to one.
   \param maxScale The (rescaled) progress value that indicates
   completion (100 %), see the details below.\n
   If less than one, the value is silently set to one.

   Parameters \c maxProgress and \c progressIncrement are the values the
   Reader or Writer class is dealing with.\n
   \c maxScale, on the other hand, is the maximum value that will ever
   be supplied to your implementation of \ref progressReport(int progScale)
   "progressReport".\n
   \n
   The value that is reported to \c progressReport is
   calculated as: <tt>progress / maxProgress * maxScale</tt>.
   \par Example:
   A file of 100000 bytes is to be read with a report increment of
   2000. Parameter \c maxScale is set at 100.\n
   \n
   Then \c progressReport will be called with its parameter increasing
   from 0 to 100 as the Reader progresses from 0 to 100000.\n
   The Reader will update this ProgressReporter every time 2000
   bytes have been read.
*/

ProgressReporter::ProgressReporter(long maxProgress, long reportIncrement,
                                                             long maxScale)
: reportInc(reportIncrement), maxProg(maxProgress), maxSc(maxScale),
  curProg(0), curSc(0), abort(false)
{
  if (reportInc < 1) reportInc = 1;
  if (maxProg < 1) maxProg   = 1;
  if (maxScale < 1) maxScale = 1;
}

//---------------------------------------------------------------------------
/** Resets the state of this progress reporter.
  The abort status is set to \c false and the progress value is set to zero.
  \param maxProgress The new value that indicates Reader or Writer completion.
  \param maxScale The new value that indicates completion in your
  implementation of method \ref progressReport(int progScale) "progressReport".
*/

void ProgressReporter::reset(long maxProgress, long maxScale)
{
  abort = false;
  curProg = 0;
  curSc = 0;

  maxProg = maxProgress;
  maxSc   = maxScale;

  if (maxProg < 1)  maxProg  = 1;
  if (maxScale < 1) maxScale = 1;
}

//---------------------------------------------------------------------------
/** \fn long ProgressReporter::getReportInc() const
  Returns the report increment used by the Reader or Writer.

  This value was supplied to the
  \ref ProgressReporter(long maxProgress, long reportIncrement, long maxScale)
  "constructor".
  \return The active <em>report increment</em> value.
*/

//---------------------------------------------------------------------------
/** <b>Do not call:</b> Progress report method called by a Reader or Writer
    to report its progress to this class.
    \param newProgress The progress that the Reader or Writer has made, normally
    the number of bytes \ref Reader::read(char *, int) "read" or
    \ref Writer::write(const char *, int) "written".
    \return \c true if reading or writing is to continue.\n
    \c false if the operation is to be aborted.\n
    \n
    The return value is determined by the value returned
    by your implementation of method \ref progressReport(int progScale)
    "progressReport".
*/

bool ProgressReporter::setProgress(int newProgress)
{
  if (abort) return false;

  curProg = newProgress;

  long newSc = (long)(1.0 * curProg / maxProg * maxSc);

  if (newSc == curSc) return true;

  curSc = newSc;

  abort = !progressReport(curSc);

  return !abort;
}

//---------------------------------------------------------------------------
/** <b>Do not call:</b> Progress report method called by a Reader or Writer
    to report its progress to this class.
    \param progressInc The incremental progress that the Reader or Writer
    has made.
    \return \c true if reading or writing is to continue.\n
    \c false if the operation is to be aborted.\n
    \n
    The return value is determined by the value returned
    by your implementation of method \ref progressReport(int progScale)
    "progressReport".
*/

bool ProgressReporter::incProgress(int progressInc)
{
  if (abort) return false;

  curProg += progressInc;

  long newSc = (long)(1.0 * curProg / maxProg * maxSc);

  if (newSc == curSc) return true;

  curSc = newSc;

  abort = !progressReport(curSc);

  return !abort;
}

//---------------------------------------------------------------------------
/** \fn bool ProgressReporter::progressReport(int progScale)
   Implement this method in your derived class, for instance to
   control a progress bar.

   This method will be called at regular intervals to report the progress
   made.\n
   Return \c true if the read or write operation must continue,\n
   return \c false to abort the operation at the next opportunity.\n
   Method \ref Reader::read(char *, int) "read" or 
   \ref Writer::write(const char *, int) "write" of the Reader or Writer
   will return -1 in the latter case.\n
   \n
   It is guaranteed that this method will only be called when the Reader
   or Writer has made at least \ref getReportInc() const "report increment"
   progress (in bytes).\n
   In addition this method will not be called twice with the same value for
   parameter \c progScale.
   \param progScale The current progress value (<tt>0..maxScale</tt>), see the
   \ref ProgressReporter(long maxProgress, long reportIncrement, long maxScale)
   "constructor".
   \return \c true if the operation must continue,\n
   \c false if the read or write operation must be aborted, perhaps because
   the user pressed the Cancel button.
*/

//---------------------------------------------------------------------------
/** \fn bool ProgressReporter::mustAbort() const
   Returns the abort status.
   \return \c true If the operation will continue,\n
   \c false If the operation must be aborted at the next opportunity.
*/

} // namespace Ino

//---------------------------------------------------------------------------
