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

#include "DxfRead.h"

#include "Exceptions.h"
#include "DxfReader.h"
#include "DxfNameTable.h"
#include "HeaderTable.h"
#include "LineTypeTable.h"
#include "TextStyleTable.h"
#include "LayerTable.h"
#include "BlockTable.h"

namespace Ino
{

//---------------------------------------------------------------------------

DxfAttr::DxfAttr(DxfBlock& blk, const DxfLayer& lay, int colNr,
                 const DxfColor& col, const DxfLineType& lt, double ltScale)
: block(blk), layer(lay), colorNr(colNr), color(col),
  lineType(lt), lineTypeScale(ltScale)
{
}

//---------------------------------------------------------------------------

DxfRead::DxfRead(ASCIIReader& dxfRdr, DxfBuilder& dxfBuilder)
: builder(dxfBuilder), rdr(NULL),
  nameTable(NULL), hdrTable(NULL), ltTable(NULL),
  txtTable(NULL), layerTable(NULL), blockTable(NULL)
{
  rdr = new DxfReader(*this,dxfRdr);

  nameTable  = new DxfNameTable();
  hdrTable   = new HeaderTable(*this,*nameTable);
  ltTable    = new LineTypeTable(*this,*nameTable);
  txtTable   = new TextStyleTable(*this,*nameTable);
  layerTable = new LayerTable(*this,*nameTable,*ltTable);
  blockTable = new BlockTable(*this,*nameTable,*ltTable,*txtTable,*layerTable);

  builder.dxfRd = this;
}

//---------------------------------------------------------------------------

DxfRead::~DxfRead()
{
  if (rdr) delete rdr;

  if (nameTable)  delete nameTable;
  if (hdrTable)   delete hdrTable;
  if (ltTable)    delete ltTable;
  if (txtTable)   delete txtTable;
  if (layerTable) delete layerTable;
  if (blockTable) delete blockTable;
}

//---------------------------------------------------------------------------

DxfRead::Status DxfRead::getStatus() const
{
  return rdr->getStatus();
}

//---------------------------------------------------------------------------

void DxfRead::getCounts(int& lineCount, int& charCount) const
{
  rdr->getCounts(lineCount,charCount);
}

//---------------------------------------------------------------------------

DxfRead::Status DxfRead::readDxf()
{
  DxfBlock *mainBlk = NULL;

  try {
    while (rdr->next()) {
      if (rdr->code != 0) {
        rdr->setStatus(NotDxf);
        throw FileFormatException("Not a DXF file");
      }

      int id = nameTable->getNameId(rdr->value);

      if (id == DxfNameTable::ID_EOF) break;

      if (id != DxfNameTable::ID_SECTION) throw FileFormatException("Expecting Section Start");

      if (!rdr->next() || rdr->code != 2) return PrematureEnd;

      id = nameTable->getNameId(rdr->value);

      switch (id) {
        case DxfNameTable::ID_HEADER: hdrTable->readTable(*rdr);
          break;

        case DxfNameTable::ID_CLASSES: skipSection(*rdr);
          break;

        case DxfNameTable::ID_TABLES: readTables(*rdr);
          break;

        case DxfNameTable::ID_BLOCKS: readBlocks(*rdr);
          break;

        case DxfNameTable::ID_ENTITIES: mainBlk = readEntities(*rdr);
          break;

        case DxfNameTable::ID_OBJECTS: skipSection(*rdr);
          break;

        case DxfNameTable::ID_THUMBNAILIMAGE: skipSection(*rdr);
          break;

        default: // Something unknown
        return FileFormatError;
      }
    }

    if (mainBlk == NULL) {
      rdr->setStatus(PrematureEnd);
      throw FileFormatException("Premature End of File");
    }
    else delete mainBlk;
  }
  catch (FileFormatException) {
    if (rdr->getStatus() == Success) rdr->setStatus(FileFormatError);
  }
  catch (NumberFormatException) {
    if (rdr->getStatus() == Success) rdr->setStatus(NumberFormatError);
  }
  catch (InterruptedException) {
    if (rdr->getStatus() == Success) rdr->setStatus(Aborted);
  }
  catch (...) {
    if (rdr->getStatus() == Success) rdr->setStatus(FileFormatError);
    throw;
  }

  return rdr->getStatus();
}

//---------------------------------------------------------------------------

void DxfRead::readTables(DxfReader& rdr)
{
  while (rdr.next()) {
    if (rdr.code != 0) continue;

    int id = nameTable->getNameId(rdr.value);

    if (id == DxfNameTable::ID_ENDSEC) return;

    if (id != DxfNameTable::ID_TABLE) { // Something wrong
      throw FileFormatException("Expecting Start of Table");
    }

    if (!rdr.next()) return;

    if (rdr.code != 2) continue; // Something wrong

    id = nameTable->getNameId(rdr.value);

    switch (id) {
      case DxfNameTable::ID_LTYPE: ltTable->readLtTable(rdr);
      break;

      case DxfNameTable::ID_LAYER: layerTable->readLayerTable(rdr);
      break;

      case DxfNameTable::ID_STYLE: txtTable->readStyleTable(rdr);
      break;

      case DxfNameTable::ID_APPID:
      case DxfNameTable::ID_BLOCK_RECORD:
      case DxfNameTable::ID_DIMSTYLE:
      case DxfNameTable::ID_UCS:
      case DxfNameTable::ID_VIEW:
      case DxfNameTable::ID_VPORT:
      default:                           skipTable(rdr);
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead::skipTable(DxfReader& rdr)
{
  while (rdr.next()) {
    switch (rdr.code) {
      case 102: skipGroup(rdr);
      break;

      case 0: {
        int id = nameTable->getNameId(rdr.value);

        if (id == DxfNameTable::ID_ENDTAB) return;
      }
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead::readBlocks(DxfReader& rdr)
{
  rdr.next();

  while (!rdr.eof) {
    blockTable->readBlock(rdr);

    while (!rdr.eof && rdr.code != 0) rdr.next();

    if (rdr.eof) return;

    int id = nameTable->getNameId(rdr.value);
    if (id != DxfNameTable::ID_BLOCK) break;
  }

  while (!rdr.eof) {
    if (rdr.code == 0) {
      int id = nameTable->getNameId(rdr.value);
      if (id == DxfNameTable::ID_ENDSEC) return;
    }

    rdr.next();
  }
}

//---------------------------------------------------------------------------

DxfBlock *DxfRead::readEntities(DxfReader& rdr)
{
  DxfBlock *blk = new DxfBlock();

  builder.addBlock(*blk);

  blockTable->readEntities(rdr,*blk);

  if (rdr.eof) return blk;

  while (!rdr.eof) {
    if (rdr.code == 0) {
      int id = nameTable->getNameId(rdr.value);

      if (id == DxfNameTable::ID_ENDSEC) return blk;
    }

    rdr.next();
  }

  return blk;
}

//---------------------------------------------------------------------------

void DxfRead::skipSection(DxfReader& rdr)
{
  while (rdr.next()) {
    switch (rdr.code) {
      case 102:
      case 1002: skipGroup(rdr);
      break;

      case 0: {
        int id = nameTable->getNameId(rdr.value);
        if (id == DxfNameTable::ID_ENDSEC) return;
      }
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead::skipGroup(DxfReader& rdr)
{
  while (rdr.next()) {
    switch (rdr.code) {
      case 102:
      case 1002:
        if (rdr.valueSz == 1 && rdr.value[0] == '}') return;
      break;
    }
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
