#version 400 core

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

in vec3 v_Normal;
in vec3 v_WorldPos;
in vec4 v_Color;

void main() {
	WorldPosOut = v_WorldPos;
	DiffuseOut = v_Color.xyz;
	NormalOut = normalize(v_Normal);
}
