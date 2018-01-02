#version 400 core

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;
layout(location = 3) out vec3 DepthOut;

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;
uniform float u_Far;
uniform float u_Near;

in vec3 v_Normal;
in vec3 v_WorldPos;
in vec4 v_Color;
in vec4 v_ViewPos;

void main() {
	WorldPosOut = v_WorldPos;
	DiffuseOut = v_Color.xyz;
	NormalOut = normalize(v_Normal);
	DepthOut = vec3(length(v_ViewPos.xyz) - abs(u_Near/normalize(v_ViewPos).z));
}
