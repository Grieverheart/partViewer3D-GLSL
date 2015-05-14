#version 330 core

#define SMAA_PIXEL_SIZE vec2(1.0 / 600.0, 1.0 / 600.0)

noperspective out vec2 texcoord;
noperspective out vec4 offset[2];

void main(void){
	texcoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	texcoord.y = (gl_VertexID == 1)? 2.0: 0.0;

    offset[0] = texcoord.xyxy + SMAA_PIXEL_SIZE.xyxy * vec4(-1.0, 0.0, 0.0, -1.0);
    offset[1] = texcoord.xyxy + SMAA_PIXEL_SIZE.xyxy * vec4( 1.0, 0.0, 0.0,  1.0);

    vec4 position = vec4(2.0 * texcoord - 1.0, 0.0, 1.0);

	gl_Position = position;
}
