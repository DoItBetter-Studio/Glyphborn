#ifndef MAP_RUNTIME_H
#define MAP_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

// -----------------------------------------------------------------------------
// Core 3D Tile & Collision Definitions
// -----------------------------------------------------------------------------

typedef uint16_t TileId;

// TileId format is unchanged:
// [TileType:10][Rotation:3][Variant:3]
#define TILEID_GET_TYPE(id)     ((uint16_t)((id) >> 6))
#define TILEID_GET_ROTATION(id) ((uint16_t)((id) >> 3) & 0x7)
#define TILEID_GET_VARIANT(id)  ((uint16_t)((id) & 0x7))

#define MAP_CHUNK_X 32
#define MAP_CHUNK_Y 32
#define MAP_CHUNK_Z 32
#define MAP_VOXEL_COUNT (MAP_CHUNK_X * MAP_CHUNK_Y * MAP_CHUNK_Z)

// Temperary Forward Declarations
typedef struct mrb_state mrb_state;
typedef struct lua_State lua_State;
typedef struct Vertex    Vertex;

// -----------------------------------------------------------------------------
// Geometry: map.bin
// -----------------------------------------------------------------------------

typedef struct MapGeometry
{
    uint8_t size_x;
    uint8_t size_y;
    uint8_t size_z;

    const uint8_t* base;
    const TileId* tiles;
} MapGeometry;

// -----------------------------------------------------------------------------
// Collision: collision.bin
// -----------------------------------------------------------------------------

typedef struct MapCollision
{
    uint8_t size_x;
    uint8_t size_y;
    uint8_t size_z;

    const uint8_t* base;
    const uint8_t* cells;
} MapCollision;

// -----------------------------------------------------------------------------
// MapMetadata: matrix.bin
// -----------------------------------------------------------------------------

typedef struct MapMetadata
{
    char        name[32];
    char        area_type[16];
    char        icon[32];
    char        music_day[32];
    char        music_night[32];
    char        weather[16];
    uint16_t    map_id;
    uint16_t    flags;
} MapMetadata;

// -----------------------------------------------------------------------------
// MapEvents: events.bin
// -----------------------------------------------------------------------------

typedef struct MapEvent
{
    uint16_t x, y, z;
    uint16_t type;
    uint16_t script_id;
} MapEvent;

typedef struct MapEvents
{
    uint16_t count;
    MapEvent* list;
} MapEvents;

// -----------------------------------------------------------------------------
// MapScripts: scripts.inc / bank.mrb
// -----------------------------------------------------------------------------

typedef struct MapScriptEntry
{
    uint32_t id;
    uint32_t offset;
    uint32_t length;
} MapScriptEntry;

typedef struct MapScripts
{
    uint32_t count;
    MapScriptEntry* entries;
    const uint8_t* bank;
} MapScripts;

// -----------------------------------------------------------------------------
// MapText: text.bin
// -----------------------------------------------------------------------------

typedef struct MapTextEntry {
    const char* data;
    uint32_t    length;
} MapTextEntry;

typedef struct MapText {
    uint16_t      count;
    MapTextEntry* entries;
} MapText;

// -----------------------------------------------------------------------------
// RubyScriptVM: internal scripting engine
// -----------------------------------------------------------------------------

typedef struct RubyScriptVM {
    mrb_state* mrb;  // from mruby
    void*      user; // optional internal userdata
} RubyScriptVM;

// -----------------------------------------------------------------------------
// LuaModVM: used for modding with Lua 5.1 + LuaSocket.
// -----------------------------------------------------------------------------

typedef struct LuaModVM
{
    lua_State* L;
} LuaModVM;

// -----------------------------------------------------------------------------
// MapMeshCache: stores any generated mesh
// -----------------------------------------------------------------------------

typedef struct MapMeshCache {
    Vertex*    vertices;
    uint32_t   vertex_count;
    uint16_t*  indices;
    uint32_t   index_count;
    uint8_t    valid;    // 1 if cache is valid
} MapMeshCache;

// -----------------------------------------------------------------------------
// Final Map Runtime Object
// -----------------------------------------------------------------------------

