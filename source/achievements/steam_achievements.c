#include "achievements.h"
#include "stdio.h"

#ifdef DISTRO_STEAM

#include "steam/steam_api.h"

static bool steam_initialized = false;

bool steam_achievements_init(void)
{
    if (steam_initialized) return;

    printf("[Steam] Initializing Steam API...\n");
    if (!SteamAPI_Init())
    {
        fprintf(stderr, "[Steam] Failed to initialize Steam API.\n");
        return;
    }
    steam_initialized = true;
}

void steam_achievements_update(void)
{
    if (!steam_initialized) return;
    SteamAPI_RunCallbacks();
}

void steam_achievements_unlock(AchievementID id)
{
    if (!steam_initialized) return;

    const AchievementDef* def = &achievement_defs[id];
    printf("[Steam] Mirroring unlock: %s\n", def->name);

    if (SteamUserStats()) {
        SteamUserStats()->SetAchievement(def->name);
        SteamUserStats()->StoreStats();
    }
}

void steam_achievements_shutdown(void)
{
    if (!steam_initialized) return;
    printf("[Steam] Shutting down...\n");
    SteamAPI_Shutdown();
    steam_initialized = false;
}
#endif