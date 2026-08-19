#ifndef LOCALE_H
#define LOCALE_H

#include <stdint.h>
#include "dialogue/dialogue.h"
#include "generated/Locales.h"

#define LOCALE_MAGIC 0x4F4C4247       // "GBLO" (little-endian) — matches ExportCompiler.cs's LocaleMagic
#define LOCALE_VERSION 1

typedef struct {
    const char** strings; // points directly into the blob, never separately allocated
    uint16_t* lengths;
    uint32_t string_count;

    char* scratch;
    size_t scratch_capacity;
} LocaleDatabase;

extern LocaleDatabase g_ActiveLocale;

void locale_set_active(uint32_t locale_index);
void locale_free_active(void);

const char* locale_get_string(uint32_t text_id, uint16_t* out_length);
const char* locale_get_string_terminated(uint32_t text_id);

#endif // !LOCALES_H