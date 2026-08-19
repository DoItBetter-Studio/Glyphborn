#include "entities/skill_registry.h"
#include <math.h>
#include <stddef.h>

const SkillDef* skill_registry_find_skill(const SkillRegistry* reg, uint32_t skill_id)
{
	for (uint32_t i = 0; i < reg->skill_count; i++)
	{
		if (reg->skill_defs[i].id == skill_id)
			return &reg->skill_defs[i];
	}

	return NULL;
}

const TechniqueDef* skill_registry_find_technique(const SkillRegistry* reg, uint32_t technique_id)
{
	for (uint32_t i = 0; i < reg->technique_count; i++)
	{
		if (reg->technique_defs[i].id == technique_id)
			return &reg->technique_defs[i];
	}

	return NULL;
}

float skill_xp_required(XPCurveType curve, float param_a, float param_b, uint16_t level)
{
	switch (curve)
	{
		case XP_CURVE_LINEAR:
			return param_a * (float)level;

		case XP_CURVE_EXPONENTIAL:
			return param_a * powf(param_b, (float)level);

		case XP_CURVE_LOG_EARLY_STEEP_LATE:
			return param_a * powf((float)level, param_b);

		default:
			return param_a * (float)level;
	}
}