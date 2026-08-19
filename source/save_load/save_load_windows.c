#include "save/save_load.h"
#include <windows.h>
#include <shlobj.h>
#include <string.h>

// ---------------------------------------------------------------------
// Chunk registration / dispatch
// ---------------------------------------------------------------------

#define MAX_SAVE_CHUNK_HANDLERS 16

typedef struct
{
    SaveChunkType type;
    uint32_t version;
    SaveChunkWriteFn write_fn;
    SaveChunkReadFn read_fn;
} SaveChunkHandler;

static SaveChunkHandler s_handlers[MAX_SAVE_CHUNK_HANDLERS];
static int32_t s_handler_count = 0;

void save_system_register_chunk(SaveChunkType type, uint32_t version, SaveChunkWriteFn write_fn, SaveChunkReadFn read_fn)
{
    if (s_handler_count >=  MAX_SAVE_CHUNK_HANDLERS)
        return;

    SaveChunkHandler* h = &s_handlers[s_handler_count++];
    h->type = type;
    h->version = version;
    h->write_fn = write_fn;
    h->read_fn = read_fn;
}

// ---------------------------------------------------------------------
// Path resolution -- %APPDATA%\Glyphborn\saves
// ---------------------------------------------------------------------

bool save_get_directory(char* out, size_t out_size)
{
    char appdata[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata) != S_OK)
        return false;
    
    char glyphborn_dir[MAX_PATH];
    snprintf(glyphborn_dir, sizeof(glyphborn_dir), "%s\\Glyphborn", appdata);
    CreateDirectoryA(glyphborn_dir, NULL); // ok it it already exists

    int32_t written = snprintf(out, out_size, "%s\\saves\\", glyphborn_dir);
    if (written < 0 || (size_t)written >= out_size)
        return false;
    
    CreateDirectoryA(out, NULL); // ok if it already exists
    return true;
}

// ---------------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------------

bool save_game(const char* slot_name)
{
    char dir[512];
    if (!save_get_directory(dir, sizeof(dir)))
        return false;
    
    char path[768];
    snprintf(path, sizeof(path), "%s%s.gbsav", dir, slot_name);

    FILE* fp = fopen(path, "wb");
    if (!fp)
        return false;

    SaveHeader header = { SAVE_MAGIC, SAVE_VERSION, (uint32_t)s_handler_count };
    fwrite(&header, sizeof(header), 1, fp);

    for (int32_t i = 0; i < s_handler_count; i++)
    {
        SaveChunkHandler* h = &s_handlers[i];

        SaveChunkHeader chunk_header = { (uint32_t)h->type, h->version, 0 };
        int64_t header_pos = ftell(fp);
        fwrite(&chunk_header, sizeof(chunk_header), 1, fp);

        int64_t data_start = ftell(fp);
        h->write_fn(fp);
        int64_t data_end = ftell(fp);

        // Patch in the real size now that we know it -- chunk contents
        // are variable-length (counts + flat arrays), so size can't be
        // known up front.
        chunk_header.size = (uint64_t)(data_end - data_start);
        fseek(fp, header_pos, SEEK_SET);
        fwrite(&chunk_header, sizeof(chunk_header), 1, fp);
        fseek(fp, data_end, SEEK_SET);
    }

    fclose(fp);
    return true;
}

bool load_game(const char* slot_name)
{
    char dir[512];
    if (!save_get_directory(dir, sizeof(dir)))
        return false;
    
    char path[768];
    snprintf(path, sizeof(path), "%s%s.gbsav", dir, slot_name);

    FILE* fp = fopen(path, "rb");
    if (!fp)
        return false; // no save yet -- not an error, systems stay at init() defaults
    
    SaveHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1 || header.magic != SAVE_MAGIC)
    {
        fclose(fp);
        return false;
    }

    for (uint32_t i = 0; i < header.chunk_count; i++)
    {
        SaveChunkHeader chunk_header;
        if (fread(&chunk_header, sizeof(chunk_header), 1, fp) != 1)
            break;
        
        int64_t data_start = ftell(fp);

        SaveChunkHandler* h = NULL;
        for (int32_t j = 0; j < s_handler_count; j++)
        {
            if (s_handlers[j].type == (SaveChunkType)chunk_header.type)
            {
                h = &s_handlers[j];
                break;
            }
        }

        if (h)
            h->read_fn(fp, chunk_header.version, chunk_header.size);

        // Seek to the declared end of this chunk regardless -- this is
		// what makes unknown/newer chunks safe to skip.
		fseek(fp, data_start + (int64_t)chunk_header.size, SEEK_SET);
    }

    fclose(fp);
    return true;
}