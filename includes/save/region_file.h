#ifndef REGION_FILE_H
#define REGION_FILE_H

#include "save_load.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// A region file covers an N x N block of WorldCells and streams alongside
// world_update's 3x3 active grid -- when a WorldCell at (cell_x, cell_y)
// loads, the region file covering it is opened (if not already) and that
// cell's slot is read; on unload, the slot is (re)written.
//
// REGION_SIZE 8 means a player near a region boundary touches at most a
// 2x2 set of region files even at the edges of the 3x3 cell grid.
#define REGION_SIZE   8
#define REGION_MAGIC  0x47524547u // 'GREG'
#define REGION_VERSION 1

typedef struct
{
	uint32_t magic;
	uint32_t version;
	uint32_t region_x, region_y;

	// Byte offset of each cell's slot within the file, 0 = never written
	// (cell has no PersonRecords/WorldDeltas yet -- registries simply
	// stay at their init() defaults for that cell).
	uint64_t cell_offsets[REGION_SIZE * REGION_SIZE];
} RegionHeader;

typedef struct
{
	FILE* fp;
	RegionHeader header;
} RegionFile;

// Per-cell chunk callbacks -- unlike save_load.h's global handlers, these
// are scoped to one WorldCell at a time, identified by its world cell
// coordinates. A system (e.g. the person registry) registers once and
// these are called for whichever cell is currently being loaded/saved.
typedef void (*RegionChunkWriteFn)(FILE* fp, uint32_t cell_world_x, uint32_t cell_world_y);
typedef void (*RegionChunkReadFn)(FILE* fp, uint32_t cell_world_x, uint32_t cell_world_y,
                                   uint32_t chunk_version, uint64_t size);

void region_register_chunk(SaveChunkType type, uint32_t version,
                             RegionChunkWriteFn write_fn, RegionChunkReadFn read_fn);

// Converts a WorldCell's world coordinates to the region coordinates that
// contain it (plain integer division -- Glyphborn's world matrix is
// entirely non-negative, so no floor-division correction is needed).
void region_coords_from_cell(uint32_t cell_world_x, uint32_t cell_world_y,
                               uint32_t* out_region_x, uint32_t* out_region_y);

// Opens (creating if necessary) the region file for (region_x, region_y)
// under <world_dir>/regions/. A fresh file gets a zeroed offset table --
// every cell starts empty.
bool region_file_open(RegionFile* region, const char* world_dir, uint32_t region_x, uint32_t region_y);
void region_file_close(RegionFile* region);

// Loads/saves the slot for the cell at (cell_world_x, cell_world_y), which
// must fall within `region`'s bounds. Dispatches to every registered chunk
// handler in turn, same tagged-chunk format as save_load.h.
//
// region_file_save_cell appends the new slot at end-of-file and updates the
// offset table -- the old slot (if any) becomes a hole. This trades disk
// space for simplicity; a future offline compaction pass can reclaim holes
// by rewriting the file with only the offsets currently in use.
bool region_file_load_cell(RegionFile* region, uint32_t cell_world_x, uint32_t cell_world_y);
bool region_file_save_cell(RegionFile* region, uint32_t cell_world_x, uint32_t cell_world_y);

#endif // !REGION_FILE_H