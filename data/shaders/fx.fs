
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform float u_time;

void main()
{
	vec4 day = 	vec4(1.0, 1.0, 1.0, 1.0);
	vec4 dusk = vec4(0.8, 0.5, 0.2, 1.0);
	vec4 night = vec4(0.3, 0.4, 0.7, 1.0);

	vec4 text = texture2D(u_texture, v_uv);

	vec4 color = text;
	
	float time_t = sin(u_time * 0.05) * 0.5 + 0.5;

	color *= mix(day, night, 1 - time_t);

	//color *= mix(day, dusk, 1.5 * time_t);

	//color = mix(color, text*night,  time_t);


	gl_FragColor = color;
}
