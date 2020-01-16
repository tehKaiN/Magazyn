
/* DBufScreen.h - double buffered screen */

#ifndef DBUFSCREEN_H
#define DBUFSCREEN_H

#include <exec/types.h>
#include <exec/lists.h>
#include <graphics/text.h>

/* Blitter operations */
enum {
    DRAWICON /* Draw single tile */
};

/* Blitter operation */
struct blitOp {
    struct Node node;
    WORD type;
    WORD tile;
    WORD x, y, width, height;
};

/* Screen's UserData */
struct screenUser {
    struct DBufInfo *dbi;
    struct MsgPort  *mp[2]; /* Message ports for double-buffering */
    struct BitMap   *bm[2];
    BOOL safeToWrite, safeToChange;
    UBYTE toggleFrame;
    struct BitMap *gfx; /* Graphics */
    struct List blits, rep; /* Queue */
};

/* Protos */

struct Screen *openScreen(WORD width, WORD height, UBYTE depth, ULONG modeid, struct BitMap *bm[], struct TextAttr *ta, ULONG *colors, struct BitMap *gfx, BOOL editPanel);
void closeScreen(struct Screen *s);
void drawScreen(struct Screen *s);
void changeBitMap(struct Screen *s);

void renderScreen(struct BitMap *gfx, struct BitMap *bm[], BOOL editPanel);

void addBlit(struct List *list, WORD type, WORD tile, WORD x, WORD y, WORD width, WORD height);

#endif /* DBUFSCREEN_H */
