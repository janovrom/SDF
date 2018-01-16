#version 420

uniform sampler2D u_DiffuseTex;
uniform vec3 u_LightPos;

in vec2 v_UV;
in vec3 v_Pos;

out float FragColor;

void main()
{
	vec4 c = texture(u_DiffuseTex, v_UV).bgra;
	if (c.a < 0.5)
		discard;

	FragColor = length(v_Pos - u_LightPos);
}