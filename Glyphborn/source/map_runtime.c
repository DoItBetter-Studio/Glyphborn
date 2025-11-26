#include "map_runtime.h"

#include <string.h>
#include <stdlib.h>

// LE-safe decoders
static uint16_t read_u16_le(const uint8_t* p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t read_u32_le(const uint8_t* p)
{
    return (uint32_t)(p[0]
        | (p[1] << 8)
        | (p[2] << 16)
        | (p[3] << 24));
}

#define GEO_MAGIC0 'G'
#define GEO_MAGIC1 'B'
#define GEO_MAGIC2 'G'
#define GEO_MAGIC3 '0'

#define COL_MAGIC0 'G'
#define COL_MAGIC1 'B'
#define COL_MAGIC2 'C'
#define COL_MAGIC3 '0'

#define MAP_HEADER_SIZE 0x10

// -----------------------------------------------------------------------------
// map.bin: Geometry Loader (TileId volume)
// -----------------------------------------------------------------------------

int map_geometry_init_from_bin(MapGeometry* out, const uint8_t* data, size_t data_size)
{
    if (!out || !data) return 0;
    if (data_size < MAP_HEADER_SIZE + (MAP_VOXEL_COUNT * sizeof(TileId))) return 0;

    memset(out, 0, sizeof(*out));
    out->base = data;

    // Header
    const uint8_t* h = data;

    if (h[0] != GEO_MAGIC0 ||
        h[1] != GEO_MAGIC1 ||
        h[2] != GEO_MAGIC2 ||
        h[3] != GEO_MAGIC3)
    {
        return 0;
    }

    uint16_t version = read_u16_le(h + 4);
    if (version != 0x0001) return 0;

    uint8_t sx = h[6];
    uint8_t sy = h[7];
    uint8_t sz = h[8];

    if (sx != MAP_CHUNK_X ||
        sy != MAP_CHUNK_Y ||
        sz != MAP_CHUNK_Z)
    {
        return 0;
    }

    out->size_x = sx;
    out->size_y = sy;
    out->size_z = sz;

    // Volume begins immediately after header
    out->tiles = (const TileId*)(data + MAP_HEADER_SIZE);

    return 1;
}

// -----------------------------------------------------------------------------
// collision.bin: Collision Loader (uint8 volume)
// -----------------------------------------------------------------------------

int map_collision_init_from_bin(MapCollision* out, const uint8_t* data, size_t data_size)
{
    if (!out || !data) return 0;
    if (data_size < MAP_HEADER_SIZE + MAP_VOXEL_COUNT) return 0;

    memset(out, 0, sizeof(*out));
    out->base = data;

    // Header
    const uint8_t* h = data;

    if (h[0] != COL_MAGIC0 ||
        h[1] != COL_MAGIC1 ||
        h[2] != COL_MAGIC2 ||
        h[3] != COL_MAGIC3)
    {
        return 0;
    }

    uint16_t version = read_u16_le(h + 4);
    if (version != 0x0001) return 0;

    uint8_t sx = h[6];
    uint8_t sy = h[7];
    uint8_t sz = h[8];

    if (sx != MAP_CHUNK_X ||
        sy != MAP_CHUNK_Y ||
        sz != MAP_CHUNK_Z)
    {
        return 0;
    }

    out->size_x = sx;
    out->size_y = sy;
    out->size_z = sz;

    out->cells = (const uint8_t*)(data + MAP_HEADER_SIZE);

    return 1;
}

// -----------------------------------------------------------------------------
// Combined Loader
// -----------------------------------------------------------------------------

int map_runtime_init(
    MapRuntime* out,
    const uint8_t* map_bin, size_t map_bin_size,
    const uint8_t* col_bin, size_t col_bin_size)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!map_geometry_init_from_bin(&out->geo, map_bin, map_bin_size))      return 0;

    if (!map_collision_init_from_bin(&out->col, col_bin, col_bin_size))     return 0;

    return 1;
}


// ============================================================================
//  METADATA (matrix.bin)
// ============================================================================
//
// Format for matrix.bin (for now, simple metadata table):
//
// uint16 entry_count
// MetadataEntry[entry_count]
//
// where MetadataEntry matches the MapMetadata binary layout:
//
//   char name[32];
//   char area_type[16];
//   char icon[32];
//   char music_day[32];
//   char music_night[32];
//   char weather[16];
//   uint16 map_id;
//   uint16 flags;
//
// Total per entry = 32 + 16 + 32 + 32 + 32 + 16 + 2 + 2 = 164 bytes.
//

