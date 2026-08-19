// Auto-generated. DO NOT EDIT.

#include "generated/Materials.h"

Blob g_Materials[] = {
    { NULL, MATERIALS_BODY_SIZE },
    { NULL, MATERIALS_EYE_SIZE },
};

const size_t g_Materials_Count = 2;

void Materials_init(void) {
    g_Materials[0].data = platform_get_asset(MATERIALS_BODY_OFFSET);
    g_Materials[1].data = platform_get_asset(MATERIALS_EYE_OFFSET);
}
