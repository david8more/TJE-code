
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform sampler2D u_sky_texture;
uniform vec3 u_camera_pos;
uniform float u_time;

void main()
{
	vec4 color = texture2D(u_texture, v_uv);

	vec4 fog_color = vec4(176.0/256.0, 178.0/256.0, 196.0/256.0, 1.0);
	float distance = length(v_world_position - u_camera_pos);

	float factor = clamp(pow(distance / 10000.0, 0.2), 0.0, 1.0);

	color.xyz = mix(color.xyz, fog_color.xyz, factor);

	gl_FragColor = color;
}
