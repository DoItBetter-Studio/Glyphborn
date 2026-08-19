#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// A save file is a header followed by a sequence of independently-sized,
// tagged chunks. Each system owns one chunk type and reads/writes only its
// own bytes -- nothing here knows what a PersonRecord or Settlement is.
//
// Unknown chunk types (from a newer build) are skipped via `size`. Missing
// chunk types (loading an older save with a newer build) simply aren't
// called -- that system's init() should already leave it in a valid empty
// state, which is exactly what entity_pool_init/etc already do.

#define SAVE_MAGIC      0x4759504Bu // 'GYPK'
#define SAVE_VERSION    1

typedef enum
{
    CHUNK_ACHIEVEMENTS      = 1,
    CHUNK_PLAYER_STATE      = 2,
    CHUNK_PERSON_REGISTRY   = 3,
    CHUNK_SETTLEMENTS       = 4,
    CHUNK_WORLD_DELTAS      = 5
} SaveChunkType;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t chunk_count;
} SaveHeader;

typedef struct
{
    uint32_t type;      // SaveChunkType
    uint32_t version;   // pre-chunk version --lets one system evolve independently
    uint64_t size;      // byte length of this chunk's data, immediately following
} SaveChunkHeader;

// A system implements these and registers them once (typically from its own
// _init function). `read` receives the chunk's declared size so it can skip
// trailing bytes from a newer chunk version it doesn't understand yet.
typedef void (*SaveChunkWriteFn)(FILE* fp);
typedef void (*SaveChunkReadFn)(FILE* fp, uint32_t chunk_version, uint64_t size);
 
void save_system_register_chunk(SaveChunkType type, uint32_t version,
                                  SaveChunkWriteFn write_fn, SaveChunkReadFn read_fn);
 
// `slot_name` becomes a filename under the platform save directory --
// save_get_directory() supplies the directory, this just appends
// "<slot_name>.gbsav". For achievements this is a fixed global name; for
// world saves the caller passes a path under saves/worlds/<world_name>/.
bool save_game(const char* slot_name);
bool load_game(const char* slot_name);
 
// Implemented per-platform (save_load_windows.c / save_load_linux.c).
// Writes a writable, OS-appropriate, trailing-separator-terminated
// directory path into `out` (creating it if necessary) and returns true,
// or returns false if it couldn't be determined/created.
bool save_get_directory(char* out, size_t out_size);

#endif // SAVE_LOAD_H