#version 400 core

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;

uniform sampler2D u_Tex;

in vec3 v_Normal;
in vec3 v_WorldPos;
in vec2 v_UV;

void main() {
	/*float zc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).z;
	float wc = (u_MVPMatrix * vec4(v_WorldPos, 1.0)).w;
	float depthP = zc / wc;
	gl_FragDepth = depthP;*/
	WorldPosOut = v_WorldPos;
	DiffuseOut = texture(u_Tex, v_UV).rgb;
	NormalOut = normalize(v_Normal);
}
