#version 330 core

struct Light{
	vec3 direction;
	float Si;
	float Di;
	float Ai;
	float Intensity;
};

uniform Light light;

uniform sampler2D DepthMap;
uniform sampler2D ColorMap;
uniform sampler2D NormalMap;
// uniform sampler2D IDMap;

uniform vec2 projAB;
uniform vec3 bgColor;

noperspective in vec2 pass_TexCoord;
smooth in vec3 viewRay;

layout(location = 0) out vec4 out_Color;

vec3 CalcPosition(float depth){
	float linearDepth = projAB.y / (depth - projAB.x);
	vec3 ray = viewRay / viewRay.z;
	return linearDepth * ray;
}

vec3 CalcLight(vec3 position, vec3 normal){
	
	vec3 DiffuseColor = vec3(0.0);
	vec3 SpecularColor = vec3(0.0);
	
	vec3 light_Direction = -normalize(light.direction);
	float diffuse = max(0.0, dot(normal, light_Direction));
	
	if(diffuse > 0.0){
		DiffuseColor = vec3(diffuse);
		
		vec3 half_vector = normalize(light_Direction - normalize(position));
		
		float specular = max(0.0, dot(normal, half_vector));
		float fspecular = pow(specular, 128.0);
		SpecularColor = vec3(fspecular);
	}
	return light.Intensity * (light.Di * DiffuseColor + light.Si * SpecularColor) + vec3(light.Ai);
}

bool isEdge(vec3 normal){
	vec3 normalRight = textureOffset(NormalMap, pass_TexCoord, ivec2(1, 0)).xyz; //Assume normalized
	vec3 normalUp = textureOffset(NormalMap, pass_TexCoord, ivec2(0, 1)).xyz; //Assume normalized
	
	
	float amountX = dot(normalRight, normal);
	float amountY = dot(normalUp, normal);
	
	if(amountX < 0.9 || amountY < 0.9 ) return true;
	else return false;
}

void main(void){

	vec2 TexCoord = pass_TexCoord;
	float Depth = texture(DepthMap, pass_TexCoord).r;
	vec3 Position = CalcPosition(Depth);
	vec3 Color = texture(ColorMap, TexCoord).rgb;
	if(Color == 0) out_Color = vec4(bgColor, 0.0);
	else {
		vec4 NormalAO = texture(NormalMap, TexCoord); //Assume normalized
		vec3 Normal = NormalAO.xyz;
		float AO = NormalAO.a;
	
		out_Color = vec4(Color * CalcLight(Position, Normal) * AO, 1.0);
		if(isEdge(Normal)) out_Color = out_Color * 0.7;
	}
}