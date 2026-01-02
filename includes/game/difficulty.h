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

	// Combar
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

static const GameSettings DIFFICULTY_PRESETS[DIFFICULTY_COUNT - 1] = {
	// STORY
	{
		DIFFICULTY_STORY,
		0.25f, 0.5f, 2.0f,
		3.0f, 9999, -1,
		false, false, true
	},
	// CASUAL
	{
		DIFFICULTY_CASUAL,
		0.5f, 0.75f, 1.5f,
		2.0f, 500, -1,
		false, false, true
	},
	// NORMAL
	{
		DIFFICULTY_NORMAL,
		1.0f, 1.0f, 1.0f,
		1.0f, 100, 5,
		false, false, true
	},
	// HARDCORE
	{
		DIFFICULTY_HARDCODE,
		1.5f, 1.5f, 1.0f,
		1.0f, 50, 1,
		true, false, false
	},
	// NIGHTMARE
	{
		DIFFICULTY_NIGHTMARE,
		2.0f, 2.0f, 0.75f,
		0.75f, 15, 0,
		true, true, false
	}
};

static GameSettings g_difficulty_settings;

inline void set_difficulty(DifficultyMode mode) {
	if (mode == DIFFICULTY_CUSTOM)
	{
		// Leave g_difficulty_settings as-is (already tweaked
		g_difficulty_settings.difficulty = DIFFICULTY_CUSTOM;
	}
	else {
		g_difficulty_settings = DIFFICULTY_PRESETS[mode];
	}
}

#endif // !DIFFICULTY_H
