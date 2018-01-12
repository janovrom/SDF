#version 420 core

layout(location = 0) in vec3 a_Vertex;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

uniform mat4  u_LightView;
uniform mat4  u_LightProjection;
uniform mat4  u_Model;
uniform mat4  u_Normal;

out vec2 v_UV;

void main(void)
{
	gl_Position = u_LightProjection * u_LightView * u_Model * vec4(a_Vertex, 1.0);
	v_UV = a_TexCoord;
}