#version 330 core

uniform vec3 diffColor;
//uniform mat4 ProjectionMatrix;

//smooth in vec4 Position;
smooth in vec2 TexCoord;
//smooth in vec3 pass_Normal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main(void){
    if(length(TexCoord - vec2(0.5)) > 0.5) discard;
    outColor  = diffColor;
	outNormal = vec3(0.0, 0.0, 1.0);
    //gl_FragDepth = 0.1;
}
