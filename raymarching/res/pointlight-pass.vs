#version 400

struct PointLight
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
	float constant;
	float linear;
	float exp;
	vec3 pos;
	float radius;
};

layout(location = 0) in vec3 a_Pos;


uniform mat4 u_MVPMatrix;
layout(std140, binding = 1) uniform PointLightBlock
{
	PointLight pLight;
};

void main()
{
	gl_Position = u_MVPMatrix * vec4(pLight.radius * a_Pos + pLight.pos, 1.0);
}