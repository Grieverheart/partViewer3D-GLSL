#version 330 core

uniform sampler2D DepthMap;

uniform vec2 projAB;
uniform mat4 projectionMatrix;

const vec3 light_p = vec3(-30.0, 30.0, 30.0);

noperspective in vec2 pass_TexCoord;
smooth in vec3 viewRay;

layout(location = 0) out float out_AO;

vec3 CalcPosition(void){
	float depth = texture(DepthMap, pass_TexCoord).r;
	float linearDepth = projAB.y / (depth - projAB.x);
	vec3 ray = viewRay / viewRay.z;
	return linearDepth * ray;
}

void main(void){
	vec2 texel_size = vec2(1.0 / 600.0);
	vec3 origin = CalcPosition();
	if(origin.z < -60) discard;
	vec2 pixOrigin = pass_TexCoord;

	vec3 dir = normalize(light_p - origin);
	vec4 tempDir = projectionMatrix * vec4(dir, 0.0);
	vec2 pixDir = -tempDir.xy / tempDir.w;
	float dirLength = length(pixDir);
	pixDir = pixDir / dirLength;
	
	
	vec2 nextT, deltaT;
	
	if(pixDir.x < 0){
		deltaT.x = -texel_size.x / pixDir.x;
		nextT.x = (floor(pixOrigin.x / texel_size.x) * texel_size.x - pixOrigin.x) / pixDir.x;
	}
	else {
		deltaT.x = texel_size.x / pixDir.x;
		nextT.x = ((floor(pixOrigin.x / texel_size.x) + 1.0) * texel_size.x - pixOrigin.x) / pixDir.x;
	}
	if(pixDir.y < 0){
		deltaT.y = -texel_size.y / pixDir.y;
		nextT.y = (floor(pixOrigin.y / texel_size.y) * texel_size.y - pixOrigin.y) / pixDir.y;
	}
	else {
		deltaT.y = texel_size.y / pixDir.y;
		nextT.y = ((floor(pixOrigin.y / texel_size.y) + 1.0) * texel_size.y - pixOrigin.y) / pixDir.y;
	}
	
	
	float t = 0.0;
	ivec2 pixIndex = ivec2(pixOrigin / texel_size);
	out_AO = 1.0;
	while(true){
		if(t > 0){
			float rayDepth = (origin + t * dir).z;
			vec2 texCoord = pixOrigin + 0.5 * pixDir * t * dirLength;
			float depth = texture(DepthMap, texCoord).r;
			float linearDepth = projAB.y / (depth - projAB.x);
			if(linearDepth > rayDepth + 0.1){
				out_AO = 0.2;
				break;
			}
		}
		if(nextT.x < nextT.y){
			t = nextT.x;
			nextT.x += deltaT.x;
			if(pixDir.x < 0) pixIndex.x -= 1;
			else pixIndex.x += 1;
		}
		else {
			t = nextT.y;
			nextT.y += deltaT.y;
			if(pixDir.y < 0) pixIndex.y -= 1;
			else pixIndex.y += 1;
		}
		if(pixIndex.x < 0 || pixIndex.x > 600 || pixIndex.y < 0 || pixIndex.y > 600) break;
	}
}