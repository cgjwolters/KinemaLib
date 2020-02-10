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

#include "DxfRead3D.h"

#include "Exceptions.h"
#include "DxfReader3D.h"
#include "DxfNameTable3D.h"
#include "HeaderTable3D.h"
#include "LineTypeTable3D.h"
#include "TextStyleTable3D.h"
#include "LayerTable3D.h"
#include "BlockTable3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

DxfAttr3D::DxfAttr3D(DxfBlock3D& blk, const DxfLayer3D& lay, int colNr,
                 const DxfColor3D& col, const DxfLineType3D& lt, double ltScale)
: block(blk), layer(lay), colorNr(colNr), colByBlock(colNr==0), color(col),
  lineType(lt), lineTypeScale(ltScale)
{
}

//---------------------------------------------------------------------------

DxfRead3D::DxfRead3D(ASCIIReader3D& dxfRdr, DxfBuilder3D& dxfBuilder)
: builder(dxfBuilder), rdr(NULL),
  nameTable(NULL), hdrTable(NULL), ltTable(NULL),
  txtTable(NULL), layerTable(NULL), blockTable(NULL)
{
  rdr = new DxfReader3D(*this,dxfRdr);

  nameTable  = new DxfNameTable3D();
  hdrTable   = new HeaderTable3D(*this,*nameTable);
  ltTable    = new LineTypeTable3D(*this,*nameTable);
  txtTable   = new TextStyleTable3D(*this,*nameTable);
  layerTable = new LayerTable3D(*this,*nameTable,*ltTable);
  blockTable = new BlockTable3D(*this,*nameTable,*ltTable,*txtTable,*layerTable);

  builder.dxfRd = this;
}

//---------------------------------------------------------------------------

DxfRead3D::~DxfRead3D()
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

DxfRead3D::Status DxfRead3D::getStatus() const
{
  return rdr->getStatus();
}

//---------------------------------------------------------------------------

void DxfRead3D::getCounts(int& lineCount, int& charCount) const
{
  rdr->getCounts(lineCount,charCount);
}

//---------------------------------------------------------------------------

DxfRead3D::Status DxfRead3D::readDxf()
{
  DxfBlock3D *mainBlk = NULL;

  try {
    while (rdr->next()) {
      if (rdr->code != 0) {
        rdr->setStatus(NotDxf);
        throw FileFormatException("Not a DXF file");
      }

      int id = nameTable->getNameId(rdr->value);

      if (id == DxfNameTable3D::ID_EOF) break;

      if (id != DxfNameTable3D::ID_SECTION) throw FileFormatException("Expecting Section Start");

      if (!rdr->next() || rdr->code != 2) return PrematureEnd;

      id = nameTable->getNameId(rdr->value);

      switch (id) {
        case DxfNameTable3D::ID_HEADER: hdrTable->readTable(*rdr);
          break;

        case DxfNameTable3D::ID_CLASSES: skipSection(*rdr);
          break;

        case DxfNameTable3D::ID_TABLES: readTables(*rdr);
          break;

        case DxfNameTable3D::ID_BLOCKS: readBlocks(*rdr);
          break;

        case DxfNameTable3D::ID_ENTITIES: mainBlk = readEntities(*rdr);
          break;

        case DxfNameTable3D::ID_OBJECTS: skipSection(*rdr);
          break;

        case DxfNameTable3D::ID_THUMBNAILIMAGE: skipSection(*rdr);
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

void DxfRead3D::readTables(DxfReader3D& rdr)
{
  while (rdr.next()) {
    if (rdr.code != 0) continue;

    int id = nameTable->getNameId(rdr.value);

    if (id == DxfNameTable3D::ID_ENDSEC) return;

    if (id != DxfNameTable3D::ID_TABLE) { // Something wrong
      throw FileFormatException("Expecting Start of Table");
    }

    if (!rdr.next()) return;

    if (rdr.code != 2) continue; // Something wrong

    id = nameTable->getNameId(rdr.value);

    switch (id) {
      case DxfNameTable3D::ID_LTYPE: ltTable->readLtTable(rdr);
      break;

      case DxfNameTable3D::ID_LAYER: layerTable->readLayerTable(rdr);
      break;

      case DxfNameTable3D::ID_STYLE: txtTable->readStyleTable(rdr);
      break;

      case DxfNameTable3D::ID_APPID:
      case DxfNameTable3D::ID_BLOCK_RECORD:
      case DxfNameTable3D::ID_DIMSTYLE:
      case DxfNameTable3D::ID_UCS:
      case DxfNameTable3D::ID_VIEW:
      case DxfNameTable3D::ID_VPORT:
      default:                           skipTable(rdr);
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead3D::skipTable(DxfReader3D& rdr)
{
  while (rdr.next()) {
    switch (rdr.code) {
      case 102: skipGroup(rdr);
      break;

      case 0: {
        int id = nameTable->getNameId(rdr.value);

        if (id == DxfNameTable3D::ID_ENDTAB) return;
      }
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead3D::readBlocks(DxfReader3D& rdr)
{
  rdr.next();

  while (!rdr.eof) {
    blockTable->readBlock(rdr);

    while (!rdr.eof && rdr.code != 0) rdr.next();

    if (rdr.eof) return;

    int id = nameTable->getNameId(rdr.value);
    if (id != DxfNameTable3D::ID_BLOCK) break;
  }

  while (!rdr.eof) {
    if (rdr.code == 0) {
      int id = nameTable->getNameId(rdr.value);
      if (id == DxfNameTable3D::ID_ENDSEC) return;
    }

    rdr.next();
  }
}

//---------------------------------------------------------------------------

DxfBlock3D *DxfRead3D::readEntities(DxfReader3D& rdr)
{
  DxfBlock3D *blk = new DxfBlock3D();

  builder.addBlock(*blk);

  blockTable->readEntities(rdr,*blk);

  if (rdr.eof) return blk;

  while (!rdr.eof) {
    if (rdr.code == 0) {
      int id = nameTable->getNameId(rdr.value);

      if (id == DxfNameTable3D::ID_ENDSEC) return blk;
    }

    rdr.next();
  }

  return blk;
}

//---------------------------------------------------------------------------

void DxfRead3D::skipSection(DxfReader3D& rdr)
{
  while (rdr.next()) {
    switch (rdr.code) {
      case 102:
      case 1002: skipGroup(rdr);
      break;

      case 0: {
        int id = nameTable->getNameId(rdr.value);
        if (id == DxfNameTable3D::ID_ENDSEC) return;
      }
      break;
    }
  }
}

//---------------------------------------------------------------------------

void DxfRead3D::skipGroup(DxfReader3D& rdr)
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
