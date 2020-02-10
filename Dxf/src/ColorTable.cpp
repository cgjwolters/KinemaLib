//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf Format Reader -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters June 2005----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "ColorTable.h"

#include "DxfRead.h"

namespace Ino
{

//---------------------------------------------------------------------------
// Std Acad color table from: http://www.isctex.net/acadcolors.php

DxfColor DxfColor::red(255,0,0);
DxfColor DxfColor::yellow(255,255,0);
DxfColor DxfColor::green(0,255,0);
DxfColor DxfColor::cyan(0,255,255);
DxfColor DxfColor::blue(0,0,255);
DxfColor DxfColor::magenta(255,0,255);
DxfColor DxfColor::white(255,255,255);
DxfColor DxfColor::gray(65,65,65);
DxfColor DxfColor::lightGray(128,128,128);

//---------------------------------------------------------------------------

DxfColor **DxfColorTable::initTable()
{
  DxfColor **tab = new DxfColor *[256];

  tab[0] = &DxfColor::white;
  tab[1] = &DxfColor::red;
  tab[2] = &DxfColor::yellow;
  tab[3] = &DxfColor::green;
  tab[4] = &DxfColor::cyan;
  tab[5] = &DxfColor::blue;
  tab[6] = &DxfColor::magenta;
  tab[7] = &DxfColor::white;
  tab[8] = &DxfColor::gray;
  tab[9] = &DxfColor::lightGray;

  int idx = 10;

  tab[idx++] = new DxfColor(255,0,0);
  tab[idx++] = new DxfColor(255,170,170);
  tab[idx++] = new DxfColor(189,0,0);
  tab[idx++] = new DxfColor(189,126,126);
  tab[idx++] = new DxfColor(129,0,0);
  tab[idx++] = new DxfColor(129,86,86);
  tab[idx++] = new DxfColor(104,0,0);
  tab[idx++] = new DxfColor(104,69,69);
  tab[idx++] = new DxfColor(79,0,0);
  tab[idx++] = new DxfColor(79,53,53);
  tab[idx++] = new DxfColor(255,63,0);
  tab[idx++] = new DxfColor(255,191,170);
  tab[idx++] = new DxfColor(189,46,0);
  tab[idx++] = new DxfColor(189,141,126);
  tab[idx++] = new DxfColor(129,31,0);
  tab[idx++] = new DxfColor(129,96,86);
  tab[idx++] = new DxfColor(104,25,0);
  tab[idx++] = new DxfColor(104,78,69);
  tab[idx++] = new DxfColor(79,19,0);
  tab[idx++] = new DxfColor(79,59,53);
  tab[idx++] = new DxfColor(255,127,0);
  tab[idx++] = new DxfColor(255,212,170);
  tab[idx++] = new DxfColor(189,94,0);
  tab[idx++] = new DxfColor(189,157,126);
  tab[idx++] = new DxfColor(129,64,0);
  tab[idx++] = new DxfColor(129,107,86);
  tab[idx++] = new DxfColor(104,52,0);
  tab[idx++] = new DxfColor(104,86,69);
  tab[idx++] = new DxfColor(79,39,0);
  tab[idx++] = new DxfColor(79,66,53);
  tab[idx++] = new DxfColor(255,191,0);
  tab[idx++] = new DxfColor(255,234,170);
  tab[idx++] = new DxfColor(189,141,0);
  tab[idx++] = new DxfColor(189,173,126);
  tab[idx++] = new DxfColor(129,96,0);
  tab[idx++] = new DxfColor(129,118,86);
  tab[idx++] = new DxfColor(104,78,0);
  tab[idx++] = new DxfColor(104,95,69);
  tab[idx++] = new DxfColor(79,59,0);
  tab[idx++] = new DxfColor(79,73,53);
  tab[idx++] = new DxfColor(255,255,0);
  tab[idx++] = new DxfColor(255,255,170);
  tab[idx++] = new DxfColor(189,189,0);
  tab[idx++] = new DxfColor(189,189,126);
  tab[idx++] = new DxfColor(129,129,0);
  tab[idx++] = new DxfColor(129,129,86);
  tab[idx++] = new DxfColor(104,104,0);
  tab[idx++] = new DxfColor(104,104,69);
  tab[idx++] = new DxfColor(79,79,0);
  tab[idx++] = new DxfColor(79,79,53);
  tab[idx++] = new DxfColor(191,255,0);
  tab[idx++] = new DxfColor(234,255,170);
  tab[idx++] = new DxfColor(141,189,0);
  tab[idx++] = new DxfColor(173,189,126);
  tab[idx++] = new DxfColor(96,129,0);
  tab[idx++] = new DxfColor(118,129,86);
  tab[idx++] = new DxfColor(78,104,0);
  tab[idx++] = new DxfColor(95,104,69);
  tab[idx++] = new DxfColor(59,79,0);
  tab[idx++] = new DxfColor(73,79,53);
  tab[idx++] = new DxfColor(127,255,0);
  tab[idx++] = new DxfColor(212,255,170);
  tab[idx++] = new DxfColor(94,189,0);
  tab[idx++] = new DxfColor(157,189,126);
  tab[idx++] = new DxfColor(64,129,0);
  tab[idx++] = new DxfColor(107,129,86);
  tab[idx++] = new DxfColor(52,104,0);
  tab[idx++] = new DxfColor(86,104,69);
  tab[idx++] = new DxfColor(39,79,0);
  tab[idx++] = new DxfColor(66,79,53);
  tab[idx++] = new DxfColor(63,255,0);
  tab[idx++] = new DxfColor(191,255,170);
  tab[idx++] = new DxfColor(46,189,0);
  tab[idx++] = new DxfColor(141,189,126);
  tab[idx++] = new DxfColor(31,129,0);
  tab[idx++] = new DxfColor(96,129,86);
  tab[idx++] = new DxfColor(25,104,0);
  tab[idx++] = new DxfColor(78,104,69);
  tab[idx++] = new DxfColor(19,79,0);
  tab[idx++] = new DxfColor(59,79,53);
  tab[idx++] = new DxfColor(0,255,0);
  tab[idx++] = new DxfColor(170,255,170);
  tab[idx++] = new DxfColor(0,189,0);
  tab[idx++] = new DxfColor(126,189,126);
  tab[idx++] = new DxfColor(0,129,0);
  tab[idx++] = new DxfColor(86,129,86);
  tab[idx++] = new DxfColor(0,104,0);
  tab[idx++] = new DxfColor(69,104,69);
  tab[idx++] = new DxfColor(0,79,0);
  tab[idx++] = new DxfColor(53,79,53);
  tab[idx++] = new DxfColor(0,255,63);
  tab[idx++] = new DxfColor(170,255,191);
  tab[idx++] = new DxfColor(0,189,46);
  tab[idx++] = new DxfColor(126,189,141);
  tab[idx++] = new DxfColor(0,129,31);
  tab[idx++] = new DxfColor(86,129,96);
  tab[idx++] = new DxfColor(0,104,25);
  tab[idx++] = new DxfColor(69,104,78);
  tab[idx++] = new DxfColor(0,79,19);
  tab[idx++] = new DxfColor(53,79,59);
  tab[idx++] = new DxfColor(0,255,127);
  tab[idx++] = new DxfColor(170,255,212);
  tab[idx++] = new DxfColor(0,189,94);
  tab[idx++] = new DxfColor(126,189,157);
  tab[idx++] = new DxfColor(0,129,64);
  tab[idx++] = new DxfColor(86,129,107);
  tab[idx++] = new DxfColor(0,104,52);
  tab[idx++] = new DxfColor(69,104,86);
  tab[idx++] = new DxfColor(0,79,39);
  tab[idx++] = new DxfColor(53,79,66);
  tab[idx++] = new DxfColor(0,255,191);
  tab[idx++] = new DxfColor(170,255,234);
  tab[idx++] = new DxfColor(0,189,141);
  tab[idx++] = new DxfColor(126,189,173);
  tab[idx++] = new DxfColor(0,129,96);
  tab[idx++] = new DxfColor(86,129,118);
  tab[idx++] = new DxfColor(0,104,78);
  tab[idx++] = new DxfColor(69,104,95);
  tab[idx++] = new DxfColor(0,79,59);
  tab[idx++] = new DxfColor(53,79,73);
  tab[idx++] = new DxfColor(0,255,255);
  tab[idx++] = new DxfColor(170,255,255);
  tab[idx++] = new DxfColor(0,189,189);
  tab[idx++] = new DxfColor(126,189,189);
  tab[idx++] = new DxfColor(0,129,129);
  tab[idx++] = new DxfColor(86,129,129);
  tab[idx++] = new DxfColor(0,104,104);
  tab[idx++] = new DxfColor(69,104,104);
  tab[idx++] = new DxfColor(0,79,79);
  tab[idx++] = new DxfColor(53,79,79);
  tab[idx++] = new DxfColor(0,191,255);
  tab[idx++] = new DxfColor(170,234,255);
  tab[idx++] = new DxfColor(0,141,189);
  tab[idx++] = new DxfColor(126,173,189);
  tab[idx++] = new DxfColor(0,96,129);
  tab[idx++] = new DxfColor(86,118,129);
  tab[idx++] = new DxfColor(0,78,104);
  tab[idx++] = new DxfColor(69,95,104);
  tab[idx++] = new DxfColor(0,59,79);
  tab[idx++] = new DxfColor(53,73,79);
  tab[idx++] = new DxfColor(0,127,255);
  tab[idx++] = new DxfColor(170,212,255);
  tab[idx++] = new DxfColor(0,94,189);
  tab[idx++] = new DxfColor(126,157,189);
  tab[idx++] = new DxfColor(0,64,129);
  tab[idx++] = new DxfColor(86,107,129);
  tab[idx++] = new DxfColor(0,52,104);
  tab[idx++] = new DxfColor(69,86,104);
  tab[idx++] = new DxfColor(0,39,79);
  tab[idx++] = new DxfColor(53,66,79);
  tab[idx++] = new DxfColor(0,63,255);
  tab[idx++] = new DxfColor(170,191,255);
  tab[idx++] = new DxfColor(0,46,189);
  tab[idx++] = new DxfColor(126,141,189);
  tab[idx++] = new DxfColor(0,31,129);
  tab[idx++] = new DxfColor(86,96,129);
  tab[idx++] = new DxfColor(0,25,104);
  tab[idx++] = new DxfColor(69,78,104);
  tab[idx++] = new DxfColor(0,19,79);
  tab[idx++] = new DxfColor(53,59,79);
  tab[idx++] = new DxfColor(0,0,255);
  tab[idx++] = new DxfColor(170,170,255);
  tab[idx++] = new DxfColor(0,0,189);
  tab[idx++] = new DxfColor(126,126,189);
  tab[idx++] = new DxfColor(0,0,129);
  tab[idx++] = new DxfColor(86,86,129);
  tab[idx++] = new DxfColor(0,0,104);
  tab[idx++] = new DxfColor(69,69,104);
  tab[idx++] = new DxfColor(0,0,79);
  tab[idx++] = new DxfColor(53,53,79);
  tab[idx++] = new DxfColor(63,0,255);
  tab[idx++] = new DxfColor(191,170,255);
  tab[idx++] = new DxfColor(46,0,189);
  tab[idx++] = new DxfColor(141,126,189);
  tab[idx++] = new DxfColor(31,0,129);
  tab[idx++] = new DxfColor(96,86,129);
  tab[idx++] = new DxfColor(25,0,104);
  tab[idx++] = new DxfColor(78,69,104);
  tab[idx++] = new DxfColor(19,0,79);
  tab[idx++] = new DxfColor(59,53,79);
  tab[idx++] = new DxfColor(127,0,255);
  tab[idx++] = new DxfColor(212,170,255);
  tab[idx++] = new DxfColor(94,0,189);
  tab[idx++] = new DxfColor(157,126,189);
  tab[idx++] = new DxfColor(64,0,129);
  tab[idx++] = new DxfColor(107,86,129);
  tab[idx++] = new DxfColor(52,0,104);
  tab[idx++] = new DxfColor(86,69,104);
  tab[idx++] = new DxfColor(39,0,79);
  tab[idx++] = new DxfColor(66,53,79);
  tab[idx++] = new DxfColor(191,0,255);
  tab[idx++] = new DxfColor(234,170,255);
  tab[idx++] = new DxfColor(141,0,189);
  tab[idx++] = new DxfColor(173,126,189);
  tab[idx++] = new DxfColor(96,0,129);
  tab[idx++] = new DxfColor(118,86,129);
  tab[idx++] = new DxfColor(78,0,104);
  tab[idx++] = new DxfColor(95,69,104);
  tab[idx++] = new DxfColor(59,0,79);
  tab[idx++] = new DxfColor(73,53,79);
  tab[idx++] = new DxfColor(255,0,255);
  tab[idx++] = new DxfColor(255,170,255);
  tab[idx++] = new DxfColor(189,0,189);
  tab[idx++] = new DxfColor(189,126,189);
  tab[idx++] = new DxfColor(129,0,129);
  tab[idx++] = new DxfColor(129,86,129);
  tab[idx++] = new DxfColor(104,0,104);
  tab[idx++] = new DxfColor(104,69,104);
  tab[idx++] = new DxfColor(79,0,79);
  tab[idx++] = new DxfColor(79,53,79);
  tab[idx++] = new DxfColor(255,0,191);
  tab[idx++] = new DxfColor(255,170,234);
  tab[idx++] = new DxfColor(189,0,141);
  tab[idx++] = new DxfColor(189,126,173);
  tab[idx++] = new DxfColor(129,0,96);
  tab[idx++] = new DxfColor(129,86,118);
  tab[idx++] = new DxfColor(104,0,78);
  tab[idx++] = new DxfColor(104,69,95);
  tab[idx++] = new DxfColor(79,0,59);
  tab[idx++] = new DxfColor(79,53,73);
  tab[idx++] = new DxfColor(255,0,127);
  tab[idx++] = new DxfColor(255,170,212);
  tab[idx++] = new DxfColor(189,0,94);
  tab[idx++] = new DxfColor(189,126,157);
  tab[idx++] = new DxfColor(129,0,64);
  tab[idx++] = new DxfColor(129,86,107);
  tab[idx++] = new DxfColor(104,0,52);
  tab[idx++] = new DxfColor(104,69,86);
  tab[idx++] = new DxfColor(79,0,39);
  tab[idx++] = new DxfColor(79,53,66);
  tab[idx++] = new DxfColor(255,0,63);
  tab[idx++] = new DxfColor(255,170,191);
  tab[idx++] = new DxfColor(189,0,46);
  tab[idx++] = new DxfColor(189,126,141);
  tab[idx++] = new DxfColor(129,0,31);
  tab[idx++] = new DxfColor(129,86,96);
  tab[idx++] = new DxfColor(104,0,25);
  tab[idx++] = new DxfColor(104,69,78);
  tab[idx++] = new DxfColor(79,0,19);
  tab[idx++] = new DxfColor(79,53,59);
  tab[idx++] = new DxfColor(51,51,51);
  tab[idx++] = new DxfColor(80,80,80);
  tab[idx++] = new DxfColor(105,105,105);
  tab[idx++] = new DxfColor(130,130,130);
  tab[idx++] = new DxfColor(190,190,190);
  tab[idx++] = new DxfColor(255,255,255);

  for (int i=idx; i<256; ++i) tab[i] = NULL;

  return tab;
}

//---------------------------------------------------------------------------

DxfColorTable::DxfColorTable()
: colTable(initTable()), cap(256)
{
}

//---------------------------------------------------------------------------

DxfColorTable::~DxfColorTable()
{
  if (colTable) {
    for (int i=10; i<256; ++i) {
      if (colTable[i]) delete colTable[i];
    }

    delete[] colTable;
  }
}

//---------------------------------------------------------------------------

const DxfColor& DxfColorTable::getColor(int idx) const
{
  if (idx < 0 || idx >= 256) return DxfColor::white; // White

  DxfColor *col = colTable[idx];

  if (col) return *col;

  return DxfColor::white;
}

} // namespace Ino

//---------------------------------------------------------------------------

