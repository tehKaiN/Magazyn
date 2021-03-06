
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/modeid.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <intuition/screens.h>

#include <clib/intuition_protos.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

#define TWIDTH  (4) /* Szeroko�� i wysoko�� kafla (2^n) */
#define THEIGHT (4)

#define BUFLEN  (63) /* Rozmiar bufora na znaki */

UBYTE dir[] = "Data1"; /* Katalog roboczy */

ULONG modeID = LORES_KEY; /* Domy�lny tryb ekranu */

LONG ifferr = 0; /* B��d IFF */

enum
{
    BMHD,
    CMAP,
    PROPS
};

struct IFFHandle *openIFF(STRPTR name)
{
    UBYTE buffer[BUFLEN + 1];
    struct IFFHandle *iff;

    strncpy(buffer, dir, BUFLEN);
    AddPart(buffer, name, BUFLEN);

    if (iff = AllocIFF())
    {
        BPTR f = Open(buffer, MODE_OLDFILE);
        if (iff->iff_Stream = f)
        {
            InitIFFasDOS(iff);
            if ((ifferr = OpenIFF(iff, IFFF_READ)) == 0)
            {
                if ((ifferr = ParseIFF(iff, IFFPARSE_STEP)) == 0)
                {
                    return(iff);
                }
                CloseIFF(iff);
            }
            Close(f);
        }
        else
            printf("Couldn't open '%s'!\n", buffer);
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

struct IFFHandle *openILBM(STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name))
    {
        if ((ifferr = PropChunk(iff, ID_ILBM, ID_BMHD)) == 0)
        {
            if ((ifferr = PropChunk(iff, ID_ILBM, ID_CMAP)) == 0)
            {
                if ((ifferr = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                {
                    if ((ifferr = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                    {
                        return(iff);
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

BOOL findILBMProps(struct IFFHandle *iff, struct StoredProperty *sp[])
{
    if (sp[BMHD] = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        if (sp[CMAP] = FindProp(iff, ID_ILBM, ID_CMAP))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

ULONG *loadColors(struct StoredProperty *sp)
{
    ULONG *colors;
    ULONG size = sp->sp_Size;
    WORD count = size / 3, c;
    UBYTE *cmap = sp->sp_Data;

    if (colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
    {
        colors[0] = count << 16;
        for (c = 0; c < size; c++)
        {
            UBYTE data = *cmap++;
            colors[c + 1] = RGB(data);
        }
        colors[size + 1] = 0L;
        return(colors);
    }
    return(NULL);
}

BYTE *loadBODY(struct IFFHandle *iff, LONG *psize)
{
    struct ContextNode *cn;
    BYTE *buffer;
    LONG size;

    if (cn = CurrentChunk(iff))
    {
        size = cn->cn_Size;
        if (buffer = AllocMem(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, size) == size)
            {
                *psize = size;
                return(buffer);
            }
            FreeMem(buffer, size);
        }
    }
    return(NULL);
}

BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, BYTE **planeptr, WORD bpr, UBYTE cmp)
{
    BYTE *buf = *bufptr;
    LONG size = *sizeptr;
    BYTE *plane = *planeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        CopyMem(buf, plane, bpr);
        size -= bpr;
        buf += bpr;
        plane += bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE c;
            if (size < 1)
            {
                return(FALSE);
            }
            size--;
            if ((c = *buf++) >= 0)
            {
                WORD count = c + 1;
                if (size < count || bpr < count)
                {
                    return(FALSE);
                }
                size -= count;
                bpr -= count;
                while (count-- > 0)
                {
                    *plane++ = *buf++;
                }
            }
            else if (c != -128)
            {
                WORD count = (-c) + 1;
                BYTE data;
                if (size < 1 || bpr < count)
                {
                    return(FALSE);
                }
                size--;
                bpr -= count;
                data = *buf++;
                while (count-- > 0)
                {
                    *plane++ = data;
                }
            }
        }
    }

    *bufptr = buf;
    *sizeptr = size;
    *planeptr = plane;
    return(TRUE);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, WORD width, WORD height, UBYTE depth, UBYTE cmp)
{
    PLANEPTR planes[8];
    WORD i, j;
    WORD bpr = RowBytes(width);

    for (i = 0; i < depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < depth; i++)
        {
            if (!unpackRow(&buffer, &size, &planes[i], bpr, cmp))
            {
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

struct BitMap *loadBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
    struct BitMap *bm;
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    BYTE *buffer;
    LONG size;

    if (bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        if (buffer = loadBODY(iff, &size))
        {
            if (unpackBitMap(buffer, size, bm, width, height, depth, bmhd->bmh_Compression))
            {
                FreeMem(buffer, size);
                return(bm);
            }
            FreeMem(buffer, size);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

BOOL loadILBM(STRPTR name, struct BitMap **gfx, ULONG **pal)
{
    struct IFFHandle *iff;
    struct StoredProperty *sp[PROPS];

    if (iff = openILBM(name))
    {
        if (findILBMProps(iff, sp))
        {
            if (*pal = loadColors(sp[CMAP]))
            {
                if (*gfx = loadBitMap(iff, (struct BitMapHeader *)sp[BMHD]->sp_Data))
                {
                    closeIFF(iff);
                    return(TRUE);
                }
                FreeVec(*pal);
            }
        }
        closeIFF(iff);
    }
    return(FALSE);
}

int main()
{
    struct BitMap *gfx;
    ULONG *pal;

    if (loadILBM("Graphics.iff", &gfx, &pal))
    {
        struct Screen *s;

        if (s = OpenScreenTags(NULL,
            SA_BitMap,  gfx,
            SA_Colors32,    pal,
            SA_ShowTitle,   FALSE,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE))
        {
            Delay(400);
            CloseScreen(s);
        }
        FreeBitMap(gfx);
        FreeVec(pal);
    }
    return(0);
}
