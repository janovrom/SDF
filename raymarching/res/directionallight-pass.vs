#version 420

layout(location = 0) in vec2 a_Vertex;


void main(void)
{
	gl_Position = vec4(a_Vertex.x, a_Vertex.y, 0, 1.0);
}