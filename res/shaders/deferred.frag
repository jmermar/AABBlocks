#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 1, binding = 0) uniform sampler2D gAlbedo;
layout(set = 1, binding = 1) uniform sampler2D gNormal;
layout(set = 1, binding = 2) uniform sampler2D gPosition;
layout(set = 1, binding = 3) uniform sampler2D gMaterial;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly uniform CameraData {
    mat4 proj;
    mat4 view;
    mat4 projView;
	vec4 planes[6];
	vec3 cameraPosition;
    float ambientLight;
    vec3 lightDirection;
    float lightIntensity;
} ubo;

const float PI = 3.141592;
const float Epsilon = 0.00001;

float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = texture(gAlbedo, fragUV).rgb;
	vec3 position = texture(gPosition, fragUV).rgb;
    vec3 normal = normalize(texture(gNormal, fragUV).xyz * 2 - 1);
    float metallic = texture(gMaterial, fragUV).r;
    float roughness = texture(gMaterial, fragUV).g;
    metallic = 0.3;
    roughness = 0.4;

    // Materials
    vec3 Fdielectric = vec3(0.04);

	vec3 Lo = normalize(ubo.cameraPosition.xyz - position);

	vec3 N = normal;
	
	float cosLo = max(0.0, dot(N, Lo));
		
	vec3 Lr = 2.0 * cosLo * N - Lo;

	vec3 F0 = mix(Fdielectric, albedo, metallic);

	vec3 directLighting = albedo.rgb * ubo.ambientLight;

    vec3 Li = normalize(ubo.lightDirection);
    vec3 Lradiance = vec3(ubo.lightIntensity);

    vec3 Lh = normalize(Li + Lo);

    float cosLi = max(0.0, dot(N, Li));
    float cosLh = max(0.0, dot(N, Lh));

    vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    float D = ndfGGX(cosLh, roughness);
    float G = gaSchlickGGX(cosLi, cosLo, roughness);

    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metallic);

    vec3 diffuseBRDF = kd * albedo;

    vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;

    outColor = vec4(directLighting, 0);
}
