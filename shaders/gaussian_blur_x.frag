#version 330 core

uniform sampler2D image;
noperspective in vec2 TexCoord;

layout(location = 0) out vec4 blurred_image;

float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main(void){
    blurred_image = texture(image, TexCoord) * weight[0];
    for(int i = 1; i < 3; ++i){
        blurred_image += texture(image, TexCoord + vec2(0.0, offset[i]) / 600.0) * weight[i];
        blurred_image += texture(image, TexCoord - vec2(0.0, offset[i]) / 600.0) * weight[i];
    }
}
