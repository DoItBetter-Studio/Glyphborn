#ifndef SKILL_SET_H
#define SKILL_SET_H

#include "skill_registry.h"

#include <stdint.h>
#include <stdbool.h>

// Skills and techniques both live in this fixed-size table on the entity.
// Definitions (xp curves, costs, unlock requirements) live in the
// SkillRegistry -- this struct only holds per-entity progression.
//
// `id` is a hashed identifier (see skill_registry.h) that refers to either
// a SkillDef or a TechniqueDef. Both skills and techniques share the same
// progression shape (level + xp), so they share this table rather than
// having separate arrays.

#define MAX_ENTITY_SKILLS 64

typedef struct
{
    uint32_t id;            // hashed skill/technique id, 0 = empty slot
    float xp;
    uint16_t level;
    bool unlocked;          // false until a synthesis rule (or unlock_level) grants it
} SkillState;

typedef struct
{
    SkillState skills[MAX_ENTITY_SKILLS];
    int count;
} SkillSet;

// What kind of outcome an action produced. Determines which xp value from
// the registry is awarded. CRITICAL is reserved for future use (masterwork
// crafting, perfect parries, etc.) -- not awarded by any system yet.
typedef enum
{
    XP_OUTCOME_FAILURE,
    XP_OUTCOME_SUCCESS,
    XP_OUTCOME_CRITICAL
} XPOutcome;

// Forward declaration -- full definition in skill_registry.h. Only a pointer
// is needed here, so we avoid a circular include.
struct SkillRegistry;

// Find an existing entry for `skill_id`. Returns NULL if the entity has
// never had xp awarded toward this skill (i.e. it has no slot yet).
SkillState* skill_set_find(SkillSet* set, uint32_t skill_id);
 
// Add a new slot for `skill_id` at `starting_level` with 0 xp. Used by the
// NPC loader to seed authored starting skills, and by synthesis unlocks.
// Returns NULL if the set is full (MAX_ENTITY_SKILLS reached).
SkillState* skill_set_add(SkillSet* set, uint32_t skill_id, uint16_t starting_level);
 
// Award xp for `skill_id` based on `outcome`, using values from `reg`.
// If `skill_id` has no slot yet, one is created at level 0.
// On level-up, runs the synthesis check (skill_set_check_synthesis).
void skill_award_xp(SkillSet* set, const SkillRegistry* reg,
                     uint32_t skill_id, XPOutcome outcome);
 
// Run the synthesis rule table against the current skill levels using
// g_difficulty_settings.skill_window_tolerance. Unlocks any result skills
// whose conditions are met and not already present. Called automatically
// by skill_award_xp on level-up -- exposed separately for tooling/tests.
void skill_set_check_synthesis(SkillSet* set, const SkillRegistry* reg);

#endif // SKILL_SET_H