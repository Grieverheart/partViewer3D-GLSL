#version 330 core

uniform mat4 projection_matrix;

smooth in vec2 TexCoord;
flat in vec2 depth;

layout(location = 0) out vec3 outColor;

void main(void){
    vec3 pos = vec3(TexCoord - vec2(0.5), 0.0);
    if(length(pos) > 0.5) discard;

    pos.z = sqrt(0.25 - pos.x * pos.x - pos.y * pos.y);

    vec2 a = depth + 2.0 * vec2(projection_matrix[2][2] * pos.z, -pos.z);

    gl_FragDepth = 0.5 * (a.x / a.y) + 0.5;

    outColor = vec3(1.0, 1.0, 0.0);
}

