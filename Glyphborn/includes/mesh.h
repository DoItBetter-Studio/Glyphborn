#include <maths/vec3.h>
#ifndef MESH_H
#define MESH_H

typedef struct
{
	Vec3* vertices;   // Array of vertices
	int vertex_count; // Number of vertices
	int (*edges)[2]; // Array of edges, each edge is a pair of vertex indices
	int edge_count;   // Number of edges
} RenderMesh;

#endif // !MESH_H
