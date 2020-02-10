
/* Board.h - Game board */

#ifndef BOARD_H
#define BOARD_H

#include <exec/types.h>

#include "Tiles.h"

#define WIDTH  20 /* Board dimensions */
#define HEIGHT 16

typedef struct Coords { /* Tile position */
    WORD x, y;
} COORDS;

typedef struct Diff { /* Movement direction */
    WORD dx, dy;
} DIFF;

struct Board {
    TILE    tileBoard[ HEIGHT ][ WIDTH ];
    WORD    goldRequired; /* Number of gold to collect */
    WORD    goldCollected;
    WORD    fruit;
    COORDS  heroCoords; /* Hero position */
    struct List *blits; /* Blit list */
};

/* Main engine routine - hero movement */
extern LONG heroMove( struct Board *board, DIFF diff );

extern void setTile( struct Board *board, COORDS coords, WORD type, WORD subType );

#endif /* BOARD_H */