static const char spheres_frag[] = R"(
#version 330 core

uniform mat4 ProjectionMatrix;
uniform float radius;
uniform bool clip;

flat in vec3 pass_Color;
noperspective in vec3 ray_origin;
noperspective in vec3 ray_dir;
noperspective in vec3 clip_position;
flat in vec3 sphere_position;
flat in vec4 view_clip_plane;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

//TODO: Improve this
bool intersect_sphere(in vec3 position, in float radius, in vec3 ray_r0, in vec3 ray_dir, inout float t){
	vec3 direction = position - ray_r0;
	float B = dot(ray_dir, direction);
	float det = B * B - dot(direction, direction) + radius * radius;

	if(det < 0.0) return false;

	float t0 = B + sqrt(det);
	float t1 = B - sqrt(det);

    bool ret_val = false;

	if((t0 < t) && (t0 > 0.0)){
		t = t0;
        ret_val = true;
	}

	if((t1 < t) && (t1 > 0.0)){
		t = t1;
        ret_val = true;
	}

	return ret_val;
}

void main(void){
    float t = 10000000.0;
    vec3 ray_dir_ = normalize(ray_dir);
    if(clip && dot(view_clip_plane, vec4(sphere_position, 1.0)) < -radius) discard; //TODO: Check performance when removing this
    if(!intersect_sphere(sphere_position, radius, ray_origin, ray_dir_, t)) discard;

    vec3 hit_position = ray_origin + t * ray_dir_;

    if(clip && dot(view_clip_plane, vec4(hit_position, 1.0)) < 0.0){
        vec3 pos = clip_position - sphere_position;
        if(dot(pos, pos) > radius * radius) discard;

        hit_position = clip_position;
        outNormal = -view_clip_plane.xyz;
    }
    else outNormal = normalize(hit_position - sphere_position);

    outColor = pass_Color;

    mat2 proj = mat2(ProjectionMatrix[2][2], ProjectionMatrix[2][3],
                     ProjectionMatrix[3][2], ProjectionMatrix[3][3]);

    vec2 pos = proj * vec2(hit_position.z, 1.0);

    gl_FragDepth = 0.5 * (pos.x / pos.y) + 0.5;
}

)";
static const char spheres_vert[] = R"(
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
)";
