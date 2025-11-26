#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	// Difficulty completions
	ACH_BEAT_SAGA,
	ACH_BEAT_FARARMADR,
	ACH_BEAT_VIKINGR,
	ACH_BEAT_BERSERKR,
	ACH_BEAT_EINHERJAR,
	ACH_BEAT_VIKINGR_NO_LOWER,		// Beat on Vikingr without lowering
	ACH_BEAT_BERSERKR_NO_LOWER,		// Beat on Berserkr without lowering
	ACH_BEAR_EINHERJAR_NO_LOWER,	// Beat on Einherjar without lowering

	// Hidden skills
	ACH_UNLOCK_DUAL_WIELD,
	ACH_UNLOCK_BERSERKER,
	ACH_UNLOCK_SHADOW_STEP,
	ACH_UNLOCK_AEGIS,
	ACH_UNLOCK_ALL_HIDDEN_COMBAT,   // All combat hidden skills

	// Glyphborn
	ACH_GLYPHBORN_AWAKENING,
	ACH_LEARN_10_GLYPHS,
	ACH_LEARN_50_GLYPHS,
	ACH_LEARN_ALL_GLYPHS,
	ACH_TRIGGER_100_VISIONS,
	ACH_SEER_KING,                  // Ultimate Glyphborn skill

	// Skill mastery
	ACH_MASTER_SWORD,               // One-Handed Sword 1000
	ACH_MASTER_5_SKILLS,            // Any 5 skills to 1000
	ACH_MASTER_10_SKILLS,
	ACH_MASTER_ALL_WEAPON_SKILLS,	// All weapon skills to 1000

	// Combat
	ACH_FIRST_KILL,
	ACH_KILL_100_ENEMIES,
	ACH_KILL_1000_ENEMIES,
	ACH_PERFECT_PARRY_100,
	ACH_DODGE_1000_ATTACKS,
	ACH_WIN_WITHOUT_DAMAGE,         // Complete fight without taking damage
	ACH_DEFEAT_LEGENDARY_WARRIOR,

	// Exploration
	ACH_DISCOVER_10_MAPS,
	ACH_DISCOVER_100_MAPS,
	ACH_DISCOVER_ALL_REGIONS,
	ACH_FIND_ALL_RUNESTONES,

	// Crafting
	ACH_FORGE_FIRST_WEAPON,
	ACH_CRAFT_LEGENDARY_ITEM,
	ACH_MASTER_BLACKSMITH,          // Blacksmithing 1000

	// Social
	ACH_PERSUADE_100_NPCS,
	ACH_INTIMIDATE_50_NPCS,
	ACH_TRADE_1M_GOLD,

	// Survival
	ACH_SURVIVE_100_DAYS,
	ACH_SURVIVE_1_YEAR,
	ACH_HUNT_100_ANIMALS,
	ACH_CATCH_LEGENDARY_FISH,

	// Misc/Hidden
	ACH_DIE_FIRST_TIME,             // You died!
	ACH_PERMADEATH_SUCCESS,         // Beat game with permadeath on
	ACH_NO_RESPEC,                  // Beat game without using respec
	ACH_PACIFIST,                   // Beat game killing <10 enemies
	ACH_SPEEDRUN_10H,               // Beat game in under 10 hours

	ACH_COUNT
} AchievementID;

typedef struct
{
	AchievementID id;
	const char* name;				// Display name
	const char* description;		// What you need to do
	const char* icon_path;			// Path to achievement icon
	bool is_hidden;					// Hide until unlocked
} AchievementDef;

typedef struct
{
	bool unlocked[ACH_COUNT];
	float progress[ACH_COUNT];
	uint32_t unlock_time[ACH_COUNT];
} AchievementData;

extern const AchievementDef achievement_defs[ACH_COUNT];
extern AchievementData g_achievements;
extern bool g_achievements_initialized;

void achievements_init(void);
void achievements_shutdown(void);
void achievements_update(void);
void achievements_unlock(AchievementID id);
bool achievements_is_unlocked(AchievementID id);
float achievements_get_progress(AchievementID id);
void achievements_set_progress(AchievementID id, float progress);
void achievements_save(void);
void achievements_load(void);

#ifdef DISTRO_STEAM
void steam_achievements_init(void);
void steam_achievements_update(void);
void steam_achievements_unlock(AchievementID id);
void steam_achievements_shutdown(void);
#endif

#ifdef DISTRO_GOG
void gog_achievements_init(void);
void gog_achievements_update(void);
void gog_achievements_unlock(AchievementID id);
void gog_achievements_shutdown(void);
#endif

#endif // !ACHIEVEMENTS_H
