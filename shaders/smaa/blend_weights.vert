#version 330 core

#define SMAA_PRESET_MEDIUM

#if defined(SMAA_PRESET_LOW)
#define SMAA_MAX_SEARCH_STEPS 4
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_MAX_SEARCH_STEPS 8
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_MAX_SEARCH_STEPS 16
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_MAX_SEARCH_STEPS 32
#endif

#define SMAA_PIXEL_SIZE vec2(1.0 / 600.0, 1.0 / 600.0)

noperspective out vec2 texcoord;
noperspective out vec2 pixcoord;
noperspective out vec4 offset[3];

void main(void){
	texcoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	texcoord.y = (gl_VertexID == 1)? 2.0: 0.0;

    pixcoord = texcoord / SMAA_PIXEL_SIZE;

    offset[0] = texcoord.xyxy + SMAA_PIXEL_SIZE.xyxy * vec4(-0.25, -0.125, 1.25, -0.125);
    offset[1] = texcoord.xyxy + SMAA_PIXEL_SIZE.xyxy * vec4(-0.125, -0.25, -0.125, 1.25);

    offset[2] = vec4(offset[0].xz, offset[1].yw) + vec4(-2.0, 2.0, -2.0, 2.0) * SMAA_PIXEL_SIZE.xxyy * float(SMAA_MAX_SEARCH_STEPS);

    vec4 position = vec4(2.0 * texcoord - 1.0, 0.0, 1.0);

	gl_Position = position;
}
