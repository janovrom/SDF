#version 400 core

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;
uniform float u_Far;
uniform float u_Near;
uniform mat4 u_MVPMatrix;

in vec3 v_Normal;
in vec3 v_WorldPos;
in vec4 v_Color;
in vec4 v_ViewPos;

void main() {
	//float zc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).z;
	//float wc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).w;
	//float depthP = zc / wc;
	//gl_FragDepth = depthP;
	WorldPosOut = v_WorldPos;
	DiffuseOut = v_Color.xyz;
	NormalOut = normalize(v_Normal);
}