#define META_ENTRY_SIZE         164
#define META_OFF_NAME           0
#define META_OFF_AREA_TYPE      32
#define META_OFF_ICON           48
#define META_OFF_MUSIC_DAY      80
#define META_OFF_MUSIC_NIGHT    112
#define META_OFF_WEATHER        144
#define META_OFF_MAP_ID         160
#define META_OFF_FLAGS          162

int map_metadata_init_from_matrix(MapMetadata* out, const uint8_t* data, size_t data_size, uint16_t map_id)
{
    if (!out || !data) return 0;
    if (data_size < 2) return 0;

    uint16_t count = read_u16_le(data);
    size_t required = 2 + (size_t)count * META_ENTRY_SIZE;
    if (data_size < required) return 0;

    const uint8_t* entries = data + 2;

    for (uint16_t i = 0; i < count; ++i)
    {
        const uint8_t* e = entries + (size_t)i * META_ENTRY_SIZE;
        uint16_t entry_map_id = read_u16_le(e + META_OFF_MAP_ID);

        if (entry_map_id == map_id)
        {
            memset(out, 0, sizeof(*out));

            memcpy(out->name,           e + META_OFF_NAME,          sizeof(out->name));
            memcpy(out->area_type,      e + META_OFF_AREA_TYPE,     sizeof(out->area_type));
            memcpy(out->icon,           e + META_OFF_ICON,          sizeof(out->icon));
            memcpy(out->music_day,      e + META_OFF_MUSIC_DAY,     sizeof(out->music_day));
            memcpy(out->music_night,    e + META_OFF_MUSIC_NIGHT,   sizeof(out->music_night));
            memcpy(out->weather,        e + META_OFF_WEATHER,       sizeof(out->weather));

            out->map_id = entry_map_id;
            out->flags  = read_u16_le(e + META_OFF_FLAGS);

            return 1;
        }
    }

    // Not found
    return 0;
}

// ============================================================================
//  EVENTS (events.bin)
// ============================================================================
//
// Format:
//   uint16 event_count
//   MapEvent[event_count]
//
// MapEvent layout in file (little-endian):
//   uint16 x, y, z;
//   uint16 type;
//   uint16 script_id;
// total 10 bytes per entry.
//

int map_events_init_from_bin(MapEvents* out, const uint8_t* data, size_t data_size)
{
    if (!out || !data) return 0;
    if (data_size < 2) return 0;

    uint16_t count = read_u16_le(data);
    size_t required = 2 + (size_t)count * sizeof(MapEvent);
    if (data_size < required) return 0;

    memset(out, 0, sizeof(*out));

    out->count = count;

    // If events.bin comes from .incbin, this memory is static and safe to alias.
    // If loaded from a file, caller must ensure the buffer stays alive.
    out->list = (MapEvent*)(data + 2);

    return 1;
}

void map_events_dispose(MapEvents* events)
{
    if (!events) return;

    // We did not allocate events->list; it's just a view into a blob.
    // Nothing to free; just clear.
    memset(events, 0, sizeof(*events));
}

// ============================================================================
//  SCRIPTS (scripts.inc + mrb bank)
// ============================================================================
//
// We assume scripts.inc (or scripts.bin) has this binary layout:
//
//   uint32 script_count
//   MapScriptEntry[script_count]
//
// where MapScriptEntry is exactly:
//   uint32 id;
//   uint32 offset;
//   uint32 length;
//
// All LE-encoded.
//

int map_scripts_init(MapScripts* out, const uint8_t* table_data, size_t table_size, const uint8_t* bank_data, size_t back_size)
{
    (void)back_size; // not used yet, but kept for future validation

    if (!out || !table_data || !bank_data) return 0;
    if (table_size < 4) return 0;

    uint32_t count = read_u32_le(table_data);
    size_t required = 4 + (size_t)count * sizeof(MapScriptEntry);
    if (table_size < required) return 0;

    memset(out, 0, sizeof(*out));

    out->count      = count;
    out->entries    = (MapScriptEntry*)(table_data + 4);
    out->bank       = bank_data;

    return 1;
}

void map_scripts_dispose(MapScripts* scripts)
{
    if (!scripts) return;

    // entries + bank are just views into external blobs
    memset(scripts, 0, sizeof(*scripts));
}

// ============================================================================
//  TEXT (text.bin)
// ============================================================================
//
// Format:
//   uint16 text_count
//   struct {
//       uint32 offset; // offset from start of file
//       uint32 length;
//   } entry[text_count]
//   [text blob...]
//
// Offsets are absolute (relative to the start of text.bin).
//

