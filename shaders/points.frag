#version 330 core

uniform mat4 ProjectionMatrix;
uniform float radius;

smooth in vec2 TexCoord;
flat in vec3 sphere_position;

layout(location = 0) out vec3 outColor;

void main(void){
    vec3 pos = vec3(TexCoord, 0.0);
    float dotp = dot(pos, pos);
    float r2 = radius * radius;
    if(dotp > r2) discard;

    //TODO: Make these colors, and thickness, parameters
    outColor = (dotp > r2 * 0.7)? vec3(0.0): vec3(0.05);

    pos += sphere_position;

    mat2 proj = mat2(ProjectionMatrix[2][2], ProjectionMatrix[2][3],
                     ProjectionMatrix[3][2], ProjectionMatrix[3][3]);

    vec2 depth = proj * vec2(pos.z, 1.0);
    gl_FragDepth = 0.5 * (depth.x / depth.y) + 0.5;
}