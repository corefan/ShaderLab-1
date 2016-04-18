static const char* model3_texture_vert = STRINGIFY(

attribute vec4 position;
attribute vec2 texcoord;

varying vec2 v_texcoord;

uniform mat4 u_projection;
uniform mat4 u_modelview;
	
void main(void)
{
	gl_Position = u_projection * u_modelview * position;
	v_texcoord = texcoord;
}

);