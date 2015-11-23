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

        tex = textureOffset(aoSampler, TexCoord, offset[6]);
        weight = step(0.9, dot(tex.gba, normal));
        result += tex.r * weight;
        weights += weight;

        out_AO = vec4(vec3(0.0), result / weights);
    }
    else out_AO = vec4(vec3(0.0), texture(aoSampler, TexCoord).r);
}
