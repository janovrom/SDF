#version 440

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r32f, binding = 0) uniform image1D posIn;
layout(rg32f, binding = 1) uniform image1D uvIn;
layout(r32ui, binding = 2) uniform uimage1D idxIn;
layout(r32f, binding = 3) uniform image3D inout_grid;
//layout(rgba32f, binding = 3) uniform image2D tex;

layout(location = 0) uniform int u_baseIndex;
layout(location = 1) uniform vec3 u_gridOrigin;
layout(location = 2) uniform vec3 u_cellSize;

void main()
{
    // Get vertex index - multiply by 3 since we interpret is as only R texture
	int v0 = 3 * int(imageLoad(idxIn, int(gl_GlobalInvocationID.x)).x + u_baseIndex);
	int v1 = 3 * int(imageLoad(idxIn, int(gl_GlobalInvocationID.x + 1)).x + u_baseIndex);
	int v2 = 3 * int(imageLoad(idxIn, int(gl_GlobalInvocationID.x + 2)).x + u_baseIndex);
    // Get position of triangle
	vec3 pos0 = vec3(imageLoad(posIn, v0).x, imageLoad(posIn, v0 + 1).x, imageLoad(posIn, v0 + 2).x);
	vec3 pos1 = vec3(imageLoad(posIn, v1).x, imageLoad(posIn, v1 + 1).x, imageLoad(posIn, v1 + 2).x);
	vec3 pos2 = vec3(imageLoad(posIn, v2).x, imageLoad(posIn, v2 + 1).x, imageLoad(posIn, v2 + 2).x);
    
	vec3 minp = min(pos0, min(pos1, pos2));
	vec3 maxp = max(pos0, max(pos1, pos2));

}