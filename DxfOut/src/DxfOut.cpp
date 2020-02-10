//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Dxf Output Module -------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim International BV Jan 2004 ------------------------
//---------------------------------------------------------------------------
//------- Inofor Hoek Aut BV C. Wolters Jan 2004 ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "DxfOut.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

#include "Vec.h"

#include "El_Line.h"
#include "El_Arc.h"
#include "El_Cir.h"
#include "Trf.h"

#include "Writer.h"

#include <string.h>
#include <math.h>

enum { BufCap = 256 };

namespace Ino
{

//---------------------------------------------------------------------------
// Std Acad color table from: http://www.isctex.net/acadcolors.php
// sorted by red, then green, then blue

struct ColEntry
{
  unsigned char r,g,b,acadCol;
};

static const ColEntry colorTable[] = 
{ {0,0,0,7},       {0,0,79,178},      {0,0,104,176},     {0,0,129,174},
{0,0,189,172},     {0,0,255,5},       {0,0,255,170},     {0,19,79,168},
{0,25,104,166},    {0,31,129,164},    {0,39,79,158},     {0,46,189,162},
{0,52,104,156},    {0,59,79,148},     {0,63,255,160},    {0,64,129,154},
{0,78,104,146},    {0,79,0,98},       {0,79,19,108},     {0,79,39,118},
{0,79,59,128},     {0,79,79,138},     {0,94,189,152},    {0,96,129,144},
{0,104,0,96},      {0,104,25,106},    {0,104,52,116},    {0,104,78,126},
{0,104,104,136},   {0,127,255,150},   {0,129,0,94},      {0,129,31,104},
{0,129,64,114},    {0,129,96,124},    {0,129,129,134},   {0,141,189,142},
{0,189,0,92},      {0,189,46,102},    {0,189,94,112},    {0,189,141,122},
{0,189,189,132},   {0,191,255,140},   {0,255,0,3},       {0,255,0,90},
{0,255,63,100},    {0,255,127,110},   {0,255,191,120},   {0,255,255,4},
{0,255,255,130},   {19,0,79,188},     {19,79,0,88},      {25,0,104,186},
{25,104,0,86},     {31,0,129,184},    {31,129,0,84},     {39,0,79,198},
{39,79,0,78},      {46,0,189,182},    {46,189,0,82},     {51,51,51,250},
{52,0,104,196},    {52,104,0,76},     {53,53,79,179},    {53,59,79,169},
{53,66,79,159},    {53,73,79,149},    {53,79,53,99},     {53,79,59,109},
{53,79,66,119},    {53,79,73,129},    {53,79,79,139},    {59,0,79,208},
{59,53,79,189},    {59,79,0,68},      {59,79,53,89},     {63,0,255,180},
{63,255,0,80},     {64,0,129,194},    {64,129,0,74},     {65,65,65,8},
{66,53,79,199},    {66,79,53,79},     {69,69,104,177},   {69,78,104,167},
{69,86,104,157},   {69,95,104,147},   {69,104,69,97},    {69,104,78,107},
{69,104,86,117},   {69,104,95,127},   {69,104,104,137},  {73,53,79,209},
{73,79,53,69},     {78,0,104,206},    {78,69,104,187},   {78,104,0,66},
{78,104,69,87},    {79,0,0,18},       {79,0,19,248},     {79,0,39,238},
{79,0,59,228},     {79,0,79,218},     {79,19,0,28},      {79,39,0,38},
{79,53,53,19},     {79,53,59,249},    {79,53,66,239},    {79,53,73,229},
{79,53,79,219},    {79,59,0,48},      {79,59,53,29},     {79,66,53,39},
{79,73,53,49},     {79,79,0,58},      {79,79,53,59},     {80,80,80,251},
{86,69,104,197},   {86,86,129,175},   {86,96,129,165},   {86,104,69,77},
{86,107,129,155},  {86,118,129,145},  {86,129,86,95},    {86,129,96,105},
{86,129,107,115},  {86,129,118,125},  {86,129,129,135},  {94,0,189,192},
{94,189,0,72},     {95,69,104,207},   {95,104,69,67},    {96,0,129,204},
{96,86,129,185},   {96,129,0,64},     {96,129,86,85},    {104,0,0,16},
{104,0,25,246},    {104,0,52,236},    {104,0,78,226},    {104,0,104,216},
{104,25,0,26},     {104,52,0,36},     {104,69,69,17},    {104,69,78,247},
{104,69,86,237},   {104,69,95,227},   {104,69,104,217},  {104,78,0,46},
{104,78,69,27},    {104,86,69,37},    {104,95,69,47},    {104,104,0,56},
{104,104,69,57},   {105,105,105,252}, {107,86,129,195},  {107,129,86,75},
{118,86,129,205},  {118,129,86,65},   {126,126,189,173}, {126,141,189,163},
{126,157,189,153}, {126,173,189,143}, {126,189,126,93},  {126,189,141,103},
{126,189,157,113}, {126,189,173,123}, {126,189,189,133}, {127,0,255,190},
{127,255,0,70},    {128,128,128,9},   {129,0,0,14},      {129,0,31,244},
{129,0,64,234},    {129,0,96,224},    {129,0,129,214},   {129,31,0,24},
{129,64,0,34},     {129,86,86,15},    {129,86,96,245},   {129,86,107,235},
{129,86,118,225},  {129,86,129,215},  {129,96,0,44},     {129,96,86,25},
{129,107,86,35},   {129,118,86,45},   {129,129,0,54},    {129,129,86,55},
{130,130,130,253}, {141,0,189,202},   {141,126,189,183}, {141,189,0,62},
{141,189,126,83},  {157,126,189,193}, {157,189,126,73},  {170,170,255,171},
{170,191,255,161}, {170,212,255,151}, {170,234,255,141}, {170,255,170,91},
{170,255,191,101}, {170,255,212,111}, {170,255,234,121}, {170,255,255,131},
{173,126,189,203}, {173,189,126,63},  {189,0,0,12},      {189,0,46,242},
{189,0,94,232},    {189,0,141,222},   {189,0,189,212},   {189,46,0,22},
{189,94,0,32},     {189,126,126,13},  {189,126,141,243}, {189,126,157,233},
{189,126,173,223}, {189,126,189,213}, {189,141,0,42},    {189,141,126,23},
{189,157,126,33},  {189,173,126,43},  {189,189,0,52},    {189,189,126,53},
{190,190,190,254}, {191,0,255,200},   {191,170,255,181}, {191,255,0,60},
{191,255,170,81},  {212,170,255,191}, {212,255,170,71},  {234,170,255,201},
{234,255,170,61},  {255,0,0,1},       {255,0,0,10},      {255,0,63,240},
{255,0,127,230},   {255,0,191,220},   {255,0,255,6},     {255,0,255,210},
{255,63,0,20},     {255,127,0,30},    {255,170,170,11},  {255,170,191,241},
{255,170,212,231}, {255,170,234,221}, {255,170,255,211}, {255,191,0,40},
{255,191,170,21},  {255,212,170,31},  {255,234,170,41},  {255,255,0,2},
{255,255,0,50},    {255,255,170,51},  {255,255,255,7} 
};

//---------------------------------------------------------------------------

static int colDiff(unsigned char r, unsigned char g, unsigned char b,
                                                        const ColEntry& col)
{
  int dr = r; dr -= col.r; dr *= dr;
  int dg = g; dg -= col.g; dg *= dg;
  int db = b; db -= col.b; db *= db;

  return dr + dg + db;
}

//---------------------------------------------------------------------------

static DxfOut::Color acadColor(unsigned char r, unsigned char g, unsigned char b)
{
  const int tableSz = sizeof(colorTable)/sizeof(ColEntry);

  int minIdx     = 0;
  double minDiff = 0;

  for (int i=0; i<tableSz; ++i) {
    double diff = colDiff(r,g,b,colorTable[i]);

    if (i < 1 || diff < minDiff) {
      minIdx  = i;
      minDiff = diff;
    }
  }

  return DxfOut::Color(colorTable[minIdx].acadCol);
}

//---------------------------------------------------------------------------

class DxfOut::Layer
{
  char *name;