int map_text_init_from_bin(MapText* out, const uint8_t* data, size_t data_size)
{
    if (!out || !data) return 0;
    if (data_size < 2) return 0;

    uint16_t count = read_u16_le(data);
    size_t table_size = 2 + (size_t)count * 8;
    if (data_size < table_size) return 0;

    const uint8_t* entry_ptr = data + 2;

    memset(out, 0, sizeof(*out));

    out->count = count;
    out->entries = (MapTextEntry*)malloc((size_t)count * sizeof(MapTextEntry));
    if (!out->entries)
    {
        out->count = 0;
        return 0;
    }

    for (uint16_t i = 0; i < count; ++i)
    {
        const uint8_t* e = entry_ptr + (size_t)i * 8;
        uint32_t offset = read_u32_le(e + 0);
        uint32_t length = read_u32_le(e + 0);

        if ((size_t)offset + (size_t)length > data_size)
        {
            free(out->entries);
            memset(out, 0, sizeof(*out));
            return 0;
        }

        out->entries[i].data    = (const char*)(data + offset);
        out->entries[i].length  = length;
    }

    return 1;
}

void map_text_dispose(MapText* text)
{
    if (!text) return;

    free(text->entries);
    memset(text, 0, sizeof(*text));
}

// ============================================================================
//  Ruby & Lua VM stubs (to be wired to mruby / Lua later)
// ============================================================================

int ruby_init_for_map(RubyScriptVM* vm, const MapScripts* scripts)
{
    (void)scripts;

    if (!vm) return 0;
    memset(vm, 0, sizeof(*vm));

    // TODO: hook mruby init here.
    // vm->mrb = mrb_open();
    // gb_ruby_bind_api(vm->mrb);
    // etc...

    return 1; // stub: success
}

void ruby_shutdown_for_map(RubyScriptVM* vm)
{
    if (!vm) return;

    // TOOD: call mrb_close(vm->mrb);
    vm->mrb = NULL;
    vm->user = NULL;
}

// Lua

int lua_init_for_map(LuaModVM* vm)
{
    if (!vm) return 0;
    memset(vm, 0, sizeof(*vm));

    // TODO: vm->L = luaL_newstate();
    // luaL_openlibs(vm->L);
    // load mod scripts, bind API, etc.

    return 1; // stud: success
}

void lua_shutdown_for_map(LuaModVM* vm)
{
    if (!vm) return;

    // TODO: lua_close(vm->L);
    vm->L = NULL;
}

// ============================================================================
//  Full MapRuntime initialization / disposal
// ============================================================================

int map_runtime_full_init(
    MapRuntime* out,
    const uint8_t* map_bin,             size_t map_bin_size,
    const uint8_t* col_bin,             size_t col_bin_size,
    const uint8_t* events_bin,          size_t events_size,
    const uint8_t* scripts_table,       size_t scripts_table_size,
    const uint8_t* text_bin,            size_t text_size,
    const uint8_t* matrix_bin,          size_t matrix_size,
    const uint8_t* mrb_bank,            size_t mrb_bank_size,
    uint16_t       map_id)
{
    if (!out) return 0;

    if (!map_runtime_init(out, map_bin, map_bin_size, col_bin, col_bin_size)) return 0;

    if (!map_metadata_init_from_matrix(&out->meta, matrix_bin, matrix_size, map_id)) return 0;

    if (!map_events_init_from_bin(&out->events, events_bin, events_size)) return 0;

    if (!map_scripts_init(&out->scripts, scripts_table, scripts_table_size, mrb_bank, mrb_bank_size)) return 0;

    if (!map_text_init_from_bin(&out->text, text_bin, text_size)) return 0;

    if (!ruby_init_for_map(&out->ruby, &out->scripts)) return 0;

    if (!lua_init_for_map(&out->lua)) return 0;

    // mesh_cache remains invalid until you build meshes
    memset(&out->mesh_cache, 0, sizeof(out->mesh_cache));

    return 1;
}

void map_runtime_dispose(MapRuntime* map)
{
    if (!map) return;

    // Tear down VMs
    ruby_shutdown_for_map(&map->ruby);
    lua_shutdown_for_map(&map->lua);

    // Free text entries
    map_text_dispose(&map->text);

    // Events + scripts + geo + col are views into external blobs,
    // so we don't free them here. Just clear the struct.
    map_events_dispose(&map->events);
    map_scripts_dispose(&map->scripts);

    // mesh_cahce ownership depends on how you allocate; for now assume
    // it's cleaned elsewhere or not yet used.

    memset(map, 0, sizeof(*map));
}