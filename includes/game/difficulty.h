#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <stdbool.h>

typedef enum DifficultyMode
{
	DIFFICULTY_STORY,
	DIFFICULTY_CASUAL,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARDCODE,
	DIFFICULTY_NIGHTMARE,
	DIFFICULTY_CUSTOM,
	DIFFICULTY_COUNT
} DifficultyMode;

typedef struct {
	DifficultyMode difficulty;

	// Combat
	float enemy_damage_mult;
	float enemy_health_mult;
	float player_damage_mult;

	// Progression
	float xp_gain_mult;
	int skill_window_tolerance;
	bool allow_respec;

	// Survival
	bool survival_mode;
	bool permadeath;
	bool quest_markers;
} GameSettings;

extern const GameSettings DIFFICULTY_PRESETS[DIFFICULTY_COUNT - 1];
extern GameSettings g_difficulty_settings;

void set_difficulty(DifficultyMode mode);

#endif // !DIFFICULTY_H