  Layer(const Layer& cp);             // No Assignment
  Layer& operator=(const Layer& src); // No copying;

public:
  Layer();
  ~Layer();

  bool sysLayer;
  long handle;
  Color col;

  bool bKey;
  long nKey;

  void setName(const char *name);
  const char *getName() const { return name; }
};

//---------------------------------------------------------------------------

DxfOut::Layer::Layer()
: name(NULL), sysLayer(false), handle(0),
  col(ColBlackWhite), bKey(false), nKey(-2)
{
}

//---------------------------------------------------------------------------

DxfOut::Layer::~Layer()
{
  if (name) delete[] name;
}

//---------------------------------------------------------------------------

void DxfOut::Layer::setName(const char *newName)
{
  delete[] name;
  name = NULL;

  if (newName) {
    name = new char[strlen(newName)+1];
    strcpy(name,newName);
  }
}

//---------------------------------------------------------------------------

void DxfOut::writeGroup(int code, const char *val)
{
  char buf[BufCap];

  if (crlf) 
       snprintf(buf,BufCap-1,"%d\r\n%s\r\n",code,val);
  else snprintf(buf,BufCap-1,"%d\n%s\n",code,val);

  buf[BufCap-1]=0;

  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writeGroup(int code, int val)
{
  char buf[BufCap];

  if (crlf)
       snprintf(buf,BufCap-1,"%d\r\n% 6d\r\n",code,val);
  else snprintf(buf,BufCap-1,"%d\n% 6d\n",code,val);

  buf[BufCap-1]=0;

  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writeHexGroup(int code, int val)
{
  char buf[BufCap];

  if (crlf)
       snprintf(buf,BufCap-1,"%d\r\n%X\r\n",code,val);
  else snprintf(buf,BufCap-1,"%d\n%X\n",code,val);

  buf[BufCap-1]=0;

  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writeGroup(int code, double val)
{
  if (abs(val) <= zeroTol) val = 0.0;

  char buf[BufCap];

  snprintf(buf,BufCap-1,dblFormat,code,val);

  buf[BufCap-1]=0;

  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writePoint(double x, double y)
{
  if (abs(x) <= zeroTol) x = 0.0;
  if (abs(y) <= zeroTol) y = 0.0;

  char buf[BufCap];
  
  snprintf(buf,BufCap-1,pnt2Format,x,y);
  buf[BufCap-1]=0;
  
  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writePoint2D(const Vec2& pt)
{
  writePoint(pt.x,pt.y);
}

//---------------------------------------------------------------------------

void DxfOut::writePoint(double x, double y, double z)
{
  if (abs(x) <= zeroTol) x = 0.0;
  if (abs(y) <= zeroTol) y = 0.0;
  if (abs(z) <= zeroTol) z = 0.0;

  char buf[BufCap];

  snprintf(buf,BufCap-1,pnt3Format,x,y,z);
  buf[BufCap-1]=0;

  if (wrtr) wrtr->write(buf,strlen(buf));
}

//---------------------------------------------------------------------------

void DxfOut::writePoint(const Vec3& pt)
{
  writePoint(pt.x,pt.y,pt.z);
}

//---------------------------------------------------------------------------

void DxfOut::writeHeader()
{
  writeGroup(0,"SECTION");
  writeGroup(2,"HEADER");

  writeGroup(9,"$ACADVER");
  if (version2007) writeGroup(1,"AC1021"); // May contain utf8
  else writeGroup(1,"AC1014");

  writeGroup(9,"$ACADMAINTVER");   writeGroup(70,0);
  writeGroup(9,"$INSBASE");        writePoint(0,0,0);
  writeGroup(9,"$EXTMIN");         writePoint(extMin);
  writeGroup(9,"$EXTMAX");         writePoint(extMax);
  writeGroup(9,"$LIMMIN");         writePoint(extMin.x,extMin.y);
  writeGroup(9,"$LIMMAX");         writePoint(extMax.x,extMax.y);
  writeGroup(9,"$ORTHOMODE");      writeGroup(70,0);
  writeGroup(9,"$REGENMODE");      writeGroup(70,1);
  writeGroup(9,"$FILLMODE");       writeGroup(70,1);
  writeGroup(9,"$QTEXTMODE");      writeGroup(70,0);
  writeGroup(9,"$MIRRTEXT");       writeGroup(70,0);
  writeGroup(9,"$LTSCALE");        writeGroup(40,1.0);
  writeGroup(9,"$ATTMODE");        writeGroup(70,1);
  writeGroup(9,"$TEXTSIZE");       writeGroup(40,5.0);
  writeGroup(9,"$TRACEWID");       writeGroup(40,10.0);
  writeGroup(9,"$TEXTSTYLE");      writeGroup(7,"STANDARD");
  writeGroup(9,"$CLAYER");         writeGroup(8,"0");
  writeGroup(9,"$CELTYPE");        writeGroup(6,"BYLAYER");
  writeGroup(9,"$CECOLOR");        writeGroup(62,7);
  writeGroup(9,"$CELTSCALE");      writeGroup(40,1.0);
  writeGroup(9,"$DISPSILH");       writeGroup(70,0);
  writeGroup(9,"$LUNITS");         writeGroup(70,2);
  writeGroup(9,"$LUPREC");         writeGroup(70,(int)decimals);
  writeGroup(9,"$SKETCHINC");      writeGroup(40,1.0);
  writeGroup(9,"$FILLETRAD");      writeGroup(40,0.0);
  writeGroup(9,"$AUNITS");         writeGroup(70,0);
  writeGroup(9,"$AUPREC");         writeGroup(70,(int)decimals);
  writeGroup(9,"$MENU");           writeGroup(1,"ACAD");
  writeGroup(9,"$ELEVATION");      writeGroup(40,0.0);
  writeGroup(9,"$PELEVATION");     writeGroup(40,0.0);
  writeGroup(9,"$THICKNESS");      writeGroup(40,0.0);
  writeGroup(9,"$LIMCHECK");       writeGroup(70,0);
  writeGroup(9,"$CHAMFERA");       writeGroup(40,0.0);
  writeGroup(9,"$CHAMFERB");       writeGroup(40,0.0);
  writeGroup(9,"$CHAMFERC");       writeGroup(40,0.0);
  writeGroup(9,"$CHAMFERD");       writeGroup(40,0.0);
  writeGroup(9,"$SKPOLY");         writeGroup(70,0);

  // To be updated:
  // writeGroup(9,"$TDCREATE");       writeGroup(40,0.0);
  // writeGroup(9,"$TDUPDATE");       writeGroup(40,0.0);

  writeGroup(9,"$HANDLING");       writeGroup(70,1);
  writeGroup(9,"$HANDSEED");       writeHexGroup(5,seedVal);
  writeGroup(9,"$USRTIMER");       writeGroup(70,0);
  writeGroup(9,"$ANGBASE");        writeGroup(50,0.0);
  writeGroup(9,"$ANGDIR");         writeGroup(70,0);
  writeGroup(9,"$PDMODE");         writeGroup(70,0);
  writeGroup(9,"$PDSIZE");         writeGroup(40,0.0);
  writeGroup(9,"$PLINEWID");       writeGroup(40,0.0);
  writeGroup(9,"$UCSNAME");        writeGroup(2,"");
  writeGroup(9,"$UCSORG");         writePoint(0.0,0.0,0.0);
  writeGroup(9,"$UCSXDIR");        writePoint(1.0,0.0,0.0);
  writeGroup(9,"$UCSYDIR");        writePoint(0.0,1.0,0.0);
  writeGroup(9,"$PUCSNAME");       writeGroup(2,"");
  writeGroup(9,"$PUCSORG");        writePoint(0.0,0.0,0.0);
  writeGroup(9,"$PUCSXDIR");       writePoint(1.0,0.0,0.0);
  writeGroup(9,"$PUCSYDIR");       writePoint(0.0,1.0,0.0);
  writeGroup(9,"$USERI1");         writeGroup(70,0);
  writeGroup(9,"$USERI2");         writeGroup(70,0);
  writeGroup(9,"$USERI3");         writeGroup(70,0);
  writeGroup(9,"$USERI4");         writeGroup(70,0);
  writeGroup(9,"$USERI5");         writeGroup(70,0);
  writeGroup(9,"$USERR1");         writeGroup(40,0.0);
  writeGroup(9,"$USERR2");         writeGroup(40,0.0);
  writeGroup(9,"$USERR3");         writeGroup(40,0.0);
  writeGroup(9,"$USERR4");         writeGroup(40,0.0);
  writeGroup(9,"$USERR5");         writeGroup(40,0.0);
  writeGroup(9,"$WORLDVIEW");      writeGroup(70,1);
  writeGroup(9,"$SHADEDGE");       writeGroup(70,3);
  writeGroup(9,"$SHADEDIF");       writeGroup(70,70);
  writeGroup(9,"$TILEMODE");       writeGroup(70,1);
  writeGroup(9,"$PINSBASE");       writePoint(0.0,0.0,0.0);
  writeGroup(9,"$PLIMCHECK");      writeGroup(70,0);
  writeGroup(9,"$PEXTMIN");        writePoint(1.000000E+20,1.000000E+20,1.000000E+20);
  writeGroup(9,"$PEXTMAX");        writePoint(-1.000000E+20,-1.000000E+20,-1.000000E+20);
  writeGroup(9,"$PLIMMIN");        writePoint(0.0,0.0);
  writeGroup(9,"$PLIMMAX");        writePoint(420.0,297.0);
  writeGroup(9,"$UNITMODE");       writeGroup(70,0);
  writeGroup(9,"$VISRETAIN");      writeGroup(70,0);
  writeGroup(9,"$PLINEGEN");       writeGroup(70,0);
  writeGroup(9,"$PSLTSCALE");      writeGroup(70,1);
  writeGroup(9,"$CMLSTYLE");       writeGroup(2,"STANDARD");
  writeGroup(9,"$CMLJUST");        writeGroup(70,0);
  writeGroup(9,"$CMLSCALE");       writeGroup(40,1.0);

  int msrVal = 1;
  if (userUnit == Inch) msrVal = 0;
  writeGroup(9,"$MEASUREMENT");    writeGroup(70,msrVal);

  writeGroup(0,"ENDSEC");
}

//---------------------------------------------------------------------------

void DxfOut::writeVPortTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"VPORT");
  writeGroup(5,"8");  // Must be 8 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,0);  // Max entries

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeViewTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"VIEW");
  writeGroup(5,"6");  // Must be 6 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,0);  // Max entries

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeLTypeTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"LTYPE");
  writeGroup(5,"5"); // Must be 5 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,3);  // Max entries

  writeGroup(0,"LTYPE");
  writeGroup(5,"14"); // Must be 14 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbLinetypeTableRecord");
  writeGroup(2,"BYBLOCK");
  writeGroup(70,0);
  writeGroup(3,"");
  writeGroup(72,65);
  writeGroup(73,0);
  writeGroup(40,0.0);

  writeGroup(0,"LTYPE");
  writeGroup(5,"15"); // Must be 15 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbLinetypeTableRecord");
  writeGroup(2,"BYLAYER");
  writeGroup(70,0);
  writeGroup(3,"");
  writeGroup(72,65);
  writeGroup(73,0);
  writeGroup(40,0.0);

  writeGroup(0,"LTYPE");
  writeGroup(5,"16"); // Must be 16 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbLinetypeTableRecord");
  writeGroup(2,"CONTINUOUS");
  writeGroup(70,0);
  writeGroup(3,"Solid line");
  writeGroup(72,65);
  writeGroup(73,0);
  writeGroup(40,0.0);

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeLayerTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"LAYER");
  writeGroup(5,"2");  // Must be 2 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,laySz);  // Max entries

  writeGroup(0,"LAYER");
  writeGroup(5,"10"); // Must be 10 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbLayerTableRecord");
  writeGroup(2,"0");
  writeGroup(70,0);
  writeGroup(62,layLst[0].col);
  writeGroup(6,"CONTINUOUS");
  if (version2007) writeGroup(390,0);

  for (int i=1; i<laySz; i++) { // Dont write entry zero again
    writeGroup(0,"LAYER");
    writeHexGroup(5,layLst[i].handle); // Must be 10 hex
    writeGroup(100,"AcDbSymbolTableRecord");
    writeGroup(100,"AcDbLayerTableRecord");
    writeGroup(2,layLst[i].getName());
    writeGroup(70,0);
    writeGroup(62,layLst[i].col);
    writeGroup(6,"CONTINUOUS");
    if (version2007) writeGroup(390,0);
  }

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeStyleTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"STYLE");
  writeGroup(5,"3");  // Must be 3 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,1);  // Max entries

