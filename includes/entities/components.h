#ifndef ENTITY_COMPONENTS_H
#define ENTITY_COMPONENTS_H

#include "entity_id.h"
#include "maths/vec3.h"
#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------
// Transform - tile position + facing on the 3D tile grid
// ---------------------------------------------------------------------

typedef enum
{
	FACE_NORTH = 0,
	FACE_EAST,
	FACE_SOUTH,
	FACE_WEST,
} EntityFacing;

typedef struct
{
	uint16_t x, y, z;   // tile position in world space -- the entity's
	                    // *committed* tile. Only changes when a movement
	                    // step completes, never mid-transition. Glyphborn's
	                    // world matrix is entirely non-negative.
	EntityFacing facing;

	uint32_t world_cell_x;  // which WorldCell this entity is currently inside
	uint32_t world_cell_y;
} Transform;

// Converts a tile position to a Vec3 for distance/direction/angle math
// (awareness radius, attack range, etc). Logical equality checks
// (entity_at_tile, schedules, network sync) should keep comparing the raw
// uint16_t fields directly -- this is for the math library only.
static inline Vec3 transform_position(const Transform* t)
{
	Vec3 v = { (float)t->x, (float)t->y, (float)t->z };
	return v;
}

// ---------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------

typedef enum
{
	MOVE_IDLE,
	MOVE_WALKING,
	MOVE_SPRINTING,
	MOVE_DODGING,
	MOVE_KNOCKBACK,
} MoveState;

typedef struct
{
	MoveState state;
	float move_timer;          // seconds remaining until target tile is reached
	float move_duration;       // total seconds for the current step (move_timer starts here)
	uint16_t target_x, target_y, target_z;
	float move_speed;          // tiles per second
	bool is_grounded;

	// Continuous world-space position, lerped between the entity's
	// transform tile and target tile over move_duration. Purely visual --
	// camera, renderer, audio panning, etc. read this. Game logic never
	// should; it reads Transform.x/y/z instead.
	Vec3 render_position;
} Movement;

// ---------------------------------------------------------------------
// Health
// ---------------------------------------------------------------------

#define INJURY_NONE         0x00
#define INJURY_BLEEDING     0x01
#define INJURY_BROKEN_BONE  0x02
#define INJURY_FATIGUED     0x04
#define INJURY_INFECTED     0x08

typedef struct
{
	float current;
	float max;
	uint8_t injury_flags;
	float bleed_timer;     // seconds until next bleed tick
} Health;

// ---------------------------------------------------------------------
// Stamina
// ---------------------------------------------------------------------

typedef struct
{
	float current;
	float max;
	float regen_rate;      // per second when not exerting
	bool is_recovering;    // brief lockout after hitting zero
} Stamina;

// ---------------------------------------------------------------------
// Combatant - real-time tile combat state (GDD 5.4)
// ---------------------------------------------------------------------

typedef enum
{
	WEAPON_NONE,
	WEAPON_SWORD,
	WEAPON_AXE,
	WEAPON_SPEAR,
	WEAPON_DAGGER,
	WEAPON_HAMMER,
	WEAPON_BOW,
	WEAPON_JAVELIN,
	WEAPON_THROWING_AXE,
	WEAPON_SLING,
} WeaponType;

typedef enum
{
	COMBAT_IDLE,
	COMBAT_ATTACKING,
	COMBAT_BLOCKING,
	COMBAT_DODGING,
	COMBAT_STAGGERED,
} CombatState;

typedef struct
{
	CombatState state;
	WeaponType weapon;
	WeaponType offhand_weapon;  // WEAPON_NONE unless dual-wield is unlocked
	bool has_shield;

	float attack_timer;     // seconds remaining in current attack animation
	float dodge_cooldown;   // seconds until next dodge is available
	float block_stamina;    // stamina reserved for the current block
} Combatant;

// ---------------------------------------------------------------------
// AI state - empty/unused fields for player-controlled entities
// ---------------------------------------------------------------------

typedef enum
{
	AI_IDLE,
	AI_PATROL,
	AI_ALERT,
	AI_COMBAT,
	AI_FLEE,
	AI_SCHEDULE,    // following ai_state.schedule
	AI_DEAD,
} AIBehavior;

#define MAX_SCHEDULE_ENTRIES 8

typedef struct
{
	uint8_t hour_start;
	uint8_t hour_end;
	uint16_t location_x, location_y, location_z;
	uint16_t activity_id;   // indexes a static activity table (smithing, sleeping, trading...)
} ScheduleEntry;

typedef struct
{
	AIBehavior behavior;
	EntityID target;
	float awareness_radius;     // tiles
	float think_timer;          // seconds until next think tick

	uint16_t patrol_origin_x, patrol_origin_z;

	ScheduleEntry schedule[MAX_SCHEDULE_ENTRIES];
	uint8_t schedule_count;     // 0 for animals, ships, projectiles, the player
} AIState;

// ---------------------------------------------------------------------
// Reputation - empty/zeroed for entities that don't participate socially
// ---------------------------------------------------------------------

#define MAX_FACTIONS 16

typedef struct
{
	uint16_t clan_id;                  // 0 = clanless
	int16_t faction_rep[MAX_FACTIONS]; // -1000 .. 1000
	bool is_outlaw;
	uint32_t bounty;                   // silver units
} Reputation;

#endif // !ENTITY_COMPONENTS_H