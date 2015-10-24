#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 InvProjectionMatrix;
uniform mat4 MVMatrix;
uniform vec4 clip_plane;
uniform float perspective_scale;
uniform float radius;
uniform bool clip;
uniform mat4 ModelMatrix;
uniform vec3 in_Color;

layout(location = 0) in vec3 in_Position;

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

//TODO: There seems to be a (precision) problem when rendering with clip
//and perspective projection, and when the view is nearly parallel to the
//clip plane.
void main(void){

    pass_Color = in_Color;

	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    sphere_position = gl_Position.xyz / gl_Position.w;
    gl_Position += vec4(perspective_scale * radius * TexCoords[gl_VertexID], vec2(0.0));
    vec3 view_position = gl_Position.xyz / gl_Position.w;
	gl_Position = ProjectionMatrix * gl_Position;

    vec4 ray_origin_temp = InvProjectionMatrix * vec4(gl_Position.xy / gl_Position.w, -1.0, 1.0);
    ray_origin = ray_origin_temp.xyz / ray_origin_temp.w;
    ray_dir = normalize(view_position - ray_origin);

    if(clip){
        view_clip_plane = transpose(inverse(MVMatrix)) * clip_plane;
        float t = -(dot(ray_origin, view_clip_plane.xyz) + view_clip_plane.w) / dot(ray_dir, view_clip_plane.xyz);
        clip_position = ray_origin + ray_dir * t;
    }
}
