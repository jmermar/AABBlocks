#version 450
#include "includes/globalData.h"
#include "includes/pbr.h"

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 iRay;

layout(set = 1,
	   binding = 0) uniform sampler2D gAlbedo;
layout(set = 1,
	   binding = 1) uniform sampler2D gNormal;
layout(set = 1,
	   binding = 2) uniform sampler2D gPosition;
layout(set = 1,
	   binding = 3) uniform sampler2D gMaterial;
layout(set = 1,
	   binding = 5) uniform sampler2D gDepth;
layout(set = 1,
	   binding = 6) uniform samplerCube skybox;

layout(location = 0) out vec4 outColor;

vec3 ACESFilmSimple(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x * (a * x + b)) /
					 (x * (c * x + d) + e),
				 0,
				 1);
}

void main()
{

	vec3 albedo = texture(gAlbedo, fragUV).rgb;
	vec3 position =
		texture(gPosition, fragUV).rgb;
	vec3 normal = normalize(
		texture(gNormal, fragUV).xyz * 2 - 1);
	float metallic = texture(gMaterial, fragUV).r;
	float roughness = 0.3 + (1 - metallic) * 0.7;
	vec3 camDir = normalize(
		ubo.cameraPosition.xyz - position);
	float depth = texture(gDepth, fragUV).r;
	vec3 color;
	vec3 skyColor = texture(skybox, iRay).xyz;
	vec3 refVec = reflect(-camDir, normal);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	if(depth == 1.0)
	{
		vec3 sun =
			vec3(1) *
			pow(clamp(
					dot(normalize(iRay),
						normalize(
							ubo.lightDirection)),
					0,
					1),
				100) *
			50;
		color = skyColor * 1 + sun;
	}
	else
	{
		color =
			brdf(normalize(ubo.lightDirection),
				 vec3(1) * ubo.lightIntensity,
				 position,
				 normal,
				 ubo.cameraPosition.xyz,
				 albedo,
				 F0,
				 roughness,
				 metallic);
		vec3 N = normal;
		vec3 V = normalize(
			ubo.cameraPosition.xyz - position);
		vec3 irradiance =
			texture(skybox, refVec).rgb;
		vec3 diffuse = albedo;
		vec3 F = fresnelSchlickRoughness(
			max(dot(N, V), 0.0), F0, roughness);
		vec3 specular = irradiance * F;
		vec3 ambient =
			((diffuse * (1.0 - metallic)) +
			 specular) *
			ubo.ambientLight;

		color += ambient;
	}
	outColor = vec4(
		ACESFilmSimple(color * ubo.exposure), 1);
}
