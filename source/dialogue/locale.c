#include "dialogue/locale.h"
#include <stdlib.h>
#include <string.h>

LocaleDatabase g_ActiveLocale = {0};

void locale_free_active(void)
{
    free(g_ActiveLocale.strings);
    free(g_ActiveLocale.lengths);
    free(g_ActiveLocale.scratch);

    g_ActiveLocale.strings = NULL;
    g_ActiveLocale.lengths = NULL;
    g_ActiveLocale.scratch = NULL;

    g_ActiveLocale.string_count = 0;
    g_ActiveLocale.scratch_capacity = 0;
}

void locale_set_active(uint32_t locale_index) {
    if (locale_index >= g_Locales_Count) return;

    const Blob* blob = &g_Locales[locale_index];
    if (!blob || !blob->data) return;

    const uint8_t* ptr = blob->data;

    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);
    if (magic != LOCALE_MAGIC) return;

    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);
    if (version != LOCALE_VERSION) return;

    uint32_t string_count = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    const char** strings = malloc(sizeof(const char*) * string_count);
    uint16_t* lengths = malloc(sizeof(uint16_t) * string_count);

    for (uint32_t i = 0; i < string_count; i++) {
        uint16_t len = *(const uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        strings[i] = (const char*)ptr;
        ptr += len;

        lengths[i] = len;
    }

    // Only swap once the new locale has fully parsed, so a bad load never
    // leaves g_ActiveLocale half-updated.
    locale_free_active();
    g_ActiveLocale.strings = strings;
    g_ActiveLocale.lengths = lengths;
    g_ActiveLocale.string_count = string_count;
}

const char* locale_get_string(uint32_t text_id, uint16_t* out_length)
{
    if (out_length)
        *out_length = 0;

    if (text_id == NO_INDEX ||
        text_id >= g_ActiveLocale.string_count)
        return NULL;

    uint16_t length = g_ActiveLocale.lengths[text_id];

    if (g_ActiveLocale.scratch_capacity < (size_t)length + 1)
    {
        char* new_buffer =
            realloc(g_ActiveLocale.scratch, (size_t)length + 1);

        if (!new_buffer)
            return NULL;

        g_ActiveLocale.scratch = new_buffer;
        g_ActiveLocale.scratch_capacity = (size_t)length + 1;
    }

    memcpy(
        g_ActiveLocale.scratch,
        g_ActiveLocale.strings[text_id],
        length
    );

    g_ActiveLocale.scratch[length] = '\0';

    if (out_length)
        *out_length = length;

    return g_ActiveLocale.scratch;
}

const char* locale_get_string_terminated(uint32_t text_id)
{
    return locale_get_string(text_id, NULL);
}