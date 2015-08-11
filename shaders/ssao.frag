#version 330 core

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D noise;

uniform vec2 projAB;
uniform ivec2 noiseScale;
uniform int kernelSize;
uniform vec3 kernel[256];
uniform float RADIUS;
uniform mat4 projectionMatrix;

noperspective in vec2 TexCoord;
smooth in vec3 viewRay;

layout(location = 0) out vec4 out_AO;

vec3 CalcPosition(void){
	float depth = texture(DepthMap, TexCoord).r;
	float linearDepth = projAB.y / (2.0 * depth - 1.0 - projAB.x);
	return linearDepth * (viewRay / viewRay.z);
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
			float sample_depth = texture(DepthMap, offset.xy).r;
			sample_depth = projAB.y / (2.0 * sample_depth - 1.0 - projAB.x);
            occlusion += step(sample.z, sample_depth);
		}
		out_AO = vec4(1.0 - occlusion / kernelSize, Normal);
	}
	else discard;
}
