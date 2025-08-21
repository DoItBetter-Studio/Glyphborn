#include "test_tile.h"

Vec3 cube_vertices[8] = {
	{ 0, 0, 0 }, { 1, 0, 0 },
	{ 1, 1, 0 }, { 0, 1, 0 },
	{ 0, 0, 1 }, { 1, 0, 1 },
	{ 1, 1, 1 }, { 0, 1, 1 }
};

uint32_t cube_indices[36] = {
	// Front face
	0, 1, 2,  2, 3, 0,
	// Back face
	4, 5, 6,  6, 7, 4,
	// Left face
	0, 3, 7,  7, 4, 0,
	// Right face
	1, 5, 6,  6, 2, 1,
	// Top face
	3, 2, 6,  6, 7, 3,
	// Bottom face
	0, 1, 5,  5, 4, 0
};