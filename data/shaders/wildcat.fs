varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;

void main()
{
	vec4 color = texture2D(u_texture, v_uv);
	vec3 light = vec3(0.5, 0.8, 0.9);
	light += max(0.0, v_normal.y) * vec3(0.84, 0.45, 0.21);
	color.xyz *= light;

	color.xyz *= vec3(0.7, 0.7, 0.75);
	color.xyz *= light * 0.85;

	gl_FragColor = color;
}
