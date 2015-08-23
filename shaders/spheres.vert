#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 InvProjectionMatrix;
uniform mat4 MVMatrix;
uniform vec4 clip_plane;
uniform float perspective_scale;
uniform float radius;
uniform bool clip;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in mat4 ModelMatrix;

flat out vec3 pass_Color;
noperspective out vec3 ray_origin;
noperspective out vec3 ray_dir;
noperspective out vec3 clip_position;
flat out vec3 sphere_position;
flat out vec4 view_clip_plane;

vec2 TexCoords[] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

//TODO: Improve, and fix square size!
void main(void){

    pass_Color = in_Color;

	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    sphere_position = gl_Position.xyz / gl_Position.w;
    gl_Position += vec4(perspective_scale * radius * TexCoords[gl_VertexID], vec2(0.0));
    vec3 view_position = gl_Position.xyz / gl_Position.w;
	gl_Position = ProjectionMatrix * gl_Position;

    vec4 ray_origin_temp = InvProjectionMatrix * vec4(gl_Position.xy / gl_Position.w, -1.0, 1.0);
    ray_origin = ray_origin_temp.xyz / ray_origin_temp.w;
    ray_dir = view_position - ray_origin;

    if(clip){
        vec4 clip_view_pos = MVMatrix * vec4(clip_plane.w * clip_plane.xyz, 1.0);
        vec3 clip_view_dir = normalize(mat3(transpose(inverse(MVMatrix))) * clip_plane.xyz);
        float clip_view_dist = -dot(clip_view_dir, (clip_view_pos.xyz / clip_view_pos.w));
        view_clip_plane = vec4(clip_view_dir, clip_view_dist);

        float t = -(dot(ray_origin, view_clip_plane.xyz) + view_clip_plane.w) / dot(normalize(ray_dir), view_clip_plane.xyz);
        clip_position = ray_origin + normalize(ray_dir) * t;
    }
}