  writeGroup(0,"STYLE");
  writeGroup(5,"11"); // Must be 11 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbTextStyleTableRecord");
  writeGroup(2,"STANDARD");
  writeGroup(70,0);
  writeGroup(40,0.0);
  writeGroup(41,1.0);
  writeGroup(50,0.0);
  writeGroup(71,0);
  writeGroup(42,3.5);
  writeGroup(3,"txt");
  writeGroup(4,"");

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeDimStyleTable()
{
  if (!version2007) {
    writeGroup(0,"TABLE");
    writeGroup(2,"DIMSTYLE");
    writeGroup(5,"A");  // Must be A hex
    writeGroup(100,"AcDbSymbolTable");
    writeGroup(70,0);  // Max entries

    writeGroup(0,"ENDTAB");
  }
  else {
    writeGroup(0,"TABLE");
    writeGroup(2,"DIMSTYLE");
    writeGroup(5,"A");  // Must be A hex
    writeGroup(100,"AcDbSymbolTable");
    writeGroup(70,2);  // Max entries
    writeGroup(100,"AcDbDimStyleTable");
    writeGroup(71,1);
    writeGroup(340,"7F");
    writeGroup(0,"DIMSTYLE");
    writeGroup(105,"7F");
    writeGroup(100,"AcDbSymbolTableRecord");
    writeGroup(100,"AcDbDimStyleTableRecord");
    writeGroup(2,"DIN");
    writeGroup(70,0);
    writeGroup(41,2.5);
    writeGroup(42,0.625);
    writeGroup(43,3.75);
    writeGroup(44,1.25);
    writeGroup(74,0);
    writeGroup(77,1);
    writeGroup(78,8);
    writeGroup(140,2.5);
    writeGroup(141,2.5);
    writeGroup(143,0.0394);
    writeGroup(145,1.0);
    writeGroup(147,0.625);
    writeGroup(172,1);
    writeGroup(273,8);
    writeGroup(277,6);
    writeGroup(340,11);
    writeGroup(0,"ENDTAB");
  }
}

//---------------------------------------------------------------------------

void DxfOut::writeUCSTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"UCS");
  writeGroup(5,"7");  // Must be 7 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,0);  // Max entries

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeAppIDTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"APPID");
  writeGroup(5,"9");  // Must be 9 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,1);  // Max entries

  writeGroup(0,"APPID");
  writeGroup(5,"12"); // Must be 12 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbRegAppTableRecord");
  writeGroup(2,"ACAD");
  writeGroup(70,0);

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeBlockRecordTable()
{
  writeGroup(0,"TABLE");
  writeGroup(2,"BLOCK_RECORD");
  writeGroup(5,"1");  // Must be 1 hex
  writeGroup(100,"AcDbSymbolTable");
  writeGroup(70,3);  // Max entries

  writeGroup(0,"BLOCK_RECORD");
  writeGroup(5,"1F");  // Must be 1F hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbBlockTableRecord");
  writeGroup(2,"*MODEL_SPACE");

  writeGroup(0,"BLOCK_RECORD");
  writeGroup(5,"1B");  // Must be 1B hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbBlockTableRecord");
  writeGroup(2,"*PAPER_SPACE");

  writeGroup(0,"BLOCK_RECORD");
  writeGroup(5,"23");  // Must be 23 hex
  writeGroup(100,"AcDbSymbolTableRecord");
  writeGroup(100,"AcDbBlockTableRecord");
  writeGroup(2,"*PAPER_SPACE0");

  writeGroup(0,"ENDTAB");
}

