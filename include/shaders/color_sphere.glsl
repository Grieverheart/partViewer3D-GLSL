static const char color_sphere_frag[] = R"(
#version 330 core

uniform vec3 color;
uniform float radius;

noperspective in vec3 ray_origin;
noperspective in vec3 ray_dir;
flat in vec3 sphere_position;

layout(location = 0) out vec3 outColor;

//TODO: Improve this?
bool intersect_sphere(in vec3 position, in float radius, in vec3 ray_r0, in vec3 ray_dir){
	vec3 direction = position - ray_r0;
	float B = dot(ray_dir, direction);
	float det = B * B - dot(direction, direction) + radius * radius;

	if(det < 0.0) return false;

	return true;
}

void main(void){
    vec3 ray_dir_ = normalize(ray_dir);
    if(!intersect_sphere(sphere_position, radius, ray_origin, ray_dir_)) discard;
    outColor = color;
}
)";
static const char color_sphere_vert[] = R"(
#version 330 core

uniform mat4 projection_matrix;
uniform mat4 iprojection_matrix;
uniform mat4 mv_matrix;
uniform float radius;
uniform float perspective_scale;

layout(location = 0) in vec3 in_Position;

noperspective out vec3 ray_origin;
noperspective out vec3 ray_dir;
flat out vec3 sphere_position;

vec2 TexCoords[] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

//TODO: Improve, and fix square size!
void main(void){

	gl_Position = mv_matrix * vec4(in_Position, 1.0);
    sphere_position = gl_Position.xyz / gl_Position.w;
    gl_Position += vec4(perspective_scale * radius * TexCoords[gl_VertexID], vec2(0.0));
    vec3 view_position = gl_Position.xyz / gl_Position.w;
	gl_Position = projection_matrix * gl_Position;

    vec4 ray_origin_temp = iprojection_matrix * vec4(gl_Position.xy / gl_Position.w, -1.0, 1.0);
    ray_origin = ray_origin_temp.xyz / ray_origin_temp.w;
    ray_dir = view_position - ray_origin;
}
)";
