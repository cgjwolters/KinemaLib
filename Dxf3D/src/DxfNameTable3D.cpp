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

#include "DxfNameTable3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

DxfNameTable3D::DxfNameTable3D()
: IntNameTable3D(50,50)
{
  // ATTN: Keep sequence below in ascending order!

  addName("SECTION",  ID_SECTION);
  addName("ENDSEC",   ID_ENDSEC);
  addName("EOF",      ID_EOF);
  addName("HEADER",   ID_HEADER);
  addName("CLASSES",  ID_CLASSES);
  addName("TABLES",   ID_TABLES);
  addName("TABLE",    ID_TABLE);
  addName("ENDTAB",   ID_ENDTAB);
  addName("BLOCKS",   ID_BLOCKS);
  addName("ENTITIES", ID_ENTITIES);
  addName("OBJECTS",  ID_OBJECTS);
  addName("THUMBNAILIMAGE",ID_THUMBNAILIMAGE);

  addName("APPID",       ID_APPID);
  addName("BLOCK_RECORD",ID_BLOCK_RECORD);
  addName("DIMSTYLE",    ID_DIMSTYLE);
  addName("LAYER",       ID_LAYER);
  addName("LTYPE",       ID_LTYPE);
  addName("STYLE",       ID_STYLE);
  addName("UCS",         ID_UCS);
  addName("VIEW",        ID_VIEW);
  addName("VPORT",       ID_VPORT);

  addName("BLOCK",       ID_BLOCK);
  addName("ENDBLK",      ID_ENDBLK);

  addName("ARC",         ID_ARC);
  addName("ATTRIB",      ID_ATTRIB);
  addName("CIRCLE",      ID_CIRCLE);
  addName("INSERT",      ID_INSERT);
  addName("LINE",        ID_LINE);
  addName("LWPOLYLINE",  ID_LWPOLYLINE);
  addName("MLINE",       ID_MLINE);
  addName("TEXT",        ID_TEXT);
  addName("MTEXT",       ID_MTEXT);
  addName("RTEXT",       ID_RTEXT);
  addName("POLYLINE",    ID_POLYLINE);
  addName("SPLINE",      ID_SPLINE);
  addName("VERTEX",      ID_VERTEX);
  addName("SEQEND",      ID_SEQEND);
  addName("TRACE",       ID_TRACE);
  addName("DIMENSION",   ID_DIMENSION);
  addName("HATCH",       ID_HATCH);
  addName("SOLID",       ID_SOLID);

  addName("BYLAYER",     ID_BYLAYER);
  addName("BYBLOCK",     ID_BYBLOCK);

  addName("Model_Space", ID_MODELSPACE);
  addName("Paper_Space", ID_PAPERSPACE);
  addName("Paper_Space0",ID_PAPERSPACE0);
}

} // namespace Ino

//---------------------------------------------------------------------------

