#version 400 core

layout(location = 0) out vec4 WorldPosOut;
layout(location = 1) out vec4 DiffuseOut;
layout(location = 2) out vec4 NormalOut;

uniform sampler2D u_DiffuseTex;
uniform sampler2D u_HeightTex;
uniform sampler2D u_SpecularTex;
uniform mat4 u_MVPMatrix;

in vec3 v_Normal;
in vec3 v_WorldPos;
in vec2 v_UV;

void main() {
	//float zc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).z;
	//float wc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).w;
	//float depthP = zc / wc;
	//gl_FragDepth = 0.5 * depthP + 0.5;
	DiffuseOut = texture(u_DiffuseTex, v_UV).bgra;
	if (DiffuseOut.a < 0.5)
		discard;
	else
		DiffuseOut.a = 1.0;
	WorldPosOut = vec4(v_WorldPos, 1.0);
	NormalOut = vec4(normalize(v_Normal), 1.0);
}
