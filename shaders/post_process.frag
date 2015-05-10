#version 330 core

uniform sampler2D thisSampler;

noperspective in vec2 TexCoord;

layout(location = 0) out vec4 out_color;

void main(void){
    out_color = texture(thisSampler, TexCoord);
}
