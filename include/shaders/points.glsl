static const char points_frag[] = R"(
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
    if(dotp > radius * radius) discard;

    //TODO: This is wrong, it should be (radius - outline_radius)**2
    outColor = (dotp > pow(radius - outline_radius, 2))? outline_color: color;

    pos += sphere_position;

    mat2 proj = mat2(ProjectionMatrix[2][2], ProjectionMatrix[2][3],
                     ProjectionMatrix[3][2], ProjectionMatrix[3][3]);

    vec2 depth = proj * vec2(pos.z, 1.0);
    gl_FragDepth = 0.5 * (depth.x / depth.y) + 0.5;
}
)";
static const char points_vert[] = R"(
#version 330 core

uniform vec4 clip_plane;
uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;
uniform mat4 ModelMatrix;
uniform float radius;

layout(location = 0) in vec3 in_Position;

smooth out vec2 TexCoord;
flat out vec3 sphere_position;

vec2 TexCoords[] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

void main(void){

    TexCoord = radius * TexCoords[gl_VertexID];

    gl_ClipDistance[0] = dot(clip_plane, ModelMatrix * vec4(in_Position, 1.0));

	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    sphere_position = gl_Position.xyz / gl_Position.w;
    gl_Position += radius * vec4(TexCoords[gl_VertexID], vec2(0.0));
	gl_Position = ProjectionMatrix * gl_Position;
}
)";