//---------------------------------------------------------------------------

void DxfOut::writeTables()
{
  writeGroup(0,"SECTION");
  writeGroup(2,"TABLES");

  writeBlockRecordTable();
  writeLTypeTable();
  writeLayerTable();
  writeStyleTable();
  writeDimStyleTable();
  writeVPortTable();
  writeViewTable();
  writeUCSTable();
  writeAppIDTable();

  writeGroup(0,"ENDSEC");
}

//---------------------------------------------------------------------------

void DxfOut::writeBlocks()
{
  writeGroup(0,"SECTION");
  writeGroup(2,"BLOCKS");

  // Model Space

  writeGroup(0,"BLOCK");
  writeGroup(5,"20"); // Must be 20 hex for modelspace
  writeGroup(100,"AcDbEntity");
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockBegin");
  writeGroup(2,"*MODEL_SPACE");
  writeGroup(70,0);
  writePoint(0.0,0.0,0.0);
  writeGroup(3,"*MODEL_SPACE");
  writeGroup(1,"");

  writeGroup(0,"ENDBLK");
  writeGroup(5,"21"); // Must be 21 hex for modelspace endblk
  writeGroup(100,"AcDbEntity");
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockEnd");

  // Paper Space

  writeGroup(0,"BLOCK");
  writeGroup(5,"1C"); // Must be 1C hex for paperspace
  writeGroup(100,"AcDbEntity");
  writeGroup(67,1); // Is paperspace
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockBegin");
  writeGroup(2,"*PAPER_SPACE");
  writeGroup(70,0);
  writePoint(0.0,0.0,0.0);
  writeGroup(3,"*PAPER_SPACE");
  writeGroup(1,"");

  writeGroup(0,"ENDBLK");
  writeGroup(5,"1D"); // Must be 1D hex for paperspace endblk
  writeGroup(100,"AcDbEntity");
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockEnd");

  // Paper Space0

  writeGroup(0,"BLOCK");
  writeGroup(5,"24"); // Must be 24 hex for paperspace0
  writeGroup(100,"AcDbEntity");
  writeGroup(67,1); // Is paperspace
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockBegin");
  writeGroup(2,"*PAPER_SPACE0");
  writeGroup(70,0);
  writePoint(0.0,0.0,0.0);
  writeGroup(3,"*PAPER_SPACE0");
  writeGroup(1,"");

  writeGroup(0,"ENDBLK");
  writeGroup(5,"25"); // Must be 25 hex for paperspace0 endblk
  writeGroup(100,"AcDbEntity");
  writeGroup(8,"0");
  writeGroup(100,"AcDbBlockEnd");

  writeGroup(0,"ENDSEC");
}

