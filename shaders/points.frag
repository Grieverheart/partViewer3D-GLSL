#version 330 core

uniform mat4 ProjectionMatrix;

uniform float radius;
uniform float outline_radius;
uniform vec4 color;
uniform vec4 outline_color;

smooth in vec2 TexCoord;
flat in vec3 sphere_position;

layout(location = 0) out vec4 outColor;

void main(void){
    vec3 pos = vec3(TexCoord, 0.0);
    float dotp = dot(pos, pos);
    float r2 = radius * radius;
    if(dotp > r2) discard;

    outColor = (dotp > (r2 - outline_radius * outline_radius))? outline_color: color;

    pos += sphere_position;

    mat2 proj = mat2(ProjectionMatrix[2][2], ProjectionMatrix[2][3],
                     ProjectionMatrix[3][2], ProjectionMatrix[3][3]);

    vec2 depth = proj * vec2(pos.z, 1.0);
    gl_FragDepth = 0.5 * (depth.x / depth.y) + 0.5;
}
