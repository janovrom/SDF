#version 420

uniform sampler2D u_DiffuseTex;

in vec2 v_UV;

out float FragColor;

void main()
{
	vec4 c = texture(u_DiffuseTex, v_UV).bgra;
	if (c.a < 0.5)
		discard;

	FragColor = gl_FragDepth;
}