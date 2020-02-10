//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- General Virtual Event Dispatcher ----------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Dec 2002 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOEVENTDISPATCHER_INC
#define INOEVENTDISPATCHER_INC

namespace Ino
{

//---------------------------------------------------------------------------

class EventDispatcher;

class EventListener
{
protected:
  virtual void actionDone(EventDispatcher& src, void *param=0) = 0;

public:
  virtual ~EventListener() {}

friend class EventDispatcher;
};

//---------------------------------------------------------------------------

class EventLstEntry;

class EventDispatcher
{
  EventLstEntry *fstEnt;

protected:
  void fireListeners(void *param=0);

public:
  EventDispatcher();
  virtual ~EventDispatcher();

  EventDispatcher(const EventDispatcher& cp);
  EventDispatcher& operator=(const EventDispatcher& src);

  void addListener(EventListener& newListener);
  void removeListener(EventListener& oldListener);
  void removeAllListeners();

  bool hasListeners() { return fstEnt != 0; }
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

