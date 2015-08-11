#version 330 core

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D noise;

uniform mat4 projectionMatrix;
uniform mat2 depth_iproj;
uniform vec3 kernel[256];
uniform float RADIUS;
uniform ivec2 noiseScale;
uniform int kernelSize;

noperspective in vec2 TexCoord;
smooth in vec3 ray_origin;
smooth in vec3 ray_direction;

layout(location = 0) out vec4 out_AO;

vec3 CalcPosition(void){
	vec2 depth = depth_iproj * vec2(2.0 * texture(DepthMap, TexCoord).r - 1.0, 1.0);
	return ray_origin + ray_direction * (depth.x / (depth.y * ray_direction.z));
}

mat3 CalcRMatrix(vec3 normal, vec2 texcoord){
	vec3 rvec = texture(noise, texcoord * noiseScale).xyz;
	vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(normal, tangent);
	
	return mat3(tangent, bitangent, normal);
}


void main(void){

	vec2 TexCoord = TexCoord;
	vec3 Normal = texture(NormalMap, TexCoord).xyz;
	if(Normal != 0.0){
		vec3 Position = CalcPosition();
		Normal = normalize(Normal);
		mat3 RotationMatrix = RADIUS * CalcRMatrix(Normal, TexCoord);
		
		float occlusion = 0.0;
		
		for(int i = 0; i < kernelSize; i++){
			// Get sample position
			vec3 sample = RotationMatrix * kernel[i] + Position;
			// Project and bias sample position to get its texture coordinates
			vec4 offset = projectionMatrix * vec4(sample, 1.0);
			offset.xy = (offset.xy / offset.w) * 0.5 + 0.5;
			// Get sample depth
			vec2 sample_depth = depth_iproj * vec2(2.0 * texture(DepthMap, offset.xy).r - 1.0, 1.0);
            occlusion += step(sample.z, sample_depth.x / sample_depth.y);
		}
		out_AO = vec4(1.0 - occlusion / kernelSize, Normal);
	}
	else discard;
}
