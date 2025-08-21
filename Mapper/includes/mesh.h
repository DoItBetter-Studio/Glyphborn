#ifndef MESH_H
#define MESH_H

#include <maths/vec3.h>

typedef struct
{
	Vec3* vertices;   // Array of vertices
	Vec3* normals;
	Vec3* uvs;
	uint32_t* indices;

	uint32_t vertex_count;
	uint32_t index_count;

	uint8_t* material_ids;
	uint32_t material_count;

	const unsigned char* texture_data;
	int texture_width;
	int texture_height;

	Vec3 bounds_min;
	Vec3 bounds_max;
} RenderMesh;

#endif // !MESH_H
