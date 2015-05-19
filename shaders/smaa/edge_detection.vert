#version 330 core

uniform vec2 texel_size;

noperspective out vec2 texcoord;
noperspective out vec4 offset[3];

void main(void){
	texcoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	texcoord.y = (gl_VertexID == 1)? 2.0: 0.0;

    offset[0] = texcoord.xyxy + texel_size.xyxy * vec4(-1.0, 0.0, 0.0, -1.0);
    offset[1] = texcoord.xyxy + texel_size.xyxy * vec4( 1.0, 0.0, 0.0,  1.0);
    offset[2] = texcoord.xyxy + texel_size.xyxy * vec4(-2.0, 0.0, 0.0, -2.0);

    vec4 position = vec4(2.0 * texcoord - 1.0, 0.0, 1.0);

	gl_Position = position;
}
