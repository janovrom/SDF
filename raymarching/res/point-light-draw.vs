#version 400


layout(location = 0) in vec3 a_Pos;


uniform mat4 u_MVPMatrix;
uniform vec3 u_LightPos;

void main()
{
	gl_Position = u_MVPMatrix * vec4(1.25 * a_Pos + u_LightPos, 1.0);
}