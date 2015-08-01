#version 330 core

uniform mat4 ProjectionMatrix;

smooth in vec2 TexCoord;
flat in vec2 depth;
flat in vec3 pass_Color;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

//NOTE: In principle, we have to fully raytrace the sphere because
//the result is not correct in perspective projection. Or is it?

void main(void){
    vec3 pos = vec3(TexCoord - vec2(0.5), 0.0);
    //TODO: Change to square
    if(length(pos) > 0.5) discard;

    pos.z = sqrt(0.25 - pos.x * pos.x - pos.y * pos.y);

    outColor  = pass_Color;
	outNormal = normalize(pos);

    //NOTE: This works for perspective projection. You will have to change
    //the -pos.z -> ProjectionMatrix[2][3] * pos.z
    vec2 a = depth + 2.0 * vec2(ProjectionMatrix[2][2] * pos.z, -pos.z);

    gl_FragDepth = 0.5 * (a.x / a.y) + 0.5;
}
