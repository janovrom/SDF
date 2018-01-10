#version 420 core

layout(location = 0) in vec3 a_Vertex;
layout (location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;
uniform mat4  u_Model;
uniform mat4  u_Normal;


out vec3 v_Normal;
out vec3 v_WorldPos;
out vec2 v_UV;

void main(void) {
	gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * u_Model * vec4(a_Vertex, 1.0);
	v_Normal	= (u_Normal * vec4(a_Normal, 0.0)).xyz;
	vec3 t		= (u_Normal * vec4(a_Tangent, 0.0)).xyz;
	vec3 b		= cross(v_Normal, t);
	v_WorldPos	= (u_Model * vec4(a_Vertex, 1.0)).xyz;
	v_UV		= a_TexCoord;
}