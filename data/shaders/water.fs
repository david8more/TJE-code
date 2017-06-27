
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform sampler2D u_sky_texture;
uniform sampler2D u_normal_texture;
uniform vec3 u_camera_pos;
uniform float u_time;

void main()
{
	vec4 color = 0.5 * texture2D(u_texture, -v_world_position.xz * 0.001 + vec2(sin(u_time + 3.0) * 0.007, cos(u_time) * 0.007));
	color += 0.5 * texture2D(u_texture, v_world_position.xz * 0.001 + vec2(sin(u_time) * 0.007, cos(u_time) * 0.007));

	// coger textura
	vec4 normal_map = texture2D(u_normal_texture, v_uv + vec2(sin(u_time) * 0.01 + 500, cos(u_time) * 0.01 + 500));
	
	// pasar de 0 1 a -1 1
	normal_map = normal_map * 2;
	normal_map = normal_map - 1.0;

	// cambiar Y por Z

	vec3 normal_z_map = vec3(normal_map.x, normal_map.z, normal_map.y);

	vec3 N = (normal_z_map);
	//vec3 N = v_normal;
	vec3 E = normalize(v_world_position - u_camera_pos);
	vec3 R = -reflect(E, N);

	float yaw = 0.0;
	yaw = atan(R.x,R.z) / 6.28318531 - 0.25;
	float pitch = asin(R.y) / 1.57079633;
	vec2 uv_polar = vec2(yaw,abs(pitch));

	float fresnel = clamp(1.0 - pow(-dot(N, E), 0.4), -1.0, 1.0);

	vec4 sky_color = texture2DLod(u_sky_texture, uv_polar, 3.0);

	vec4 fog_color = vec4(156.0/256.0, 178.0/256.0, 186.0/256.0, 1.0);
	float distance = length(v_world_position - u_camera_pos);
	float factor = clamp(pow(distance / 10000.0, 0.5), 0.0, 1.0);

	color *= vec4(0.7, 0.7, 0.9, 1.0);

	color = mix(color, fog_color, factor);
	color = mix(color, sky_color, fresnel);

	gl_FragColor = color;
	//gl_FragColor = vec4(N, 1.0);
}
