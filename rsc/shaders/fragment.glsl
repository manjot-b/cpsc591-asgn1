#version 460 core

in vec3 surfaceNormal;
in vec3 toLight[2];

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float roughness;
uniform vec3 lightColors[2];
uniform vec3 surfaceColor;
uniform vec3 toCamera;
uniform vec3 fresnel;

uniform bool useBeckmann = true;
uniform bool useGGX = false;
uniform bool useG = true;
uniform bool useF = true;
uniform bool useDenom = true;
uniform bool usePi = true;

out vec4 fragColor;

#define PI 3.1415926535
#define E 2.7182818284
#define EPSILON 1e-6

float beckmannNDF(vec3 unitNormal, vec3 midLightCamera)
{
	float alpha = acos(max(dot(unitNormal, midLightCamera), 0));
	float beta = tan(alpha) / roughness;
	float exponent = -beta * beta;
	float num = pow(E, exponent);
	float cosAlpha = cos(alpha);
	float denom = PI * roughness * roughness * cosAlpha * cosAlpha * cosAlpha * cosAlpha + EPSILON;
	return num / denom;
}

float ggxNDF(vec3 unitNormal, vec3 midLightCamera)
{
	float alpha = roughness * roughness;
	float dotNormalMid = max(dot(unitNormal, midLightCamera), 0);
	float b = dotNormalMid * dotNormalMid * (alpha * alpha - 1) + 1;
	return (alpha * alpha) / (PI * b * b);
}

float geometricAttenuation(vec3 unitToLight, vec3 unitToCamera, vec3 unitNormal)
{
	float dotNormalLight = max(dot(unitNormal, unitToLight), 0);
	float dotNormalCamera = max(dot(unitNormal, unitToCamera), 0);
	float k = (roughness + 1) * (roughness + 1) / 8.0f;
	float a = dotNormalLight / (dotNormalLight * (1 - k) + k);
	float b = dotNormalCamera / (dotNormalCamera * (1 - k) + k);
	return a * b;
}

vec3 fresnelReflectance(vec3 unitToCamera, vec3 midLightCamera)
{
	float a = (1 - max(dot(unitToCamera, midLightCamera), 0));
	return fresnel + (1 - fresnel) * a * a * a * a* a;
}

void main()
{
	vec3 finalColor = vec3(0.0f, 0.0f, 0.0f);

	// Ambient color
	vec3 ambient = ambientStrength * lightColors[0];
	finalColor += ambient;

	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitToCamera = normalize(toCamera);

	// Iterate over every light and calculate diffuse and specular
	// components of final color.
	for (int i = 0; i < 2; i++)
	{
		vec3 diffuse = vec3(0.0f);
		vec3 specular = vec3(0.0f);

		vec3 unitToLight = normalize(toLight[i]);
		float angleNormalLight = max(dot(unitNormal, unitToLight), 0);
		vec3 lightEnergy = lightColors[i] * angleNormalLight; 
		
		diffuse += diffuseStrength * surfaceColor/ pow(PI, int(usePi));

		vec3 midLightCamera = normalize(unitToCamera + unitToLight);
		
		// Set the bools from the cpu so that at most only one NDF is used at a time.
		float dBeckmann = useBeckmann ? beckmannNDF(unitNormal, midLightCamera) : 1.0f;
		float dGGX = useGGX ? ggxNDF(unitNormal, midLightCamera) : 1.0f;

		float g = useG ? geometricAttenuation(unitToLight, unitToCamera, unitNormal) : 1.0f;
		vec3 f = useF ? fresnelReflectance(unitToCamera, midLightCamera) : vec3(1.0f);
		vec3 num = dBeckmann * dGGX * g * f;

		// Don't clamp these dot products to 0 because they are part
		// of the denominator.
		float denom = useDenom ?
			4 * dot(unitNormal, unitToLight) * dot(unitToCamera, unitNormal) : 1.0f;

		specular += specularStrength * num / denom;  

		finalColor += lightEnergy * (diffuse + specular);

	}
	
	fragColor = vec4(finalColor, 1.0f);
}
