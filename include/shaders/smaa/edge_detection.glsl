static const char edge_detection_frag[] = R"(
#version 330 core

#define SMAA_PRESET_ULTRA

#if defined(SMAA_PRESET_LOW)
#define SMAA_THRESHOLD 0.15
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_THRESHOLD 0.1
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_THRESHOLD 0.1
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_THRESHOLD 0.05
#endif

uniform sampler2D colorTex;

noperspective in vec2 texcoord;
noperspective in vec4 offset[3];

layout(location = 0) out vec3 outColor;

void main(void){
    // Calculate color deltas:
    vec4 delta;
    vec3 C = texture(colorTex, texcoord).rgb;

    vec3 Cleft = texture(colorTex, offset[0].xy).rgb;
    vec3 t = abs(C - Cleft);
    delta.x = max(max(t.r, t.g), t.b);

    vec3 Ctop  = texture(colorTex, offset[0].zw).rgb;
    t = abs(C - Ctop);
    delta.y = max(max(t.r, t.g), t.b);

    // We do the usual threshold:
    vec2 edges = step(vec2(SMAA_THRESHOLD, SMAA_THRESHOLD), delta.xy);

    // Then discard if there is no edge:
    if (dot(edges, vec2(1.0)) == 0.0) discard;

    // Calculate right and bottom deltas:
    vec3 Cright = texture(colorTex, offset[1].xy).rgb;
    t = abs(C - Cright);
    delta.z = max(max(t.r, t.g), t.b);

    vec3 Cbottom  = texture(colorTex, offset[1].zw).rgb;
    t = abs(C - Cbottom);
    delta.w = max(max(t.r, t.g), t.b);

    // Calculate the maximum delta in the direct neighborhood:
    float maxDelta = max(max(max(delta.x, delta.y), delta.z), delta.w);

    // Calculate left-left and top-top deltas:
    vec3 Cleftleft  = texture(colorTex, offset[2].xy).rgb;
    t = abs(C - Cleftleft);
    delta.z = max(max(t.r, t.g), t.b);

    vec3 Ctoptop = texture(colorTex, offset[2].zw).rgb;
    t = abs(C - Ctoptop);
    delta.w = max(max(t.r, t.g), t.b);

    // Calculate the final maximum delta:
    maxDelta = max(max(maxDelta, delta.z), delta.w);

    // Local contrast adaptation in action:
    edges.xy *= step(0.5 * maxDelta, delta.xy);

    outColor = vec3(edges, 0.0);
}
)";
static const char edge_detection_vert[] = R"(
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
)";
