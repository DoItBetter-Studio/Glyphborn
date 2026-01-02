#include "achievements.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

AchievementData g_achievements = { 0 };
bool g_achievements_initialized = false;

const AchievementDef achievement_defs[ACH_COUNT] = {
	{
		ACH_BEAT_SAGA,
		"Saga",
		"Complete the game on Saga difficulty",
		"icons/ach_saga.png",
		false
	}
};

static uint32_t get_current_timestamp(void)
{
	return (uint32_t)time(NULL);
}

void achievements_init(void)
{
	if (g_achievements_initialized) return;

	memset(&g_achievements, 0, sizeof(AchievementData));
	achievements_load();

	g_achievements_initialized = true;

	#ifdef DISTRO_STEAM
	steam_achievements_init();
	#endif
}

void achievements_shutdown(void)
{
	if (!g_achievements_initialized) return;

	achievements_save();
	g_achievements_initialized = false;

	#ifdef DISTRO_STEAM
	steam_achievements_shutdown();
	#endif

}

void achievements_update(void)
{
	#ifdef DISTRO_STEAM
	steam_achievements_update();
	#endif
}

void achievements_unlock(AchievementID id)
{
	if (id >= ACH_COUNT || g_achievements.unlocked[id])
		return;

	g_achievements.unlocked[id] = true;
	g_achievements.progress[id] = 1.0f;
	g_achievements.unlock_time[id] = get_current_timestamp();

	achievements_save();

	const AchievementDef* def = &achievement_defs[id];
	printf("[Game] Achievement unlocked: %s\n", def->name);

	#ifdef DISTRO_STEAM
		steam_achievements_unlock(id);
	#endif
	#ifdef DISTRO_GOG
		gog_achievements_unlock(id);
	#endif
}

bool achievements_is_unlocked(AchievementID id)
{
	if (id >= ACH_COUNT) return false;
	return g_achievements.unlocked[id];
}

float achievements_get_progress(AchievementID id)
{
	if (id >= ACH_COUNT) return false;
	return g_achievements.progress[id];
}

void achievements_set_progress(AchievementID id, float progress)
{
	if (id >= ACH_COUNT) return;
	g_achievements.progress[id] = progress;

	if (progress >= 1.0f)
		achievements_unlock(id);

	achievements_save();
}

void achievements_save(void)
{
	FILE* fp = fopen("achievements.dat", "wb");
	if (fp)
	{
		fwrite(&g_achievements, sizeof(AchievementData), 1, fp);
		fclose(fp);
	}
}

void achievements_load(void)
{
	FILE* fp = fopen("achievements.dat", "rb");
	if (fp)
	{
		fread(&g_achievements, sizeof(AchievementData), 1, fp);
		fclose(fp);
	}
}
