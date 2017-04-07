
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;

void main()
{
	vec4 color = texture2D(u_texture, v_uv);
	vec3 light = vec3(0.2, 0.3, 0.5);
	light += max(0.0, v_normal.y) * vec3(0.4, 0.3, 0.0);
	color.xyz *= light;
	gl_FragColor = color;
}
