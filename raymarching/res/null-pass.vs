#version 400

layout(location = 0) in vec3 a_Pos;


uniform mat4 u_MVPMatrix;

void main()
{
	gl_Position = u_MVPMatrix * vec4(a_Pos, 1.0);
}