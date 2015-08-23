#version 330 core

uniform mat4 ProjectionMatrix;
uniform float radius;
uniform bool clip;

smooth in vec2 TexCoord;
flat in vec3 sphere_position;
flat in vec4 view_clip_plane;
noperspective in vec3 clip_position;

void main(void){
    vec3 pos = vec3(TexCoord, 0.0);
    if(clip && dot(view_clip_plane, vec4(pos + sphere_position, 1.0)) < 0.0) pos = clip_position - sphere_position;
    if(dot(pos, pos) > radius * radius) discard;

    pos += sphere_position;

    mat2 proj = mat2(ProjectionMatrix[2][2], ProjectionMatrix[2][3],
                     ProjectionMatrix[3][2], ProjectionMatrix[3][3]);

    vec2 depth = proj * vec2(pos.z, 1.0);
    gl_FragDepth = 0.5 * (depth.x / depth.y) + 0.5;
}
