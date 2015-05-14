#version 330 core

#define SMAA_PIXEL_SIZE vec2(1.0 / 600.0, 1.0 / 600.0)

uniform sampler2D colorTex;
uniform sampler2D blendTex;

noperspective in vec2 texcoord;
noperspective in vec4 offset[2];

layout(location = 0) out vec3 outColor;

void main(void){
    // Fetch the blending weights for current pixel:
    vec4 a;
    a.xz = texture(blendTex, texcoord).xz;
    a.y = texture(blendTex, offset[1].zw).g;
    a.w = texture(blendTex, offset[1].xy).a;

    // Is there any blending weight with a value greater than 0.0?
    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1.0e-5){
        outColor = texture(colorTex, texcoord).rgb;
    }
    else{
        vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

        // Up to 4 lines can be crossing a pixel (one through each edge). We
        // favor blending by choosing the line with the maximum weight for each
        // direction:
        vec2 offset;
        offset.x = a.a > a.b? a.a : -a.b; // left vs. right 
        offset.y = a.g > a.r? a.g : -a.r; // top vs. bottom

        // Then we go in the direction that has the maximum weight:
        if (abs(offset.x) > abs(offset.y)) // horizontal vs. vertical
            offset.y = 0.0;
        else
            offset.x = 0.0;

        // We exploit bilinear filtering to mix current pixel with the chosen
        // neighbor:
        vec2 blendcoord = texcoord + offset * SMAA_PIXEL_SIZE;
        outColor = texture(colorTex, blendcoord).rgb;
    }
}
