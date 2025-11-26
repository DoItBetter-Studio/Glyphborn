#ifndef SKILLS_H
#define SKILLS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ABILITY_UNLOCKS		16
#define MAX_SKILL_LEVEL			1000

typedef float (*XPRequirementFunc)(uint16_t level);

typedef struct Skill
{
	const char* id;
	const char* name_key;
	const char* description_key;

	float xp;
	uint16_t level;
	bool is_hidden;

	XPRequirementFunc xp_required;

	uint16_t ability_unlocks[MAX_ABILITY_UNLOCKS];
} Skill;

void skill_init(Skill* skill, const char* id);

#endif // !SKILLS_H
