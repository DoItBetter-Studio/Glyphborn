#ifndef TILEID_H
#define TILEID_H

#include <stdint.h>

typedef uint16_t TileId;

// [TileType:10][Rotation:3][Variant:3]
#define TILEID_GET_TYPE(id)     (((id) >> 6) & 0x03FF)
#define TILEID_GET_ROTATION(id) (((id) >> 3) & 0x0007)
#define TILEID_GET_VARIANT(id)  ((id) & 0x0007)

#define TILEID_MAKE(type, rot, var) \
    ( ((type & 0x03FF) << 6) | ((rot & 0x7) << 3) | (var & 0x7) )

#endif // TILEID_H