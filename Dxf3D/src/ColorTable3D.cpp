//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf 3D Format Reader --------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2008 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Sept 2008----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "ColorTable3D.h"

#include "DxfRead3D.h"

namespace Ino
{

//---------------------------------------------------------------------------
// Std Acad color table from: http://www.isctex.net/acadcolors.php

DxfColor3D DxfColor3D::red(255,0,0);
DxfColor3D DxfColor3D::yellow(255,255,0);
DxfColor3D DxfColor3D::green(0,255,0);
DxfColor3D DxfColor3D::cyan(0,255,255);
DxfColor3D DxfColor3D::blue(0,0,255);
DxfColor3D DxfColor3D::magenta(255,0,255);
DxfColor3D DxfColor3D::white(255,255,255);
DxfColor3D DxfColor3D::gray(65,65,65);
DxfColor3D DxfColor3D::lightGray(128,128,128);

//---------------------------------------------------------------------------

DxfColor3D **DxfColorTable3D::initTable()
{
  DxfColor3D **tab = new DxfColor3D *[256];

  tab[0] = &DxfColor3D::white;
  tab[1] = &DxfColor3D::red;
  tab[2] = &DxfColor3D::yellow;
  tab[3] = &DxfColor3D::green;
  tab[4] = &DxfColor3D::cyan;
  tab[5] = &DxfColor3D::blue;
  tab[6] = &DxfColor3D::magenta;
  tab[7] = &DxfColor3D::white;
  tab[8] = &DxfColor3D::gray;
  tab[9] = &DxfColor3D::lightGray;

  int idx = 10;

  tab[idx++] = new DxfColor3D(255,0,0);
  tab[idx++] = new DxfColor3D(255,170,170);
  tab[idx++] = new DxfColor3D(189,0,0);
  tab[idx++] = new DxfColor3D(189,126,126);
  tab[idx++] = new DxfColor3D(129,0,0);
  tab[idx++] = new DxfColor3D(129,86,86);
  tab[idx++] = new DxfColor3D(104,0,0);
  tab[idx++] = new DxfColor3D(104,69,69);
  tab[idx++] = new DxfColor3D(79,0,0);
  tab[idx++] = new DxfColor3D(79,53,53);
  tab[idx++] = new DxfColor3D(255,63,0);
  tab[idx++] = new DxfColor3D(255,191,170);
  tab[idx++] = new DxfColor3D(189,46,0);
  tab[idx++] = new DxfColor3D(189,141,126);
  tab[idx++] = new DxfColor3D(129,31,0);
  tab[idx++] = new DxfColor3D(129,96,86);
  tab[idx++] = new DxfColor3D(104,25,0);
  tab[idx++] = new DxfColor3D(104,78,69);
  tab[idx++] = new DxfColor3D(79,19,0);
  tab[idx++] = new DxfColor3D(79,59,53);
  tab[idx++] = new DxfColor3D(255,127,0);
  tab[idx++] = new DxfColor3D(255,212,170);
  tab[idx++] = new DxfColor3D(189,94,0);
  tab[idx++] = new DxfColor3D(189,157,126);
  tab[idx++] = new DxfColor3D(129,64,0);
  tab[idx++] = new DxfColor3D(129,107,86);
  tab[idx++] = new DxfColor3D(104,52,0);
  tab[idx++] = new DxfColor3D(104,86,69);
  tab[idx++] = new DxfColor3D(79,39,0);
  tab[idx++] = new DxfColor3D(79,66,53);
  tab[idx++] = new DxfColor3D(255,191,0);
  tab[idx++] = new DxfColor3D(255,234,170);
  tab[idx++] = new DxfColor3D(189,141,0);
  tab[idx++] = new DxfColor3D(189,173,126);
  tab[idx++] = new DxfColor3D(129,96,0);
  tab[idx++] = new DxfColor3D(129,118,86);
  tab[idx++] = new DxfColor3D(104,78,0);
  tab[idx++] = new DxfColor3D(104,95,69);
  tab[idx++] = new DxfColor3D(79,59,0);
  tab[idx++] = new DxfColor3D(79,73,53);
  tab[idx++] = new DxfColor3D(255,255,0);
  tab[idx++] = new DxfColor3D(255,255,170);
  tab[idx++] = new DxfColor3D(189,189,0);
  tab[idx++] = new DxfColor3D(189,189,126);
  tab[idx++] = new DxfColor3D(129,129,0);
  tab[idx++] = new DxfColor3D(129,129,86);
  tab[idx++] = new DxfColor3D(104,104,0);
  tab[idx++] = new DxfColor3D(104,104,69);
  tab[idx++] = new DxfColor3D(79,79,0);
  tab[idx++] = new DxfColor3D(79,79,53);
  tab[idx++] = new DxfColor3D(191,255,0);
  tab[idx++] = new DxfColor3D(234,255,170);
  tab[idx++] = new DxfColor3D(141,189,0);
  tab[idx++] = new DxfColor3D(173,189,126);
  tab[idx++] = new DxfColor3D(96,129,0);
  tab[idx++] = new DxfColor3D(118,129,86);
  tab[idx++] = new DxfColor3D(78,104,0);
  tab[idx++] = new DxfColor3D(95,104,69);
  tab[idx++] = new DxfColor3D(59,79,0);
  tab[idx++] = new DxfColor3D(73,79,53);
  tab[idx++] = new DxfColor3D(127,255,0);
  tab[idx++] = new DxfColor3D(212,255,170);
  tab[idx++] = new DxfColor3D(94,189,0);
  tab[idx++] = new DxfColor3D(157,189,126);
  tab[idx++] = new DxfColor3D(64,129,0);
  tab[idx++] = new DxfColor3D(107,129,86);
  tab[idx++] = new DxfColor3D(52,104,0);
  tab[idx++] = new DxfColor3D(86,104,69);
  tab[idx++] = new DxfColor3D(39,79,0);
  tab[idx++] = new DxfColor3D(66,79,53);
  tab[idx++] = new DxfColor3D(63,255,0);
  tab[idx++] = new DxfColor3D(191,255,170);
  tab[idx++] = new DxfColor3D(46,189,0);
  tab[idx++] = new DxfColor3D(141,189,126);
  tab[idx++] = new DxfColor3D(31,129,0);
  tab[idx++] = new DxfColor3D(96,129,86);
  tab[idx++] = new DxfColor3D(25,104,0);
  tab[idx++] = new DxfColor3D(78,104,69);
  tab[idx++] = new DxfColor3D(19,79,0);
  tab[idx++] = new DxfColor3D(59,79,53);
  tab[idx++] = new DxfColor3D(0,255,0);
  tab[idx++] = new DxfColor3D(170,255,170);
  tab[idx++] = new DxfColor3D(0,189,0);
  tab[idx++] = new DxfColor3D(126,189,126);
  tab[idx++] = new DxfColor3D(0,129,0);
  tab[idx++] = new DxfColor3D(86,129,86);
  tab[idx++] = new DxfColor3D(0,104,0);
  tab[idx++] = new DxfColor3D(69,104,69);
  tab[idx++] = new DxfColor3D(0,79,0);
  tab[idx++] = new DxfColor3D(53,79,53);
  tab[idx++] = new DxfColor3D(0,255,63);
  tab[idx++] = new DxfColor3D(170,255,191);
  tab[idx++] = new DxfColor3D(0,189,46);
  tab[idx++] = new DxfColor3D(126,189,141);
  tab[idx++] = new DxfColor3D(0,129,31);
  tab[idx++] = new DxfColor3D(86,129,96);
  tab[idx++] = new DxfColor3D(0,104,25);
  tab[idx++] = new DxfColor3D(69,104,78);
  tab[idx++] = new DxfColor3D(0,79,19);
  tab[idx++] = new DxfColor3D(53,79,59);
  tab[idx++] = new DxfColor3D(0,255,127);
  tab[idx++] = new DxfColor3D(170,255,212);
  tab[idx++] = new DxfColor3D(0,189,94);
  tab[idx++] = new DxfColor3D(126,189,157);
  tab[idx++] = new DxfColor3D(0,129,64);
  tab[idx++] = new DxfColor3D(86,129,107);
  tab[idx++] = new DxfColor3D(0,104,52);
  tab[idx++] = new DxfColor3D(69,104,86);
  tab[idx++] = new DxfColor3D(0,79,39);
  tab[idx++] = new DxfColor3D(53,79,66);
  tab[idx++] = new DxfColor3D(0,255,191);
  tab[idx++] = new DxfColor3D(170,255,234);
  tab[idx++] = new DxfColor3D(0,189,141);
  tab[idx++] = new DxfColor3D(126,189,173);
  tab[idx++] = new DxfColor3D(0,129,96);
  tab[idx++] = new DxfColor3D(86,129,118);
  tab[idx++] = new DxfColor3D(0,104,78);
  tab[idx++] = new DxfColor3D(69,104,95);
  tab[idx++] = new DxfColor3D(0,79,59);
  tab[idx++] = new DxfColor3D(53,79,73);
  tab[idx++] = new DxfColor3D(0,255,255);
  tab[idx++] = new DxfColor3D(170,255,255);
  tab[idx++] = new DxfColor3D(0,189,189);
  tab[idx++] = new DxfColor3D(126,189,189);
  tab[idx++] = new DxfColor3D(0,129,129);
  tab[idx++] = new DxfColor3D(86,129,129);
  tab[idx++] = new DxfColor3D(0,104,104);
  tab[idx++] = new DxfColor3D(69,104,104);
  tab[idx++] = new DxfColor3D(0,79,79);
  tab[idx++] = new DxfColor3D(53,79,79);
  tab[idx++] = new DxfColor3D(0,191,255);
  tab[idx++] = new DxfColor3D(170,234,255);
  tab[idx++] = new DxfColor3D(0,141,189);
  tab[idx++] = new DxfColor3D(126,173,189);
  tab[idx++] = new DxfColor3D(0,96,129);
  tab[idx++] = new DxfColor3D(86,118,129);
  tab[idx++] = new DxfColor3D(0,78,104);
  tab[idx++] = new DxfColor3D(69,95,104);
  tab[idx++] = new DxfColor3D(0,59,79);
  tab[idx++] = new DxfColor3D(53,73,79);
  tab[idx++] = new DxfColor3D(0,127,255);
  tab[idx++] = new DxfColor3D(170,212,255);
  tab[idx++] = new DxfColor3D(0,94,189);
  tab[idx++] = new DxfColor3D(126,157,189);
  tab[idx++] = new DxfColor3D(0,64,129);
  tab[idx++] = new DxfColor3D(86,107,129);
  tab[idx++] = new DxfColor3D(0,52,104);
  tab[idx++] = new DxfColor3D(69,86,104);
  tab[idx++] = new DxfColor3D(0,39,79);
  tab[idx++] = new DxfColor3D(53,66,79);
  tab[idx++] = new DxfColor3D(0,63,255);
  tab[idx++] = new DxfColor3D(170,191,255);
  tab[idx++] = new DxfColor3D(0,46,189);
  tab[idx++] = new DxfColor3D(126,141,189);
  tab[idx++] = new DxfColor3D(0,31,129);
  tab[idx++] = new DxfColor3D(86,96,129);
  tab[idx++] = new DxfColor3D(0,25,104);
  tab[idx++] = new DxfColor3D(69,78,104);
  tab[idx++] = new DxfColor3D(0,19,79);
  tab[idx++] = new DxfColor3D(53,59,79);
  tab[idx++] = new DxfColor3D(0,0,255);
  tab[idx++] = new DxfColor3D(170,170,255);
  tab[idx++] = new DxfColor3D(0,0,189);
  tab[idx++] = new DxfColor3D(126,126,189);
  tab[idx++] = new DxfColor3D(0,0,129);
  tab[idx++] = new DxfColor3D(86,86,129);
  tab[idx++] = new DxfColor3D(0,0,104);
  tab[idx++] = new DxfColor3D(69,69,104);
  tab[idx++] = new DxfColor3D(0,0,79);
  tab[idx++] = new DxfColor3D(53,53,79);
  tab[idx++] = new DxfColor3D(63,0,255);
  tab[idx++] = new DxfColor3D(191,170,255);
  tab[idx++] = new DxfColor3D(46,0,189);
  tab[idx++] = new DxfColor3D(141,126,189);
  tab[idx++] = new DxfColor3D(31,0,129);
  tab[idx++] = new DxfColor3D(96,86,129);
  tab[idx++] = new DxfColor3D(25,0,104);
  tab[idx++] = new DxfColor3D(78,69,104);
  tab[idx++] = new DxfColor3D(19,0,79);
  tab[idx++] = new DxfColor3D(59,53,79);
  tab[idx++] = new DxfColor3D(127,0,255);
  tab[idx++] = new DxfColor3D(212,170,255);
  tab[idx++] = new DxfColor3D(94,0,189);
  tab[idx++] = new DxfColor3D(157,126,189);
  tab[idx++] = new DxfColor3D(64,0,129);
  tab[idx++] = new DxfColor3D(107,86,129);
  tab[idx++] = new DxfColor3D(52,0,104);
  tab[idx++] = new DxfColor3D(86,69,104);
  tab[idx++] = new DxfColor3D(39,0,79);
  tab[idx++] = new DxfColor3D(66,53,79);
  tab[idx++] = new DxfColor3D(191,0,255);
  tab[idx++] = new DxfColor3D(234,170,255);
  tab[idx++] = new DxfColor3D(141,0,189);
  tab[idx++] = new DxfColor3D(173,126,189);
  tab[idx++] = new DxfColor3D(96,0,129);
  tab[idx++] = new DxfColor3D(118,86,129);
  tab[idx++] = new DxfColor3D(78,0,104);
  tab[idx++] = new DxfColor3D(95,69,104);
  tab[idx++] = new DxfColor3D(59,0,79);
  tab[idx++] = new DxfColor3D(73,53,79);
  tab[idx++] = new DxfColor3D(255,0,255);
  tab[idx++] = new DxfColor3D(255,170,255);
  tab[idx++] = new DxfColor3D(189,0,189);
  tab[idx++] = new DxfColor3D(189,126,189);
  tab[idx++] = new DxfColor3D(129,0,129);
  tab[idx++] = new DxfColor3D(129,86,129);
  tab[idx++] = new DxfColor3D(104,0,104);
  tab[idx++] = new DxfColor3D(104,69,104);
  tab[idx++] = new DxfColor3D(79,0,79);
  tab[idx++] = new DxfColor3D(79,53,79);
  tab[idx++] = new DxfColor3D(255,0,191);
  tab[idx++] = new DxfColor3D(255,170,234);
  tab[idx++] = new DxfColor3D(189,0,141);
  tab[idx++] = new DxfColor3D(189,126,173);
  tab[idx++] = new DxfColor3D(129,0,96);
  tab[idx++] = new DxfColor3D(129,86,118);
  tab[idx++] = new DxfColor3D(104,0,78);
  tab[idx++] = new DxfColor3D(104,69,95);
  tab[idx++] = new DxfColor3D(79,0,59);
  tab[idx++] = new DxfColor3D(79,53,73);
  tab[idx++] = new DxfColor3D(255,0,127);
  tab[idx++] = new DxfColor3D(255,170,212);
  tab[idx++] = new DxfColor3D(189,0,94);
  tab[idx++] = new DxfColor3D(189,126,157);
  tab[idx++] = new DxfColor3D(129,0,64);
  tab[idx++] = new DxfColor3D(129,86,107);
  tab[idx++] = new DxfColor3D(104,0,52);
  tab[idx++] = new DxfColor3D(104,69,86);
  tab[idx++] = new DxfColor3D(79,0,39);
  tab[idx++] = new DxfColor3D(79,53,66);
  tab[idx++] = new DxfColor3D(255,0,63);
  tab[idx++] = new DxfColor3D(255,170,191);
  tab[idx++] = new DxfColor3D(189,0,46);
  tab[idx++] = new DxfColor3D(189,126,141);
  tab[idx++] = new DxfColor3D(129,0,31);
  tab[idx++] = new DxfColor3D(129,86,96);
  tab[idx++] = new DxfColor3D(104,0,25);
  tab[idx++] = new DxfColor3D(104,69,78);
  tab[idx++] = new DxfColor3D(79,0,19);
  tab[idx++] = new DxfColor3D(79,53,59);
  tab[idx++] = new DxfColor3D(51,51,51);
  tab[idx++] = new DxfColor3D(80,80,80);
  tab[idx++] = new DxfColor3D(105,105,105);
  tab[idx++] = new DxfColor3D(130,130,130);
  tab[idx++] = new DxfColor3D(190,190,190);
  tab[idx++] = new DxfColor3D(255,255,255);

  for (int i=idx; i<256; ++i) tab[i] = NULL;

  return tab;
}

//---------------------------------------------------------------------------

DxfColorTable3D::DxfColorTable3D()
: colTable(initTable()), cap(256)
{
}

//---------------------------------------------------------------------------

DxfColorTable3D::~DxfColorTable3D()
{
  if (colTable) {
    for (int i=10; i<256; ++i) {
      if (colTable[i]) delete colTable[i];
    }

    delete[] colTable;
  }
}

//---------------------------------------------------------------------------

const DxfColor3D& DxfColorTable3D::getColor(int idx) const
{
  if (idx < 0 || idx >= 256) return DxfColor3D::white; // White

  DxfColor3D *col = colTable[idx];

  if (col) return *col;

  return DxfColor3D::white;
}

} // namespace Ino

//---------------------------------------------------------------------------

