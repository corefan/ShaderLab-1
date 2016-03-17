static const char* sprite_plain_frag = STRINGIFY(

\n#version 100\n
precision highp float;

varying vec2 v_texcoord; 

uniform sampler2D texture0; 

void main()
{  
	gl_FragColor = texture2D(texture0, v_texcoord);
}

);
