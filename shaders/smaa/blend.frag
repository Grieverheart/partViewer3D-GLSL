#version 330 core

uniform vec2 texel_size;

uniform sampler2D colorTex;
uniform sampler2D blendTex;

noperspective in vec2 texcoord;
noperspective in vec4 offsets;

layout(location = 0) out vec3 outColor;

void main(void){
    // Fetch the blending weights for current pixel:
    vec4 a;
    a.xz = texture(blendTex, texcoord).xz;
    a.y  = texture(blendTex, offsets.zw).g;
    a.w  = texture(blendTex, offsets.xy).a;

    // Is there any blending weight with a value greater than 0.0?
    if (dot(a, vec4(1.0)) < 1.0e-5){
        outColor = texture(colorTex, texcoord).rgb;
    }
    else{
        // Up to 4 lines can be crossing a pixel (one through each edge). We
        // favor blending by choosing the line with the maximum weight for each
        // direction:
        vec2 offset = vec2(
            (a.a > a.b? a.a : -a.b),// left vs. right 
            (a.g > a.r? a.g : -a.r) // top vs. bottom
        );

        // Then we go in the direction that has the maximum weight:
        if (abs(offset.x) > abs(offset.y)) // horizontal vs. vertical
            offset.y = 0.0;
        else
            offset.x = 0.0;

        // We exploit bilinear filtering to mix current pixel with the chosen
        // neighbor:
        vec2 blendcoord = texcoord + offset * texel_size;
        outColor = texture(colorTex, blendcoord).rgb;
    }
}
