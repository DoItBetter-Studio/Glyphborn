#ifndef ENTITY_H
#define ENTITY_H

#include "entity_id.h"
#include "components.h"
#include "skill_set.h"
#include <stdbool.h>

// Every entity in Glyphborn -- the player, Bob Blacksmith, a bear, a ship,
// a dropped axe -- is one of these. There is no per-type struct and no
// component bitmask: the fields are always present, and an entity that
// doesn't use a field just leaves it zeroed (a ship's SkillSet.count is 0,
// an animal's Reputation is all zero, etc).
//
// What makes "Bob Blacksmith" different from "John Carpenter" is purely
// the values in this struct -- their skill_set levels, transform position,
// and ai_state.schedule -- not their type or shape. Both are
// ENTITY_TYPE_NPC.
typedef struct
{
	EntityType type;
	bool alive;
	const char* name;   // points into a localization/name volume, not owned
 
	Transform transform;
	Movement movement;
	Health health;
	Stamina stamina;
	Combatant combatant;
	AIState ai_state;
	Reputation reputation;
	SkillSet skill_set;     // empty (count == 0) for ships, projectiles, item drops
} Entity;

#endif // ENTITY_H