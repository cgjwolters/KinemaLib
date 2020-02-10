//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "PersistentIO.h"

#include "Basics.h"
#include "Exceptions.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup persistence Persistence
 @{
*/

//---------------------------------------------------------------------------
/** \class Persistable
  Base class for all classes to be persisted.

  Every class that is to be persisted must be derived from this class or
  from MainPersistable.

  Then provide an implementation for:

  \li Method definePersistentFields()
  \li The \link Persistable(PersistentReader&) persistence constructor\endlink
  \li Method writePersistentObject()

  \note Due to a current limitation in the persistence library, this class
  must be the \b first base class (in case there are more than one).

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** \class MainPersistable
  Base class for all classes that need to be passed to
  PersistentReader::readMainObject or PersistentWriter::writeMainObject().

  You must provide an implementation for:

  \li Method definePersistentFields()
  \li The \link Persistable(PersistentReader&) persistence constructor\endlink
  \li Method writePersistentObject()
  \li Optionally method readPersistentComplete()

  \note Due to a current limitation in the persistence library, this class
  must be the \b first base class (in case there are more than one).

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** \class PersistentBaseType
  Base class for types PersistentType and PersistentAbstractType.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** \class PersistentType
  Defines the class type of a class derived from Persistable.

  Use operator \c new to create an instance of this class for every class\n
  derived from Persistable
  that is to become part of a persistent stream.

  Then use method PersistentTypeDef::add() to add it to the
  definition of your stream format.

  \see PersistentTypeDef.
*/

//---------------------------------------------------------------------------
/** \class PersistentAbstractType
  Defines the class type of an \b abstract class derived from Persistable.

  Use operator \c new to create an instance of this class for every class\n
  derived from Persistable
  that is to become part of a persistent stream.

  Then use method PersistentTypeDef::add() to add it to the
  definition of your stream format.

  \see PersistentTypeDef.
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/** \fn Persistable::Persistable(PersistentReader& pi)
    Implement this constructor in a derived class to initialize its fields.

    This constructor gets called by PersistentReader for every object
    that is restored from the stream.

    Call the \c read() methods of PersistentReader in the initialisation
    section of the constructor.

    \warning Fields of type pointer are initialized but will normally not
    have been constructed yet at this point (only allocated).\n
    So no attempt must be made to derefence the pointer!\n\n
    Fields of basic types and (as an exception) of type <tt>wchar_t *</tt>
    \b are valid immediately after they have been initialized.

    Example:\n
    <tt>
    MyClass::MyClass(PersistentReader& pi)\n
    : Persistable(),\n
      &nbsp;&nbsp;myIntFld(pi.readInt("IntFld".0),\n
      &nbsp;&nbsp;myStrFld(pi.readString("StrFld",NULL),\n
      &nbsp;&nbsp;myObjFld((MyClass *)pi.readObject("ObjFld",NULL)\n
    {\n
    }\n
    </tt>
*/

//---------------------------------------------------------------------------
/** \fn void Persistable::definePersistentFields(PersistentWriter& po) const
   Implement this method in a derived class to define its fields.

   This method gets called by PersistentWriter just before the first class
   instance of this type is written to the stream.\n
   This method is called only once for each type.

   To define the field use the methods:
   
   \li PersistentWriter::addField(const char *fldName, const type_info& inf)\n\n
   Use this method to define all fields except arrays.
   For basic types and Persistable objects pass the \c typeid(),\n
   for <tt>wchar_t *</tt> strings pass <tt>typed(whcar_t *)</tt>.\n\n
   Examples:\n
   <tt>
   int n;\n
   double d;\n
   wchar_t *s;\n
   class MyClass : public Persistable {} myClass;\n\n
   po.addField("IntFld",typeid(int));\n
   po.addField("DblFld",typeid(double));\n
   po.addField("StrFld",typeid(wchar_t *));\n
   po.addField("ObjFld",typeid(MyClass));</tt> Do \b not pass <tt>MyClass *</tt>\n\n
   \b Note It is \b not possible to define a field for an \c unsigned type.\n
   The reason is that this is not supported in a language such as Java,\n
   and being compatible with all languages is a goal.

   \li PersistentWriter::addArrayField(const char *fldName, T *a)\n\n
   Use this method to define all fields that are arrays of all type except Persistable*.\n\n
   Example:\n
   <tt>
   int *intArr = new int[100];\n
   wchar_t **strArr = new wchar_t*[50];\n\n
   po.addArrayField("IntArr",(int *)NULL);\n
   po.addArrayField("StrArr",(wchar_t **)NULL);\n
   </tt>

   \li PersistentWriter::addObjectArrayField(const char *fldName, T **a)\n\n
   Use this method to define all fields that are arrays of Persistable*.\n\n
   Example:\n
   <tt>
   class MyClass : public Persistable {} myClass;\n\n
   po.addObjectArrayField("ObjArr",(MyClass **)NULL);\n
   </tt>
   \n
   \b Note that is not possible to persist an array of objects, \b only an
   array of object \b pointers.\n
   I.e. <tt>po.addObjectArrayField("ObjArr",(MyClass *)NULL);</tt> will \b not work.\n\n
   One reason is that an array of objects is not supported in languages such as Java,
   .NET and Delphi and being compatible with those is a goal.

    \warning The names of fields must be unique for the entire class,\n
    that is the name of a field in a class must not be the same as
    that of a field in a base class or a derived class!

*/

//---------------------------------------------------------------------------
/** \fn void Persistable::writePersistentObject(PersistentWriter& po) const
   Implement this method in a derived class to write the fields of your class.

   This method gets called by PersistentWriter for each class to be written
   to the stream.

   Use the \c write() methods of PersistentWriter to write the fields that
   were defined in definePersistentFields().

   Example:
   <tt>
   int myInt;\n
   wchar_t *myStr;\n
   MyClass *myObj;\n\n

   po.writeInt("IntFld",myInt);\n
   po.writeString("StrFld",myStr);\n
   po.writeObject("ObjFld",myObj);\n
   </tt>
*/

//---------------------------------------------------------------------------
/** \fn void Persistable::postProcess(PersistentReader& pi)
    This method is \b seriously deprecated, so do \b not use!
*/

//---------------------------------------------------------------------------
/** \fn void Persistable::postProcess(PersistentWriter& po) const
    This method is \b seriously deprecated, so do \b not use!
*/

//---------------------------------------------------------------------------
/** \fn MainPersistable::MainPersistable(PersistentReader& pi)
    Implement this constructor in a derived class to initialize its fields.

    See Persistable::Persistable(PersistentReader& pi) for more details.
*/

//---------------------------------------------------------------------------
/** \fn void MainPersistable::readPersistentComplete(PersistentReader& pi)
    This method is called by PersistentReader just before the return from
    method PersistentReader::readMainObject().

    At this point all classes have been completely restored.

    This method may be used to perform any necessary postprocessing just
    before returning from PersistentReader::readMainObject().
*/

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/** \var const type_info& PersistentBaseType::info
  Holds the \c type_info of the user defined Persistable class.
*/

//---------------------------------------------------------------------------
/** \var const int PersistentBaseType::sz
  Holds the memory size of the user defined Persistable class.
*/

//---------------------------------------------------------------------------
/** \var const char* const PersistentBaseType::name
  Holds the (reversed DNS) name of the user defined Persistable class.
*/

bool PersistentBaseType::legalName(const char *name)
{
  if (!name) return false;
  
  int len = strlen(name);
  if (len < 1) return false;

  for (int i=0; i<len; i++) {
    char c = name[i];

    if (c=='_') continue;
    if (c >= 'A' && c <= 'Z') continue;
    if (c >= 'a' && c <= 'z') continue;

    if (i > 0) {
      if (c == '.') continue;
      if (c >= '0' && c <= '9') continue;
    }

    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Constructor (called by PersistentType and PersistentAbstractType).
  
  \param tpName The name of the class declared.\n
  By convention a reverse DNS name should be used.
  \param inf The \c type_info returned by a \c typeid() of the user class.
  \param tpSz The value returned by \c sizeof() of the user class.\n
  This value is used by PersistentReader when memory for a class is
  allocated.
*/

PersistentBaseType::PersistentBaseType(const char *tpName,
                                     const type_info& inf, int tpSz)
: nameNext(NULL), infoNext(NULL),
  info(inf), sz(tpSz), name(dupStr(tpName))
{
  if (!legalName(name))
        throw IllegalArgumentException("PersistentBaseType::tpName");
}

//---------------------------------------------------------------------------
/** Destructor.
*/

PersistentBaseType::~PersistentBaseType()
{
  if (name) delete[] (char *)name;
}

//---------------------------------------------------------------------------
/** \fn bool PersistentBaseType::canCast(const Persistable *p) const
  <b>Do not call! For internal use only</b>
  
  Determines if the parameter \p p can be cast to the type of
  the template argument T of this class.

  \param p The type to cast to template type T.
  \return \c true if the type can be cast, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** \fn void PersistentBaseType::construct(Persistable* p, PersistentReader& is) const
  <b>Do not call! For internal use only</b>

  Used internally by PersistentReader to construct classes as they are read
  from the stream.
*/

//---------------------------------------------------------------------------
/** \fn PersistentType::PersistentType(const char *tpName)
  Constructor.

  \param tpName The name of the user class.\n
  By convention use a reversed DNS name.\n
  See PersistentTypeDef::add().
*/

//---------------------------------------------------------------------------
/** \fn void PersistentType::construct(Persistable* p, PersistentReader& is) const
  <b>Do not call! For internal use only</b>

  Used internally by PersistentReader to construct classes as they are read
  from the stream.
*/

//---------------------------------------------------------------------------
/** \fn bool PersistentType::canCast(const Persistable *p) const
  <b>Do not call! For internal use only</b>
  
  Determines if the parameter \p p can be cast to the type of
  the template argument T of this class.

  \param p The type to cast to template type T.
  \return \c true if the type can be cast, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** \fn PersistentAbstractType::PersistentAbstractType(const char *tpName)
  Constructor.

  \param tpName The name of the \b abstract user class.\n
  By convention use a reversed DNS name.\n
  See PersistentTypeDef::add().
*/

//---------------------------------------------------------------------------
/** \fn void PersistentAbstractType::construct(Persistable* p, PersistentReader& is) const
  <b>Do not call! For internal use only</b>

  Throws an IllegalStateException when called.
*/

//---------------------------------------------------------------------------
/**
*/

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/** \class PersistentTypeDef
  Derive a class from this class to declare all classes that go into a
  stream.

  Then declare/implement a constructor of the following signature:\n\n

  <tt>MyDataDef(long famMagicNr, long magicNr, char majorNr, char minorNr);</tt>

  In the body of the constructor call the add() method once for every class\n
  (derived from Persistable) that will be part of the persistent stream.

  See the
  \ref PersistentTypeDef(long famMagicNr,long magicNr,char majorNr,char minorNr)
  "constructor" for details.

  To read or write a persistent stream pass an instance of the derived class\n
  to PersistentWriter::writeMainObject() and/or PersistentReader::readMainObject().
*/

//---------------------------------------------------------------------------
/** \var const long PersistentTypeDef::famMagic
  The family magic number passed to the
  \link PersistentTypeDef(long famMagicNr,long magicNr,char majorNr,char minorNr)
  constructor\endlink.
*/

//---------------------------------------------------------------------------
/** \var const long PersistentTypeDef::magic
  The magic number passed to the
  \link PersistentTypeDef(long famMagicNr,long magicNr,char majorNr,char minorNr)
  constructor\endlink.
*/

//---------------------------------------------------------------------------
/** \var const long PersistentTypeDef::mmajor
  The major version number passed to the
  \link PersistentTypeDef(long famMagicNr,long magicNr,char majorNr,char minorNr)
  constructor\endlink.
*/

//---------------------------------------------------------------------------
/** \var const long PersistentTypeDef::mminor
  The minor version number passed to the
  \link PersistentTypeDef(long famMagicNr,long magicNr,char majorNr,char minorNr)
  constructor\endlink.
*/

PersistentBaseType *PersistentTypeDef::makeChain()
{
  if (!nameLst) return NULL;

  PersistentBaseType *first = NULL, *last = NULL;

  for (int i=0; i<cap; ++i) {
    if (!nameLst[i]) continue;

    if (!first) first = last = nameLst[i];
    else last->nameNext = nameLst[i];

    while (last->nameNext) last = last->nameNext;
  }

  return first;
}

//---------------------------------------------------------------------------

void PersistentTypeDef::incCapacity()
{
  PersistentBaseType *first = makeChain();

  if (nameLst) delete[] nameLst;
  if (infoLst) delete[] infoLst;

  cap += IncCap;

  nameLst = new PersistentBaseType *[cap];
  infoLst = new PersistentBaseType *[cap];

  while (first) {
    PersistentBaseType *cur = first;
    first = first->nameNext;

    add(cur);
  }
}

//---------------------------------------------------------------------------

const PersistentBaseType *PersistentTypeDef::find(const type_info& inf) const
{
#ifndef WIN32
  int idx = hashCode(inf.name()) % cap;
#else
  int idx = hashCode(inf.raw_name()) % cap;
#endif

  if (idx < 0) idx += cap;

  PersistentBaseType *cur = infoLst[idx];

  while (cur) {
#ifndef WIN32
    if (!strcmp(inf.name(),cur->info.name())) return cur;
#else
    if (!strcmp(inf.raw_name(),cur->info.raw_name())) return cur;
#endif

    cur = cur->infoNext;
  }

  return NULL;
}

//---------------------------------------------------------------------------
/** Constructor.
  
  \param famMagicNr The family magic number for the stream.
  \param magicNr The magic number for the stream.
  \param majorNr The major version number for the stream definition.
  \param minorNr The minor version number for the stream definition.

  The family magic number and the magic number must uniquely identify the
  contents/format of the persistent stream.

  These magic numbers must be (randomly) picked <b>once and for all</b>
  for every stream format and should be universally unique.\n
  The numbers must \b never be changed, once defined.

  It is advised that the family magic number is the same for a set
  of related stream format definitions (a set of related programs).

  The magic numbers are written to the very front of the stream
  and, when reading back,\n are used to immediately determine that
  the stream is of the correct format.

  \par <b>Declaring the classes.

  In the body of your derived constructor insert a call to method
  add() for every class that is to be part of the stream.\n
  See also PersistentType and PersistentAbstractType.

  Example:

  <tt>
  &nbsp;class MyDataDef: public PersistentTypeDef\n
  &nbsp;{\n
  &nbsp;&nbsp;MyDataDef(long famMagicNr, long magicNr, char majorNr, char minorNr)\n
  &nbsp;&nbsp;: PersistentTypeDef(famMagicNr, magicNr, majorNr, minorNr)\n
  &nbsp;&nbsp;{\n
  &nbsp;&nbsp;&nbsp;add(new PersistentType<MyClass>("com.mycompany.mysoftware.myClass"));\n
  &nbsp;&nbsp;&nbsp;add(new PersistentType<MyClass2>("com.mycompany.mysoftware.myClass2"));\n
  &nbsp;&nbsp;&nbsp;add(new PersistentAbstractType<MyAbstractClass>(
                        "com.mycompany.mysoftware.myAbstractClass"));\n
  &nbsp;&nbsp;}\n
  &nbsp;}\n
  </tt>

  As can be seen abstract classes can (and must) be declared as well using the add()
  method.

  see Method add()
*/

PersistentTypeDef::PersistentTypeDef(long famMagicNr, long magicNr, 
                                                  char majorNr, char minorNr)
: famMagic(famMagicNr), magic(magicNr), mmajor(majorNr), mminor(minorNr),
  nameLst(new PersistentBaseType *[IncCap]),
  infoLst(new PersistentBaseType *[IncCap]),
  sz(0), cap(IncCap), completed(false)
{
  memset(nameLst,0,cap*sizeof(PersistentBaseType *));
  memset(infoLst,0,cap*sizeof(PersistentBaseType *));
}

//---------------------------------------------------------------------------
/** Destructor.
  Cleans up the data of the list of classes.
*/

PersistentTypeDef::~PersistentTypeDef()
{
  PersistentBaseType *first = makeChain();

  while (first) {
    PersistentBaseType *cur = first;
    first = first->nameNext;

    delete cur;
  }

  if (nameLst) delete[] nameLst;
  if (infoLst) delete[] infoLst;
}

//---------------------------------------------------------------------------
/** Adds/declares a class that is to become part of the persistent stream.

  \param tp A pointer to a either a PersistentType or a PersistentAbstractType.\n
  This class will become \b owner of the instance pointed to by \p tp,\n
  so it must have been allocated on the heap (using operator \c new).

  Example:

  <tt>  add(new PersistentType<MyClass>("com.mycompany.mysoftware.myClass"));</tt>
  \n
  or\n
  <tt>  add(new PersistentAbstractType<MyAbstractClass>
                                       ("com.mycompany.mysoftware.myClass"));</tt>

  Use the latter case if the class declared is abstract (pure virtual).

  Class names provided should be universally unique.\n
  By convertion use a reversed DNS name as in the example above.
*/

void PersistentTypeDef::add(PersistentBaseType *tp)
{
  if (!tp) throw NullPointerException("PersistentTypeDef::add");

  if (completed)
           throw IllegalStateException("PersistentTypeDef::add(completed)");

  if (find(tp->info)) throw DuplicateNameException("PersistentTypeDef::add");

  int idx = hashCode(tp->name) % cap;
  if (idx < 0) idx += cap;

  tp->nameNext = nameLst[idx];
  nameLst[idx]  = tp;

#ifndef WIN32
  idx = hashCode(tp->info.name()) % cap;
#else
  idx = hashCode(tp->info.raw_name()) % cap;
#endif

  if (idx < 0) idx += cap;

  tp->infoNext = infoLst[idx];
  infoLst[idx]  = tp;
}

//---------------------------------------------------------------------------
/** Returns the \link PersistentBaseType type definition\endlink of a class
    by name.

    \param name The name of the class.
    \return The PersistentBaseType of the class, or \c NULL if no such class
    exists.
*/

const PersistentBaseType *PersistentTypeDef::get(const char *name) const
{
  completed = true;

  int idx = hashCode(name) % cap;
  if (idx < 0) idx += cap;

  PersistentBaseType *cur = nameLst[idx];

  while (cur) {
    if (!strcmp(name,cur->name)) return cur;

    cur = cur->nameNext;
  }

  return NULL;
}

//---------------------------------------------------------------------------
/** Returns the \link PersistentBaseType type definition\endlink of a class
    by \c type_info.

    \param inf The rtti \c type_info object of the class as returned by typeid().

    \return The PersistentBaseType of the class, or \c NULL if no such class
    exists.
*/

const PersistentBaseType *PersistentTypeDef::get(const type_info& inf) const
{
  completed = true;

  return find(inf);
}

} // namespace Ino

//---------------------------------------------------------------------------

