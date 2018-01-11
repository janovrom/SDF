#version 440

in vec3 v_WorldPos;

uniform vec3 u_pLightPosition;

out float FragColor;

void main()
{
	FragColor = length(u_pLightPosition - v_WorldPos);
}