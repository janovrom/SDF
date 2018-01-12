#version 420 core

layout (location = 0) in vec2 a_Vertex;

uniform mat4 u_ModelViewMatrixInverse;
uniform mat4 u_MVPMatrix;


out vec2 v_Vertex;
out vec3 v_Normal_worldspace;
out vec3 v_Pos_worldspace;


void main(void) {
    v_Vertex    = a_Vertex.xy;
	vec4 end = (inverse(u_MVPMatrix) * vec4(v_Vertex.x, v_Vertex.y, 1.0, 1.0));
	vec4 start = (inverse(u_MVPMatrix) * vec4(v_Vertex.x, v_Vertex.y, -1.0, 1.0));
	end /= end.w;
	start /= start.w;
	v_Pos_worldspace = start.xyz;
	v_Normal_worldspace = normalize(end.xyz - v_Pos_worldspace);
	gl_Position = vec4(v_Vertex.x, v_Vertex.y, -1.0, 1.0);
}