//---------------------------------------------------------------------------

void DxfOut::writeProlog()
{
  if (prologWritten) return;
  prologWritten = true;

  writeHeader();
  writeTables();
  writeBlocks();

  writeGroup(0,"SECTION");
  writeGroup(2,"ENTITIES");
}

//---------------------------------------------------------------------------

void DxfOut::writeObjects()
{
  writeGroup(0,"SECTION");
  writeGroup(2,"OBJECTS");

  writeGroup(0,"DICTIONARY");
  writeGroup(5,"C");  // Must be 5 hex
  writeGroup(100,"AcDbDictionary");
  writeGroup(3,"ACAD_GROUP");
  writeGroup(350,"D");
  writeGroup(3,"ACAD_MLINESTYLE");
  writeGroup(350,"17");

  writeGroup(0,"DICTIONARY");
  writeGroup(5,"D");  // Must be D hex
  writeGroup(100,"AcDbDictionary");

  writeGroup(0,"DICTIONARY");
  writeGroup(5,"17");  // Must be 17 hex
  writeGroup(100,"AcDbDictionary");
  writeGroup(3,"STANDARD");
  writeGroup(350,"18");

  writeGroup(0,"MLINESTYLE");
  writeGroup(5,"18");  // Must be 18 hex
  writeGroup(100,"AcDbMlineStyle");
  writeGroup(2,"STANDARD");
  writeGroup(70,0);
  writeGroup(3,"");
  writeGroup(62,256);
  writeGroup(51,"90.0");
  writeGroup(52,"90.0");
  writeGroup(71,2);
  writeGroup(49,"0.5");
  writeGroup(62,256);
  writeGroup(6,"BYLAYER");
  writeGroup(49,"-0.5");
  writeGroup(62,256);
  writeGroup(6,"BYLAYER");

  writeGroup(0,"ENDSEC");
}

//---------------------------------------------------------------------------

void DxfOut::writeEpilog()
{
  if (!prologWritten || epilogWritten) return;

  epilogWritten = true;

  writeGroup(0,"ENDSEC"); // Of ENTITIES section

  writeObjects();

  writeGroup(0,"EOF");
}

//---------------------------------------------------------------------------

DxfOut::DxfOut(Writer &writer, bool appendCrLf)
: wrtr(&writer), prologWritten(false), epilogWritten(false),
  reverseExtrusionDir(false), crlf(appendCrLf), version2007(false),
  handleId(0x200), seedVal(0x8000),
  userUnit(Mm), extMin(*new Vec3()), extMax(*new Vec3()),
  layLst(new Layer[20]), laySz(0), layCap(20),
  currentLayer(0), currentColor(ColBlackWhite),forceElemColor(false),
  decimals(6), zeroTol(1e-6)
{
  layLst[0].handle = 0x10;
  layLst[0].col    = ColBlackWhite;
  layLst[0].setName("0");
  layLst[0].sysLayer = true;
  laySz++;

  setDecimals(6);

  if (writer.isClosed()) wrtr = NULL;
}

//---------------------------------------------------------------------------

DxfOut::~DxfOut()
{
  finish();

  delete[] layLst;

  delete &extMin;
  delete &extMax;
}

//---------------------------------------------------------------------------

void DxfOut::finish()
{
  if (!wrtr) return;

  writeEpilog();

  wrtr->flush();
  wrtr = NULL;
}

//---------------------------------------------------------------------------

bool DxfOut::isOpen() const
{
  return wrtr && !wrtr->isClosed();
}

//---------------------------------------------------------------------------

