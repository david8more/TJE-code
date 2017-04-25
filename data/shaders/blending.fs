varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;

void main()
{
	vec4 color = texture2D(u_texture, v_uv);

	if(color.a < 0.3)
		discard;

	gl_FragColor = color;
}
