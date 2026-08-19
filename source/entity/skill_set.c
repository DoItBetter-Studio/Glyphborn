#include "entities/skill_set.h"
#include "entities/skill_registry.h"
#include "game/difficulty.h"
#include <stddef.h>

SkillState* skill_set_find(SkillSet* set, uint32_t skill_id)
{
	for (int32_t i = 0; i < set->count; i++)
	{
		if (set->skills[i].id == skill_id)
			return &set->skills[i];
	}

	return NULL;
}

SkillState* skill_set_add(SkillSet* set, uint32_t skill_id, uint16_t starting_level)
{
	if (set->count >= MAX_ENTITY_SKILLS)
		return NULL;

	SkillState* slot = &set->skills[set->count++];
	slot->id = skill_id;
	slot->level = starting_level;
	slot->xp = 0.0f;
	slot->unlocked = true;

	return slot;
}

// A skill or technique id might refer to either table -- techniques use the
// same SkillState progression shape, so look in both.
static bool find_def_curve(const SkillRegistry* reg, uint32_t id,
                            float* out_success_xp, float* out_failure_xp,
                            XPCurveType* out_curve, float* out_a, float* out_b)
{
	const SkillDef* skill = skill_registry_find_skill(reg, id);
	if (skill)
	{
		*out_success_xp = skill->success_xp;
		*out_failure_xp = skill->failure_xp;
		*out_curve = skill->curve_type;
		*out_a = skill->curve_param_a;
		*out_b = skill->curve_param_b;
		return true;
	}

	const TechniqueDef* tech = skill_registry_find_technique(reg, id);
	if (tech)
	{
		*out_success_xp = tech->success_xp;
		*out_failure_xp = tech->failure_xp;
		*out_curve = tech->curve_type;
		*out_a = tech->curve_param_a;
		*out_b = tech->curve_param_b;
		return true;
	}

	return false;
}

void skill_award_xp(SkillSet* set, const SkillRegistry* reg, uint32_t skill_id, XPOutcome outcome)
{
	float success_xp, failure_xp, curve_a, curve_b;
	XPCurveType curve;

	if (!find_def_curve(reg, skill_id, &success_xp, &failure_xp, &curve, &curve_a, &curve_b))
		return; // unknown id -- nothing to award

	SkillState* state = skill_set_find(set, skill_id);
	if (!state)
		state = skill_set_add(set, skill_id, 0);
	if (!state)
		return; // set is full

	float gained;
	switch (outcome)
	{
		case XP_OUTCOME_SUCCESS:  gained = success_xp; break;
		case XP_OUTCOME_CRITICAL: gained = success_xp * 1.5f; break;
		case XP_OUTCOME_FAILURE:
		default:                  gained = failure_xp; break;
	}

	gained *= g_difficulty_settings.xp_gain_mult;
	state->xp += gained;

	bool leveled_up = false;
	while (state->level < MAX_SKILL_LEVEL)
	{
		float required = skill_xp_required(curve, curve_a, curve_b, state->level + 1);
		if (state->xp < required)
			break;

		state->xp -= required;
		state->level++;
		leveled_up = true;
	}

	if (state->level >= MAX_SKILL_LEVEL)
		state->xp = 0.0f; // no point accumulating xp the entity can never spend

	if (leveled_up)
		skill_set_check_synthesis(set, reg);
}

void skill_set_check_synthesis(SkillSet* set, const SkillRegistry* reg)
{
	int32_t tolerance = g_difficulty_settings.skill_window_tolerance;

	for (uint32_t i = 0; i < reg->synthesis_count; i++)
	{
		const SynthesisRule* rule = &reg->synthesis_rules[i];

		if (skill_set_find(set, rule->result_skill_id))
			continue; // already unlocked

		SkillState* a = skill_set_find(set, rule->req_skill_a_id);
		SkillState* b = skill_set_find(set, rule->req_skill_b_id);
		if (!a || !b)
			continue;

		bool a_in_window = a->level >= rule->req_skill_a_level &&
		                    a->level <= rule->req_skill_a_level + tolerance;
		bool b_in_window = b->level >= rule->req_skill_b_level &&
		                    b->level <= rule->req_skill_b_level + tolerance;

		if (a_in_window && b_in_window)
			skill_set_add(set, rule->result_skill_id, 0);
	}
}