bool DxfOut::setDecimals(int decs)
{
  if (!wrtr || prologWritten) return false;

  if (decs < MinDecimals) decs = MinDecimals;
  if (decs > MaxDecimals) decs = MaxDecimals;

  decimals = decs;

  if (crlf) {
    sprintf(dblFormat,"%%d\r\n%%.%df\r\n",decs);
    sprintf(pnt2Format,"10\r\n%%.%df\r\n20\r\n%%.%df\r\n",decs,decs);
    sprintf(pnt3Format,"10\r\n%%.%df\r\n20\r\n%%.%df\r\n30\r\n%%.%df\r\n",
                                                             decs,decs,decs);
  }
  else {
    sprintf(dblFormat,"%%d\n%%.%df\n",decs);
    sprintf(pnt2Format,"10\n%%.%df\n20\n%%.%df\n",decs,decs);
    sprintf(pnt3Format,"10\n%%.%df\n20\n%%.%df\n30\n%%.%df\n",decs,decs,decs);
  }

  zeroTol = 1.0;
  for (int i=0; i<decs; ++i) zeroTol /= 10.0;

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setReversedExtrusionDir(bool reverse)
{
  if (!wrtr || prologWritten) return false;

  reverseExtrusionDir = reverse;

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setUnits(Units newUnit)
{
  if (!wrtr || prologWritten) return false;

  userUnit = newUnit;

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setExtents(const Vec3& xMin, const Vec3& xMax)
{
  if (!wrtr || prologWritten) return false;

  extMin = xMin;
  extMax = xMax;

  if (extMin.x > extMax.x) {
    double h = extMin.x; extMin.x = extMax.x; extMax.x =h;
  }

  if (extMin.y > extMax.y) {
    double h = extMin.y; extMin.y = extMax.y; extMax.y =h;
  }

  if (extMin.z > extMax.z) {
    double h = extMin.z; extMin.z = extMax.z; extMax.z =h;
  }

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setHandSeed(long newSeed)
{
  if (!wrtr || prologWritten || newSeed < 1) return false;

  seedVal = 0x200 + newSeed;

  return true;
}

//---------------------------------------------------------------------------

long DxfOut::getLayer(const char *name) const
{
  if (!name || strlen(name) < 1) return -1;

  for (int i=0; i<laySz; i++) {
#ifdef _WIN32
    if (!_stricmp(layLst[i].getName(),name)) return i;
#else
    if (!strcasecmp(layLst[i].getName(),name)) return i;
#endif
  }

  return -1;
}

//---------------------------------------------------------------------------

long DxfOut::addLayer(const char *name, Color col, bool bKey, long nKey)
{
  if (!wrtr || prologWritten || !name ||
          strlen(name) < 1 || col < ColRed) return -1;

  if (laySz >= layCap) {
    layCap += 20;
    Layer *newLst = new Layer[layCap];

    if (layLst) {
      for (int i=0; i<laySz; i++) {
        newLst[i].sysLayer = layLst[i].sysLayer;
        newLst[i].handle   = layLst[i].handle;
        newLst[i].col      = layLst[i].col;
        newLst[i].bKey     = layLst[i].bKey;
        newLst[i].nKey     = layLst[i].nKey;
        newLst[i].setName(layLst[i].getName());
      }

      delete[] layLst;

      layLst = newLst;
    }
  }

  for (int i=0; i<laySz; i++) {
#ifdef _WIN32
    if (!_stricmp(layLst[i].getName(),name)) {
#else
    if (!strcasecmp(layLst[i].getName(),name)) {
#endif
      layLst[i].col  = col;
      layLst[i].bKey = bKey;
      layLst[i].nKey = nKey;

      currentLayer = i;

      return i;
    }
  }

  layLst[laySz].handle = handleId++;
  layLst[laySz].col    = col;
  layLst[laySz].bKey   = bKey;
  layLst[laySz].nKey   = nKey;
  layLst[laySz].setName(name);

  currentLayer = laySz;

  return laySz++;
}

//---------------------------------------------------------------------------

long DxfOut::addLayer(const char *name, unsigned char red, unsigned char green,
                      unsigned char blue, bool bKey, long nKey)
{
  Color col = acadColor(red,green,blue);

  return addLayer(name,col,bKey,nKey);
}

//---------------------------------------------------------------------------

bool DxfOut::setCurrentLayer(long layerIdx)
{
  if (!wrtr || layerIdx < 0 || layerIdx >= laySz) return false;

  currentLayer = layerIdx;
  currentColor = ColByLayer;

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setCurrentLayer(bool bKey, long nKey)
{
  if (!wrtr || !layLst) return false;

  for (int i=0; i<laySz; i++) {
    const Layer& lay = layLst[i];

    if (lay.bKey == bKey && lay.nKey == nKey) {
      currentLayer = i;
      currentColor = ColByLayer;
      return  true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------

bool DxfOut::setCurrentColor(Color color)
{
  if (!wrtr) return false;

  currentColor = color;

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::setCurrentColor(unsigned char red, unsigned char green,
                                                        unsigned char blue)
{
  if (!wrtr) return false;

  Color col = acadColor(red,green,blue);
  return setCurrentColor(col);
}

//---------------------------------------------------------------------------

bool DxfOut::addPoint(const Vec3& p1)
{
  if (!wrtr) return false;

  if (!prologWritten) writeProlog();

  writeGroup(0,"POINT");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbPoint");
  writeGroup(62,currentElemColor());

  writePoint(p1);

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::addLine(const Vec3& p1, const Vec3& p2)
{
  if (!wrtr) return false;

  if (p1.distTo3(p2) < Vec3::IdentDist)
    return addPoint(p1);

  if (!prologWritten) writeProlog();

  writeGroup(0,"LINE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbLine");
  writeGroup(62,currentElemColor());

  writePoint(p1);

  writeGroup(11,p2.x);
  writeGroup(21,p2.y);
  writeGroup(31,p2.z);

  return true;
}

//---------------------------------------------------------------------------

DxfOut::Color DxfOut::currentElemColor() const
{
  if (!forceElemColor || currentColor != ColByLayer) return currentColor;

  if (!layLst || currentLayer < 0 ||
                              currentLayer >= laySz) return currentColor;

  return layLst[currentLayer].col;
}

//---------------------------------------------------------------------------
// AutoCad Arbitrary Axis Algorithm:

void DxfOut::aaa(const Vec3& zDir, Trf3& trf)
{
  Vec3 lZDir(zDir); lZDir.unitLen3();

  Vec3 xDir;

  if (abs(lZDir.x) < 1.0/64 && abs(lZDir.y) < 1.0/64)
       xDir = Vec3(0,1,0).outer(zDir);
  else xDir = Vec3(0,0,1).outer(zDir);

  xDir.unitLen3();

  trf = Trf3(Vec3(),lZDir,xDir);
}

//---------------------------------------------------------------------------

bool DxfOut::addArc(const Vec3& c, double radius,
                     double startAng, double endAng, const Trf3& trf)
{
  if (!wrtr) return false;

  if (!prologWritten) writeProlog();

  writeGroup(0,"ARC");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbCircle");
  writeGroup(62,currentElemColor());

  Vec3 zDir(0,0,1); zDir.isDerivative = true;
  zDir.transform3(trf); zDir.unitLen3();

  if (startAng > endAng) {
    if (reverseExtrusionDir) zDir *= -1.0;
    else {
      double h = startAng; startAng = endAng; endAng = h;
    }
  }

  Trf3 objTrf; aaa(zDir,objTrf);
  objTrf *= trf;

  Vec3 centre(c); centre.transform3(objTrf);

  writePoint(centre);
  writeGroup(40,radius*trf.scaleX());

  writeGroup(210,zDir.x);
  writeGroup(220,zDir.y);
  writeGroup(230,zDir.z);

  Vec3 p1(cos(startAng),sin(startAng),0);
  p1.isDerivative = true; p1.transform3(objTrf);
  
  Vec3 p2(cos(endAng),sin(endAng),0);    
  p2.isDerivative = true; p2.transform3(objTrf);

  startAng = p1.angle() * 180.0/Vec2::Pi;
  endAng   = p2.angle() * 180.0/Vec2::Pi;

  if (startAng < 0.0) startAng += 360.0;
  if (endAng   < 0.0) endAng   += 360.0;

  if (endAng < startAng) endAng += 360.0;

  writeGroup(100,"AcDbArc");
  writeGroup(50,startAng);
  writeGroup(51,endAng);

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::addCircle(const Vec3& c, double radius, const Trf3& trf)
{
  if (!wrtr) return false;

  if (!prologWritten) writeProlog();

  writeGroup(0,"CIRCLE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbCircle");
  writeGroup(62,currentElemColor());

  Vec3 zDir(0,0,1);  zDir.isDerivative = true;
  zDir.transform3(trf); zDir.unitLen3();

  Trf3 objTrf; aaa(zDir,objTrf);
  objTrf *= trf;

  Vec3 centre(c); centre.transform3(objTrf);

  writePoint(centre);
  writeGroup(40,radius*trf.scaleX());

  writeGroup(210,zDir.x);
  writeGroup(220,zDir.y);
  writeGroup(230,zDir.z);

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::add2DPoly(const Vec2 *ptLst, int listLen)
{
  if (!wrtr || !ptLst || listLen < 1) return false;

  if (!prologWritten) writeProlog();

  writeGroup(0,"LWPOLYLINE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbPolyline");
  writeGroup(62,currentColor);

  bool closed = false;

  if (listLen > 1 && ptLst[0].distTo2(ptLst[listLen-1]) <= 1e-5) {
    closed = true;
    listLen--;
  }

  writeGroup(90,listLen);

  if (closed) writeGroup(70,1);
  else writeGroup(70,0);

  writeGroup(43,0);
  writeGroup(38,0.0);

  for (int i=0; i<listLen; ++i) writePoint2D(ptLst[i]);

  writeGroup(210,0.0);
  writeGroup(220,0.0);
  writeGroup(230,1.0);

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::write2DPoly(const Trf3& trf, const Elem_C_Cursor& from,
                                      const Elem_C_Cursor& upto, bool closed)
{
  if (!wrtr) return false;

  if (!from) return true;

  if (!prologWritten) writeProlog();

  writeGroup(0,"LWPOLYLINE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDbPolyline");
  writeGroup(62,currentColor);

  long elCnt = 0;

  Elem_C_Cursor elc(from);

  for (;elc && elc != upto;elc++) {
    const Elem& el = elc->El();

    if (el.Type() == Elem_Type_Circle) elCnt += 2;
    else if (el.Type() == Elem_Type_Arc && el.Span_Angle() >= Vec2::Pi) elCnt += 2;
    else elCnt++;
  }

  if (!closed) elCnt++;
  writeGroup(90,(int)elCnt);

  if (closed) writeGroup(70,1);
  else writeGroup(70,0);

  writeGroup(43,0);

  Vec3 zDir(0,0,1);  zDir.isDerivative = true;
  zDir.transform3(trf); zDir.unitLen3();

  Trf3 objTrf; aaa(zDir,objTrf);
  objTrf *= trf;

  elc = from;

  Vec3 endPt(elc->El().P1()); endPt.transform3(objTrf);
  writeGroup(38,endPt.z);

  for (;elc && elc != upto; ++elc) {
    const Elem& el = elc->El();

    endPt = el.P2();

    switch (el.Type()) {
    case Elem_Type_Line:
    default: {
        Vec3 p1 = el.P1(); p1.transform3(objTrf);
        writePoint2D(p1);
      }
      break;

    case Elem_Type_Arc: {
        const Elem_Arc& arc = (Elem_Arc &)el;

        Vec3 p1 = arc.P1(); p1.transform3(objTrf);
        writePoint2D(p1);

        double spanAngle = arc.Span_Angle();

        if (spanAngle >= Vec2::Pi) {
          spanAngle /= 2.0;
          double bulge = tan(spanAngle/4.0);
          writeGroup(42,bulge);

          Vec3 p2;
          arc.At_Par((arc.Begin_Par() + arc.End_Par())/2.0,p2);
          p2.transform3(objTrf);

          writePoint2D(p2);
          writeGroup(42,bulge);
        }
        else writeGroup(42,tan(spanAngle/4.0));
      }
      break;

      case Elem_Type_Circle: {
        const Elem_Circle& cir = (Elem_Circle &)el;

        Vec3 p1 = cir.P1();
        Vec3 p2(p1); p2 -= cir.C(); p2 *= -1.0; p2 += cir.C();

        p1.transform3(objTrf); p2.transform3(objTrf);

        writePoint2D(p1);
        writeGroup(42,1.0);

        writePoint2D(p2);
        writeGroup(42,1.0);
      }
      break;
    }
  }

  if (!closed) {
    endPt.transform3(objTrf);
    writePoint2D(endPt);
  }

  writeGroup(210,zDir.x);
  writeGroup(220,zDir.y);
  writeGroup(230,zDir.z);

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::add2DPoly(const Trf3& trf, const Contour& cont)
{
  if (!wrtr) return false;

  Elem_C_Cursor elc(cont);

  while (elc) {
    bool closed = false;
    const Vec3& startPt(elc->El().P1());

    Elem_C_Cursor elc2(elc);
    
    for (;;) {
      const Vec3& p1 = elc2->El().P2();

      if (p1.distTo2(startPt) <= Vec2::IdentDist) {
        closed = true;
        ++elc2;
        break;
      }

      ++elc2;
      if (!elc2) break;

      const Vec3& p2(elc2->El().P1());

      if (p2.distTo2(p1) > Vec2::IdentDist) break;
    }

    if (!write2DPoly(trf,elc,elc2,closed)) return false;

    elc = elc2;
  }

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::write3DPoly(const Trf3& trf, const Elem_C_Cursor& from,
                                     const Elem_C_Cursor& upto, bool closed)
{
  if (!wrtr) return false;

  if (!from) return true;

  if (!prologWritten) writeProlog();

  writeGroup(0,"POLYLINE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDb2dPolyline");
  writeGroup(62,currentColor);
  writeGroup(66,1);

  int closedVal = closed ? 1 : 0;
  writeGroup(70,closedVal);
  writeGroup(75,0);

  Vec3 zDir(0,0,1);  zDir.isDerivative = true;
  zDir.transform3(trf); zDir.unitLen3();

  Trf3 objTrf; aaa(zDir,objTrf);
  objTrf *= trf;

  Elem_C_Cursor elc = from;

  Vec3 endPt(elc->El().P1()); endPt.transform3(objTrf);

  writeGroup(10,0);
  writeGroup(20,0);
  writeGroup(30,endPt.z); // Elevation

  writeGroup(210,zDir.x);
  writeGroup(220,zDir.y);
  writeGroup(230,zDir.z);

  for (;elc && elc != upto; ++elc) {
    writeGroup(0,"VERTEX");
    writeHexGroup(5,handleId++);
    writeGroup(100,"AcDbEntity");
    writeGroup(8,layLst[currentLayer].getName());
    writeGroup(100,"AcDbVertex");
    writeGroup(100,"AcDb2dVertex");

    const Elem& el = elc->El();

    endPt = el.P2();

    switch (el.Type()) {
    case Elem_Type_Line:
    default: {
        Vec3 p1 = el.P1(); p1.transform3(objTrf);
        writePoint(p1);
        writeGroup(70,32);
      }
      break;

    case Elem_Type_Arc: {
        const Elem_Arc& arc = (Elem_Arc &)el;

        Vec3 p1 = arc.P1(); p1.transform3(objTrf);
        writePoint(p1);

        double spanAngle = arc.Span_Angle();

        if (spanAngle >= Vec2::Pi) {
          spanAngle /= 2.0;
          double bulge = tan(spanAngle/4.0);
          writeGroup(42,bulge);
          writeGroup(70,32);

          writeGroup(0,"VERTEX");
          writeHexGroup(5,handleId++);
          writeGroup(100,"AcDbEntity");
          writeGroup(8,layLst[currentLayer].getName());
          writeGroup(100,"AcDbVertex");
          writeGroup(100,"AcDb2dVertex");

          Vec3 p2;
          arc.At_Par((arc.Begin_Par() + arc.End_Par())/2.0,p2);
          p2.transform3(objTrf);

          writePoint(p2);
          writeGroup(42,bulge);
          writeGroup(70,32);
        }
        else writeGroup(42,tan(spanAngle/4.0));
      }
      break;

      case Elem_Type_Circle: {
        const Elem_Circle& cir = (Elem_Circle &)el;

        Vec3 p1 = cir.P1();
        Vec3 p2(p1); p2 -= cir.C(); p2 *= -1.0; p2 += cir.C();

        p1.transform3(objTrf); p2.transform3(objTrf);

        writePoint(p1);
        writeGroup(42,1.0);
        writeGroup(70,32);

        writeGroup(0,"VERTEX");
        writeHexGroup(5,handleId++);
        writeGroup(100,"AcDbEntity");
        writeGroup(8,layLst[currentLayer].getName());
        writeGroup(100,"AcDbVertex");
        writeGroup(100,"AcDb2dVertex");

        writePoint(p2);
        writeGroup(42,1.0);
        writeGroup(70,32);
      }
      break;
    }
  }

  if (!closed) {
    endPt.transform3(objTrf);

    writeGroup(0,"VERTEX");
    writeHexGroup(5,handleId++);
    writeGroup(100,"AcDbEntity");
    writeGroup(8,layLst[currentLayer].getName());
    writeGroup(100,"AcDbVertex");
    writeGroup(100,"AcDb2dVertex");

    writePoint(endPt);
  }

  writeGroup(0,"SEQEND");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::add3DPoly(const Trf3& trf, const Contour& cont)
{
  if (!wrtr) return false;

  Elem_C_Cursor elc(cont);

  while (elc) {
    bool closed = false;
    const Vec3& startPt(elc->El().P1());

    Elem_C_Cursor elc2(elc);
    
    for (;;) {
      const Vec3& p1 = elc2->El().P2();

      if (p1.distTo2(startPt) <= Vec2::IdentDist) {
        closed = true;
        ++elc2;
        break;
      }

      ++elc2;
      if (!elc2) break;

      const Vec3& p2(elc2->El().P1());

      if (p2.distTo2(p1) > Vec2::IdentDist) break;
    }

    if (!write3DPoly(trf,elc,elc2,closed)) return false;

    elc = elc2;
  }

  return true;
}

//---------------------------------------------------------------------------

bool DxfOut::add3DPoly(const Vec3* ptLst, int listLen)
{
  if (!wrtr || !ptLst || listLen < 1) return false;

  if (!prologWritten) writeProlog();

  bool closed = false;

  if (listLen > 1 &&
      ptLst[0].distTo3(ptLst[listLen-1]) <= 1e-5) closed = true;

  writeGroup(0,"POLYLINE");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());
  writeGroup(100,"AcDb3dPolyline");
  writePoint(0.0,0.0,0.0);
  writeGroup(62,currentColor);
  writeGroup(66,1);

  int closedVal = closed ? 9 : 8;
  writeGroup(70,closedVal);
  writeGroup(75,0);

  if (closed) listLen--;

  for (int i=0; i<listLen; i++) {
    writeGroup(0,"VERTEX");
    writeHexGroup(5,handleId++);
    writeGroup(100,"AcDbEntity");
    writeGroup(8,layLst[currentLayer].getName());
    writeGroup(100,"AcDbVertex");
    writeGroup(100,"AcDb3dPolylineVertex");

    writePoint(ptLst[i]);

    writeGroup(70,32);
  }
  
  writeGroup(0,"SEQEND");
  writeHexGroup(5,handleId++);
  writeGroup(100,"AcDbEntity");
  writeGroup(8,layLst[currentLayer].getName());

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
