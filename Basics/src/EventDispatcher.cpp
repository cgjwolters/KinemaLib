//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General Virtual Event Dispatcher ----------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Dec 2002 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------

/*! \file EventDispatcher.h
    \brief Contains classes EventListener and EventDispatcher.
    
    Classes EventListener and EventDispatcher can be used to
    communicate signals or events from an event dispatcher to one or more
    event listeners.

    Derive one ore more classes from the pure virtual class EventListener
    to enable them to listen to events.\n
    Derive a class from EventDispatcher to make it an event dispatcher.
    Once an EventListener is
    \link Ino::EventDispatcher::addListener(Ino::EventListener&)
    registered \endlink with an EventDispatcher it will receive
    any events fired by the \link Ino::EventDispatcher dispatcher \endlink.
    \author C. Wolters
    \date Dec 2002
*/

#include "EventDispatcher.h"

namespace Ino
{

//---------------------------------------------------------------------------
/*! \class EventListener
    \brief Derive a class from this pure virtual class and implement
    method actionDone() to make it an EventListener.

    Then \link EventDispatcher::addListener() register \endlink instances
    of this class with one or more
    \link EventDispatcher EventDispatchers \endlink to be able to receive
    events from that EventDispatcher.
*/

//---------------------------------------------------------------------------
/*! \fn void EventListener::actionDone(EventDispatcher&,void*)
    \brief Implement this method in a derived class.

    This method is called from method EventDispatcher::fireListeners() when
    an instance of this class has been \link EventDispatcher::addListener()
    registered \endlink with that EventDispatcher.
    \param src The EventDispatcher that is calling this method.
    \param param The value that is passed as the argument to
    EventDispatcher::fireListeners().
*/

//---------------------------------------------------------------------------

class EventLstEntry
{
  EventLstEntry(const EventLstEntry& cp);
  EventLstEntry& operator=(const EventLstEntry& src);

public:
  EventLstEntry *nxt;
  EventListener& listener;

  EventLstEntry(EventListener& newListener);
};

//---------------------------------------------------------------------------

EventLstEntry::EventLstEntry(EventListener& newListener)
: nxt(0), listener(newListener)
{
}

//---------------------------------------------------------------------------
/*! \class EventDispatcher
    \brief A class that fires events to one or more registered
    \link Ino::EventListener event listeners \endlink.

    Derive a class from this class to make it an \c EventDispatcher.\n
    Then call fireListeners() to fire an event to all registered listeners.

    \attention This class is currently \b not \b synchronized in any way!.
*/

//! Constructor.
/*! Constructs an EventDispatcher with no registered
    \link EventListener listeners \endlink.
*/

EventDispatcher::EventDispatcher()
: fstEnt(0)
{
}

//---------------------------------------------------------------------------
//! Destructor.
/*! First de-registers all \link EventListener event listeners \endlink and
    then destroys itself.
*/

EventDispatcher::~EventDispatcher()
{
  removeAllListeners();
}

//---------------------------------------------------------------------------
//! Copy constructor.
/*! After construction registers with itself all \link EventListener
    event listeners \endlink that are also registerd with 
    dispatcher \a cp.
    \param cp The EventDispatcher to copy construct from.
*/

EventDispatcher::EventDispatcher(const EventDispatcher& cp)
: fstEnt(0)
{
  EventLstEntry *srcEnt = cp.fstEnt;

  while (srcEnt) {
    addListener(srcEnt->listener);
    srcEnt = srcEnt->nxt;
  }
}

//---------------------------------------------------------------------------
//! Assignment operator.
/*! First deregisters all its \link EventListener listeners \endlink and
    then registers all listeners from \c src to itself.
    \param src The %EventDispatcher to use as a source.
    \return A reference to itself.
*/

EventDispatcher& EventDispatcher::operator=(const EventDispatcher& src)
{
  removeAllListeners();

  EventLstEntry *srcEnt = src.fstEnt;

  while (srcEnt) {
    addListener(srcEnt->listener);
    srcEnt = srcEnt->nxt;
  }

  return *this;
}

//---------------------------------------------------------------------------
/** \fn bool EventDispatcher::hasListeners()
   Returns \c true if at least one EventListener has registered itself
   with this dispatcher.
   \return \c true if there is at least one listener,\n
   \c false otherwise.
*/

//---------------------------------------------------------------------------
//! Registers a new EventListener with this class.
/*! If \a newListener is already registered with this class then nothing
    happens.
    \param newListener The EventListener to register.
*/

void EventDispatcher::addListener(EventListener& newListener)
{
  EventLstEntry *ent = fstEnt;

  while (ent) {
    if (&ent->listener == &newListener) return; // Already in list
    ent = ent->nxt;
  }

  EventLstEntry *newEnt = new EventLstEntry(newListener);

  if (!fstEnt) fstEnt = newEnt;
  else {
    ent = fstEnt;
    while (ent->nxt) ent = ent->nxt;

    ent->nxt = newEnt;
  }
}

//---------------------------------------------------------------------------
//! De-registers an EventListener with this class.
/*! If \a oldListener is not currently registered with this class nothing
    happens.
    \param oldListener The EventListener to de-register.
*/

void EventDispatcher::removeListener(EventListener& oldListener)
{
  if (!fstEnt) return;

  if (&fstEnt->listener == &oldListener) {
    EventLstEntry *ent = fstEnt;
    fstEnt = ent->nxt;

    delete ent;
  }
  else {
    EventLstEntry *prv = fstEnt, *nxt = prv->nxt;

    while (nxt) {
      if (&nxt->listener == &oldListener) {
        prv->nxt = nxt->nxt;
        delete nxt;
        return;
      }

      prv = nxt;
      nxt = prv->nxt;
    }
  }
}

//---------------------------------------------------------------------------
/*! \brief De-registers all \link Ino::EventListener EventListeners \endlink
     with this class.
*/

void EventDispatcher::removeAllListeners()
{
  while (fstEnt) {
    EventLstEntry *ent = fstEnt;
    fstEnt = ent->nxt;
    delete ent;
  }
}

//---------------------------------------------------------------------------
/*! \brief Calls EventListeners::actionDone() on all registered 
    \link Ino::EventListener listeners \endlink.

    There is no guarantee that the listeners will be called in any
    particular order.
    \param param This value is passed on as the second parameters to
    method EventListener::actionDone().
*/

void EventDispatcher::fireListeners(void *param)
{
  EventLstEntry *ent = fstEnt;

  while (ent) {
    ent->listener.actionDone(*this,param);
    ent = ent->nxt;
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
