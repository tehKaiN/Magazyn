
/* Tiles.h - Game tile/icon types */

#ifndef TILES_H
#define TILES_H

#include <exec/types.h>

/* Main tile types */
enum Tiles {
    FLOOR,
    WALL,
    HERO,
    BOX,    /* Various sub-types (symbol)    */
    PLACE,  /* Various sub-types (symbol)    */
    ITEM,   /* Various sub-types             */
    ARROW,  /* Various sub-types (direction) */
    TYPES
};

/* Box/place symbol */
enum Symbols {
    SYMBOL_1,
    SYMBOL_2,
    SYMBOL_3
};

/* Arrow direction */
enum Directions {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

/* Item types */
enum {
    GOLD,
    FRUIT,
    SKULL
};

/* Graphics icons */
enum Icons {
    ICON_FLOOR,
    ICON_WALL,
    ICON_HERO,
    ICON_BOX_1,
    ICON_BOX_2,
    ICON_BOX_3,
    ICON_PLACE_1,
    ICON_PLACE_2,
    ICON_PLACE_3,
    ICON_GOLD,
    ICON_FRUIT,
    ICON_SKULL,
    ICON_UP,
    ICON_RIGHT,
    ICON_DOWN,
    ICON_LEFT,
    ICONS
};

typedef struct Tile {
    WORD type, subType; /* Type and optional sub-type */
} TILE;

/* Default icons for tile types conversion function */
extern WORD tileToIcon( TILE );

/* Default tile types for icons */
extern TILE iconToTile( WORD );

#endif /* TILES_H */
