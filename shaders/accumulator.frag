#version 330 core

struct Light{
	vec3 direction;
};

uniform Light light;

uniform sampler2D DepthMap;
uniform sampler2D ColorMap;
uniform sampler2D NormalMap;

uniform vec2 projAB;

noperspective in vec2 pass_TexCoord;
smooth in vec3 viewRay;

out vec4 out_Color;

vec3 CalcPosition(void){
	float depth = texture(DepthMap, pass_TexCoord).r;
	float linearDepth = projAB.y / (depth - projAB.x);
	vec3 ray = normalize(viewRay);
	ray = ray / ray.z;
	return linearDepth * ray;
}

vec3 CalcLight(vec3 position, vec3 normal){
	
	vec3 DiffuseColor = vec3(0.0);
	vec3 SpecularColor = vec3(0.0);
	
	vec3 light_Direction = -normalize(light.direction);
	float diffuse = max(0.0, dot(normal, light_Direction));
	
	if(diffuse > 0.0){
		DiffuseColor = diffuse * vec3(1.0);
		
		vec3 camera_Direction = normalize(-position);
		vec3 half_vector = normalize(camera_Direction + light_Direction);
		
		float specular = max(0.0, dot(normal, half_vector));
		float fspecular = pow(specular, 128.0);
		SpecularColor = fspecular * vec3(1.0);
	}
	return DiffuseColor + SpecularColor + vec3(0.25);
}

void main(void){

	vec2 TexCoord = pass_TexCoord;
	vec3 Position = CalcPosition();
	vec3 Color = texture(ColorMap, TexCoord).rgb;
	float AO = texture(NormalMap, TexCoord).a;
	vec3 Normal = normalize(texture(NormalMap, TexCoord).xyz);
	
	out_Color = vec4(Color * CalcLight(Position, Normal) * AO, 1.0);
}