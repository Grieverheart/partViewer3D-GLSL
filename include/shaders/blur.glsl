static const char blur_frag[] = R"(
#version 330 core

uniform sampler2D aoSampler;
uniform bool use_blur;

noperspective in vec2 TexCoord;

layout(location = 1) out vec4 out_AO;

const ivec2 offset[8] = ivec2[](
    ivec2(-1, -1), ivec2( 0, -1), ivec2( 1, -1),
    ivec2(-1,  0), ivec2( 1,  0),
    ivec2(-1,  1), ivec2( 0,  1), ivec2( 1,  1)
);

void main(void){

    if(use_blur){
        vec4 tex = texture(aoSampler, TexCoord);
        vec3 normal = tex.gba;
        float result = tex.r;
        float weights = 1.0;

        tex = textureOffset(aoSampler, TexCoord, offset[0]);
        float weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[1]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[2]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[3]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[4]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[5]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[6]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        tex = textureOffset(aoSampler, TexCoord, offset[7]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        out_AO = vec4(vec3(0.0), result / weights);
    }
    else out_AO = vec4(vec3(0.0), texture(aoSampler, TexCoord).r);
}
)";
static const char blur_vert[] = R"(
#version 330 core

noperspective out vec2 TexCoord;

void main(void){
	TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;
    vec4 position = vec4(TexCoord * 2.0 - 1.0, 0.0, 1.0);
	
	gl_Position = position;
}
)";
