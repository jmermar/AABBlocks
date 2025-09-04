layout(set = 0,
	   binding = 0) readonly uniform CameraData
{
	mat4 proj;
	mat4 view;
	mat4 projView;
	mat4 iProjViewMatrix;
	mat4 iViewMatrix;
	mat4 iProjMatrix;
	vec4 planes[6];
	vec3 cameraPosition;
	float ambientLight;
	vec3 lightDirection;
	float lightIntensity;
	vec3 fogColor;
	float fogDensity;
	float exposure;
}
ubo;