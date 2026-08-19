#include "save/region_file.h"
#include <stdlib.h>
#include <string.h>

#define MAX_REGION_CHUNK_HANDLERS 8

typedef struct
{
	SaveChunkType type;
	uint32_t version;
	RegionChunkWriteFn write_fn;
	RegionChunkReadFn read_fn;
} RegionChunkHandler;

static RegionChunkHandler s_handlers[MAX_REGION_CHUNK_HANDLERS];
static int32_t s_handler_count = 0;

void region_register_chunk(SaveChunkType type, uint32_t version,
                             RegionChunkWriteFn write_fn, RegionChunkReadFn read_fn)
{
	if (s_handler_count >= MAX_REGION_CHUNK_HANDLERS)
		return;

	RegionChunkHandler* h = &s_handlers[s_handler_count++];
	h->type = type;
	h->version = version;
	h->write_fn = write_fn;
	h->read_fn = read_fn;
}

void region_coords_from_cell(uint32_t cell_world_x, uint32_t cell_world_y,
                               uint32_t* out_region_x, uint32_t* out_region_y)
{
	*out_region_x = cell_world_x / REGION_SIZE;
	*out_region_y = cell_world_y / REGION_SIZE;
}

static int32_t cell_slot_index(uint32_t cell_world_x, uint32_t cell_world_y)
{
	uint32_t local_x = cell_world_x % REGION_SIZE;
	uint32_t local_y = cell_world_y % REGION_SIZE;
	return (int32_t)(local_y * REGION_SIZE + local_x);
}

bool region_file_open(RegionFile* region, const char* world_dir, uint32_t region_x, uint32_t region_y)
{
	char path[768];
	snprintf(path, sizeof(path), "%sregions/r.%u.%u.gbreg", world_dir, region_x, region_y);

	// Try to open an existing region file first.
	FILE* fp = fopen(path, "r+b");
	if (fp)
	{
		if (fread(&region->header, sizeof(RegionHeader), 1, fp) != 1 ||
		    region->header.magic != REGION_MAGIC)
		{
			fclose(fp);
			return false;
		}

		region->fp = fp;
		return true;
	}

	// Doesn't exist yet -- create it with a fresh, all-empty header.
	fp = fopen(path, "w+b");
	if (!fp)
		return false; // e.g. "regions/" doesn't exist -- caller must mkdir it

	memset(&region->header, 0, sizeof(RegionHeader));
	region->header.magic = REGION_MAGIC;
	region->header.version = REGION_VERSION;
	region->header.region_x = region_x;
	region->header.region_y = region_y;

	fwrite(&region->header, sizeof(RegionHeader), 1, fp);
	region->fp = fp;
	return true;
}

void region_file_close(RegionFile* region)
{
	if (region->fp)
	{
		fclose(region->fp);
		region->fp = NULL;
	}
}

bool region_file_load_cell(RegionFile* region, uint32_t cell_world_x, uint32_t cell_world_y)
{
	int32_t idx = cell_slot_index(cell_world_x, cell_world_y);
	uint64_t offset = region->header.cell_offsets[idx];

	if (offset == 0)
		return true; // empty cell -- registries simply stay at init() defaults

	if (fseek(region->fp, (int64_t)offset, SEEK_SET) != 0)
		return false;

	uint32_t chunk_count;
	if (fread(&chunk_count, sizeof(chunk_count), 1, region->fp) != 1)
		return false;

	for (uint32_t i = 0; i < chunk_count; i++)
	{
		SaveChunkHeader chunk_header;
		if (fread(&chunk_header, sizeof(chunk_header), 1, region->fp) != 1)
			break;

		int64_t data_start = ftell(region->fp);

		RegionChunkHandler* h = NULL;
		for (int32_t j = 0; j < s_handler_count; j++)
		{
			if (s_handlers[j].type == (SaveChunkType)chunk_header.type)
			{
				h = &s_handlers[j];
				break;
			}
		}

		if (h)
			h->read_fn(region->fp, cell_world_x, cell_world_y, chunk_header.version, chunk_header.size);

		// Skip to the declared end of this chunk regardless -- safe for
		// unknown/newer chunk types or partially-handled versions.
		fseek(region->fp, data_start + (int64_t)chunk_header.size, SEEK_SET);
	}

	return true;
}

bool region_file_save_cell(RegionFile* region, uint32_t cell_world_x, uint32_t cell_world_y)
{
	int32_t idx = cell_slot_index(cell_world_x, cell_world_y);

	// New slot always goes at end-of-file -- the old slot (if any) becomes
	// a hole, reclaimed only by a future compaction pass (see region_file.h).
	if (fseek(region->fp, 0, SEEK_END) != 0)
		return false;

	uint64_t new_offset = (uint64_t)ftell(region->fp);

	uint32_t chunk_count = (uint32_t)s_handler_count;
	fwrite(&chunk_count, sizeof(chunk_count), 1, region->fp);

	for (int32_t i = 0; i < s_handler_count; i++)
	{
		RegionChunkHandler* h = &s_handlers[i];

		SaveChunkHeader chunk_header = { (uint32_t)h->type, h->version, 0 };
		int64_t header_pos = ftell(region->fp);
		fwrite(&chunk_header, sizeof(chunk_header), 1, region->fp);

		int64_t data_start = ftell(region->fp);
		h->write_fn(region->fp, cell_world_x, cell_world_y);
		int64_t data_end = ftell(region->fp);

		chunk_header.size = (uint64_t)(data_end - data_start);
		fseek(region->fp, header_pos, SEEK_SET);
		fwrite(&chunk_header, sizeof(chunk_header), 1, region->fp);
		fseek(region->fp, data_end, SEEK_SET);
	}

	region->header.cell_offsets[idx] = new_offset;

	// Rewrite the header in place -- it's a fixed size at offset 0, so this
	// never disturbs any cell slot's data.
	fseek(region->fp, 0, SEEK_SET);
	fwrite(&region->header, sizeof(RegionHeader), 1, region->fp);

	return true;
}