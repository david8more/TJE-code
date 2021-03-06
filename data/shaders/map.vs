﻿attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform vec3 u_camera_pos;

uniform mat4 u_model;
uniform mat4 u_mvp;

//this will store the color for the pixel shader
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

void main()
{	
	//calcule the normal in camera space (the NormalMatrix is like ViewMatrix but without traslation)
	v_normal = (u_model * vec4( a_normal, 0.0) ).xyz;
	
	//calcule the vertex in object space
	v_position = a_vertex;
	v_world_position = (u_model * vec4( a_vertex, 1.0) ).xyz;
	
	//store the color in the varying var to use it from the pixel shader
	v_color = a_color;

	//store the texture coordinates
	v_uv = a_uv;

	//calcule the position of the vertex using the matrices
	gl_Position = u_mvp * vec4( a_vertex, 1.0 );
	
	/*float pSize = 7.0;
	v_color = vec4(1, 0.75, 0, 1);
	
	if( gl_Position.z > 10000)
	{
		pSize = 8.5;
		v_color = vec4(1, 0, 0, 0.5);
	}
	
	gl_PointSize = pSize;*/
	
}