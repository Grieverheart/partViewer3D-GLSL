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

