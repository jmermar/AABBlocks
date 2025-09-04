const float PI = 3.141592;
const float Epsilon = 0.00001;

float DistributionGGX(vec3 N,
					  vec3 H,
					  float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV,
						 float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N,
					vec3 V,
					vec3 L,
					float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 =
		GeometrySchlickGGX(NdotV, roughness);
	float ggx1 =
		GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 +
		   (1.0 - F0) * pow(clamp(1.0 - cosTheta,
								  0.0,
								  1.0),
							5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta,
							 vec3 F0,
							 float roughness)
{
	return F0 +
		   (max(vec3(1.0 - roughness), F0) - F0) *
			   pow(clamp(
					   1.0 - cosTheta, 0.0, 1.0),
				   5.0);
}

vec3 brdf(vec3 lightDirection,
		  vec3 radiance,
		  vec3 pos,
		  vec3 normal,
		  vec3 cameraPos,
		  vec3 albedo,
		  vec3 F0,
		  float roughness,
		  float metallic)
{
	vec3 N = normal;
	vec3 V = normalize(cameraPos - pos);
	vec3 R = reflect(-V, N);
	// calculate per-light radiance
	vec3 L = normalize(lightDirection);
	vec3 H = normalize(V + L);

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F =
		fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 numerator = NDF * G * F;
	float denominator =
		4.0 * max(dot(N, V), 0.0) *
			max(dot(N, L), 0.0) +
		0.0001; // + 0.0001 to prevent divide by zero
	vec3 specular = numerator / denominator;

	// kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// add to outgoing radiance Lo
	return (kD * albedo / PI + specular) *
		   radiance *
		   NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

vec3 applyFog(vec3 color,
			  vec3 fragPos,
			  vec3 viewPos,
			  vec3 fogColor,
			  float fogDensity)
{
	float distance = length(viewPos - fragPos);
	float fogFactor =
		1.0 -
		exp(-pow((distance * fogDensity), 2.0));
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	return mix(color, fogColor, fogFactor);
}