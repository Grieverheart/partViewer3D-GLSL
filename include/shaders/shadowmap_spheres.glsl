static const char shadowmap_spheres_frag[] = R"(
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
)";
static const char shadowmap_spheres_vert[] = R"(
#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;
uniform vec4 clip_plane;
uniform float radius;
uniform bool clip;
uniform mat4 ModelMatrix;

layout(location = 0) in vec3 in_Position;

smooth out vec2 TexCoord;
flat out vec3 sphere_position;
flat out vec4 view_clip_plane;
noperspective out vec3 clip_position;

vec2 TexCoords[] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

void main(void){

    TexCoord = radius * TexCoords[gl_VertexID];

	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    sphere_position = gl_Position.xyz / gl_Position.w;
    gl_Position += radius * vec4(TexCoords[gl_VertexID], vec2(0.0));
    vec3 view_position = gl_Position.xyz / gl_Position.w;
	gl_Position = ProjectionMatrix * gl_Position;

    if(clip){
        view_clip_plane = transpose(inverse(MVMatrix)) * clip_plane;
        float t = -(dot(view_position.xy, view_clip_plane.xy) + view_clip_plane.w) / view_clip_plane.z;
        clip_position = vec3(view_position.xy, t);
    }
}
)";