typedef struct MapRuntime
{
    MapGeometry  geo;        // map.bin
    MapCollision col;        // collision.bin

    MapMetadata  meta;       // metadata from matrix.bin
    MapEvents    events;     // events.bin
    MapScripts   scripts;    // scripts.inc + bytecode bank
    MapText      text;       // text.bin

    RubyScriptVM ruby;       // internal scripting
    LuaModVM     lua;        // modding scripting

    MapMeshCache mesh_cache; // optional, for render optimization

} MapRuntime;

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

// Loads 3D TileIDs from map.bin
int map_geometry_init_from_bin(MapGeometry* out, const uint8_t* data, size_t data_size);

// Loads 3D collision mask from collision.bin
int map_collision_init_from_bin(MapCollision* out, const uint8_t* data, size_t data_size);

// Loads both geometry & collision into a single structure
int map_runtime_init(
    MapRuntime* out,
    const uint8_t* map_bin, size_t map_bin_size,
    const uint8_t* col_bin, size_t col_bin_size);

// Load metadata for a specific map_id from matrix.bin
int map_metadata_init_from_matrix(MapMetadata* out,
                                  const uint8_t* data,
                                  size_t data_size,
                                  uint16_t map_id);

// Load events from events.bin
int map_events_init_from_bin(MapEvents* out,
                             const uint8_t* data,
                             size_t data_size);
void map_events_dispose(MapEvents* events);

// Load script table from scripts.inc / scripts.bin + bytecode bank
int map_scripts_init(MapScripts* out,
                     const uint8_t* table_data,
                     size_t table_size,
                     const uint8_t* bank_data,
                     size_t bank_size);
void map_scripts_dispose(MapScripts* scripts);

// Load text entries from text.bin
int map_text_init_from_bin(MapText* out,
                           const uint8_t* data,
                           size_t data_size);
void map_text_dispose(MapText* text);

// Ruby / Lua VM wrappers (to be implemented against mruby / Lua)
int  ruby_init_for_map(RubyScriptVM* vm, const MapScripts* scripts);
void ruby_shutdown_for_map(RubyScriptVM* vm);

int  lua_init_for_map(LuaModVM* vm);
void lua_shutdown_for_map(LuaModVM* vm);

// Full MapRuntime init / dispose
int map_runtime_full_init(
    MapRuntime* out,
    const uint8_t* map_bin,         size_t map_bin_size,
    const uint8_t* col_bin,         size_t col_bin_size,
    const uint8_t* events_bin,      size_t events_size,
    const uint8_t* scripts_table,   size_t scripts_table_size,
    const uint8_t* text_bin,        size_t text_size,
    const uint8_t* matrix_bin,      size_t matrix_size,
    const uint8_t* mrb_bank,        size_t mrb_bank_size,
    uint16_t      map_id
);

void map_runtime_dispose(MapRuntime* map);

// -----------------------------------------------------------------------------
// Accessors
// -----------------------------------------------------------------------------

static inline TileId map_geo_get_tile(const MapGeometry* geo, uint8_t x, uint8_t y, uint8_t z)
{
    if (!geo || !geo->tiles) return 0;
    if (x >= geo->size_x || y >= geo->size_y || z >= geo->size_z) return 0;

    uint32_t idx = (uint32_t)y * (MAP_CHUNK_X * MAP_CHUNK_Z)
                    + (uint32_t)z * MAP_CHUNK_X
                    + (uint32_t)x;
    
    return geo->tiles[idx];
}

static inline uint8_t map_col_get(const MapCollision* col, uint8_t x, uint8_t y, uint8_t z)
{
    if (!col || !col->cells) return 0xFF;
    if (x >= col->size_x || y >= col->size_y || z >= col->size_z) return 0xFF;

    uint32_t idx = (uint32_t)y * (MAP_CHUNK_X * MAP_CHUNK_Z)
                    + (uint32_t)z * MAP_CHUNK_X
                    + (uint32_t)x;
                
    return col->cells[idx];
}

#endif // MAP_RUNTIME_H