#version 440

layout(location = 0) in vec3 a_Vertex;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;
uniform mat4  u_Model;

out vec3 v_WorldPos;


void main(void)
{
	gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * u_Model * vec4(a_Vertex, 1.0);
	v_WorldPos = (u_Model * vec4(a_Vertex, 1.0)).xyz;
}