// Auto-generated. DO NOT EDIT.

#include "generated/Collision.h"

Blob g_Collision[] = {
    { NULL, COLLISION_TEST_MAP_0_0_SIZE },
    { NULL, COLLISION_TEST_MAP_0_1_SIZE },
    { NULL, COLLISION_TEST_MAP_0_2_SIZE },
    { NULL, COLLISION_TEST_MAP_1_0_SIZE },
    { NULL, COLLISION_TEST_MAP_1_1_SIZE },
    { NULL, COLLISION_TEST_MAP_1_2_SIZE },
    { NULL, COLLISION_TEST_MAP_2_0_SIZE },
    { NULL, COLLISION_TEST_MAP_2_1_SIZE },
    { NULL, COLLISION_TEST_MAP_2_2_SIZE },
};

const size_t g_Collision_Count = 9;

void Collision_init(void) {
    g_Collision[0].data = platform_get_asset(COLLISION_TEST_MAP_0_0_OFFSET);
    g_Collision[1].data = platform_get_asset(COLLISION_TEST_MAP_0_1_OFFSET);
    g_Collision[2].data = platform_get_asset(COLLISION_TEST_MAP_0_2_OFFSET);
    g_Collision[3].data = platform_get_asset(COLLISION_TEST_MAP_1_0_OFFSET);
    g_Collision[4].data = platform_get_asset(COLLISION_TEST_MAP_1_1_OFFSET);
    g_Collision[5].data = platform_get_asset(COLLISION_TEST_MAP_1_2_OFFSET);
    g_Collision[6].data = platform_get_asset(COLLISION_TEST_MAP_2_0_OFFSET);
    g_Collision[7].data = platform_get_asset(COLLISION_TEST_MAP_2_1_OFFSET);
    g_Collision[8].data = platform_get_asset(COLLISION_TEST_MAP_2_2_OFFSET);
}
