// Auto-generated. DO NOT EDIT.

#include "generated/Locales.h"

Blob g_Locales[] = {
    { NULL, LOCALES_EN_US_SIZE },
    { NULL, LOCALES_PT_BR_SIZE },
};

const size_t g_Locales_Count = 2;

void Locales_init(void) {
    g_Locales[0].data = platform_get_asset(LOCALES_EN_US_OFFSET);
    g_Locales[1].data = platform_get_asset(LOCALES_PT_BR_OFFSET);
}
