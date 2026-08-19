#ifndef SKILL_REGISTRY_H
#define SKILL_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>

// This entire structure is designed to be loaded straight out of a data
// volume via platform_get_asset() and used in place -- no pointers, no
// variable-length arrays, no relocation. Same philosophy as world_matrix
// and world_headers: a flat POD blob that's just cast over mapped memory.
//
// All ids (skill ids, name/description keys) are pre-hashed uint32_t values
// computed by the Damascus skill editor at export time. Nothing here is a
// human-readable string in the shipped binary or volumes.

#define SKILL_REGISTRY_MAGIC    0x534B4C52u // 'SKLR'

#define MAX_SKILL_DEFS          128
#define MAX_TECHNIQUE_DEFS      512
#define MAX_SYNTHESIS_RULES     64

// Shared level cap for both skills and techniques -- skill_award_xp clamps
// here so a single large xp grant can't overshoot past 1000.
#define MAX_SKILL_LEVEL     1000

// XP curves can't be function pointers in mapped data, so the curve is
// selected by enum + two float parameters. See skill_xp_required() for
// how each curve interprets param_a / param_b.
typedef enum
{
	XP_CURVE_LINEAR,                  // required = param_a * level
	XP_CURVE_EXPONENTIAL,             // required = param_a * pow(param_b, level)
	XP_CURVE_LOG_EARLY_STEEP_LATE,    // required = param_a * level^param_b
} XPCurveType;

typedef struct
{
    uint32_t id;                // hashed skill identifier (e.g. hash("skill.sword"))
    uint32_t name_key;          // hashed localization key for display name
    uint32_t description_key;

    float success_xp;
    float failure_xp;

    XPCurveType curve_type;
    float curve_param_a;
    float curve_param_b;

    bool is_hidden;              // true for synthesis-only result skills until unlocked
} SkillDef;

typedef struct
{
    uint32_t id;                // hashed technique identifier
    uint32_t parent_skill_id;
    uint32_t name_key;
    uint32_t description_key;

    uint16_t unlock_level;      // parent skill level required before this technique appears

    float success_xp;
    float failure_xp;

    XPCurveType curve_type;
    float curve_param_a;
    float curve_param_b;
    
    // Tile hit pattern, relative to the entity's facing, resolved at
	// attack time by the combat system. hit_count may be 0 for
	// non-combat techniques (e.g. a crafting technique).
	int8_t hit_dx[8];
	int8_t hit_dy[8];
	int8_t hit_dz[8];
	uint8_t hit_count;
} TechniqueDef;

// A+B level windows -> unlocks `result_skill_id`. Checked on level-up only.
// Window is one-sided and inclusive: a skill is "in window" when
//   level >= required_level && level <= required_level + skill_window_tolerance
// Overshooting the window on a high-tolerance difficulty is effectively
// impossible; on low-tolerance difficulties it permanently closes the rule.
typedef struct
{
    uint32_t result_skill_id;

    uint32_t req_skill_a_id;
    uint16_t req_skill_a_level;

    uint32_t req_skill_b_id;
    uint16_t req_skill_b_level;
} SynthesisRule;

typedef struct
{
    uint32_t magic;             // SKILL_REGISTRY_MAGIC, sanity checked on load

    uint32_t skill_count;
    SkillDef skill_defs[MAX_SKILL_DEFS];

    uint32_t technique_count;
    TechniqueDef technique_defs[MAX_TECHNIQUE_DEFS];

    uint32_t synthesis_count;
    SynthesisRule synthesis_rules[MAX_SYNTHESIS_RULES];
} SkillRegistry;


// Looks up a SkillDef by hashed id. Returns NULL if not found.
const SkillDef* skill_registry_find_skill(const SkillRegistry* reg, uint32_t skill_id);
 
// Looks up a TechniqueDef by hashed id. Returns NULL if not found.
const TechniqueDef* skill_registry_find_technique(const SkillRegistry* reg, uint32_t technique_id);
 
// Evaluates the xp required to reach `level` for a given curve.
float skill_xp_required(XPCurveType curve, float param_a, float param_b, uint16_t level);

#endif // SKILL_REGISTRY_H