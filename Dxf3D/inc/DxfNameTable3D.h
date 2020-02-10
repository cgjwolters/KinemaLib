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

#ifndef DXFNAMETABLE3D_INC
#define DXFNAMETABLE3D_INC

#include "IntNameTable3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class DxfNameTable3D : public IntNameTable3D
{
public:
  enum {ID_SECTION = 0, ID_ENDSEC,       ID_EOF,

        ID_HEADER,      ID_CLASSES,      ID_TABLES,
        ID_TABLE,       ID_ENDTAB,       ID_BLOCKS,
        ID_ENTITIES,    ID_OBJECTS,      ID_THUMBNAILIMAGE,

        ID_APPID,       ID_BLOCK_RECORD, ID_DIMSTYLE,
        ID_LAYER,       ID_LTYPE,        ID_STYLE,
        ID_UCS,         ID_VIEW,         ID_VPORT,

        ID_BLOCK,       ID_ENDBLK,       ID_ARC,
        ID_ATTRIB,      ID_CIRCLE,       ID_INSERT,
        ID_LINE,        ID_LWPOLYLINE,   ID_MLINE,
        ID_TEXT,        ID_MTEXT,        ID_RTEXT,
        ID_POLYLINE,    ID_SPLINE,       ID_VERTEX,
        ID_SEQEND,      ID_TRACE,        ID_DIMENSION,
        ID_HATCH,       ID_SOLID,        ID_BYLAYER,
        ID_BYBLOCK,     ID_MODELSPACE,   ID_PAPERSPACE,
        ID_PAPERSPACE0 };

private:
  DxfNameTable3D(const DxfNameTable3D&cp);              // No Copying
  DxfNameTable3D& operator=(const DxfNameTable3D& src); // No Assignment

public:
  DxfNameTable3D();
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
