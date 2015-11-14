#version 330 core

uniform sampler2D inSampler;
uniform vec4 inColor;
smooth in vec2 TexCoord;

layout(location = 0) out vec4 out_Color;

void main(void){
    out_Color = vec4(1.0, 1.0, 1.0, texture(inSampler, TexCoord).r) * inColor;
}
