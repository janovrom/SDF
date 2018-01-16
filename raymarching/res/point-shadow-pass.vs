#version 420 core

layout(location = 0) in vec3 a_Vertex;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

uniform mat4  u_Model;
uniform mat4  u_LightProjection;
uniform mat4  u_LightView;

out vec2 v_UV;
out vec3 v_Pos;


void main(void)
{
	
	vec4 pos = u_Model * vec4(a_Vertex, 1.0);
	v_Pos = pos.xyz;
	gl_Position = u_LightProjection * u_LightView * pos;
	v_UV = a_TexCoord;
}