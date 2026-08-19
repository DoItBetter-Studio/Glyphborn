#ifndef PERSON_RECORD_H
#define PERSON_RECORD_H

#include "entity_id.h"
#include "components.h"
#include "skill_set.h"
#include <stdint.h>
#include <stdbool.h>

#define PERSON_NAME_LEN 32

// The persistent record for every NPC and animal that has ever existed --
// whether or not it's currently loaded into the active EntityPool. This is
// "cold" data: identity, lineage, lifecycle, and the progression state that
// needs to persist while unloaded (SkillSet, Reputation, AIState.schedule).
//
// While a person's home cell is loaded, a matching Entity exists in the
// active pool with fresh Transform/Movement/Health/Stamina/Combatant --
// active_id points at it and is_loaded is true. On unload, skill_set,
// reputation, and ai_state are copied back here and the Entity slot is
// freed; active_id becomes stale until the next load.
//
// Age is derived, not stored: age_ticks = current_world_tick - birth_tick.
// Death/role-assignment/etc are resolved lazily (on cell load, or
// immediately if the person happens to be loaded when a threshold is
// crossed) -- nothing here needs to be ticked while unloaded.
typedef struct
{
	uint32_t person_id;   // globally unique, persistent, never reused

	EntityType type;       // ENTITY_TYPE_NPC or ENTITY_TYPE_ANIMAL
	char name[PERSON_NAME_LEN];

	uint32_t birth_tick;
	uint32_t death_tick;     // 0 = still alive
	uint32_t lifespan_ticks; // rolled at birth, with variance

	uint32_t parent_a_id;    // 0 = no parent on record (founding generation)
	uint32_t parent_b_id;

	uint32_t settlement_id;  // 0 = unaffiliated/wild
	uint32_t role_id;        // 0 = unassigned (Child life stage)

	// Home location -- determines which WorldCell/region this record is
	// stored and streamed with.
	uint32_t home_cell_x, home_cell_y;
	uint16_t home_x, home_y, home_z;

	SkillSet skill_set;
	Reputation reputation;
	AIState ai_state;

	EntityID active_id; // valid only while is_loaded
	bool is_loaded;
} PersonRecord;

#endif // !PERSON_RECORD_H