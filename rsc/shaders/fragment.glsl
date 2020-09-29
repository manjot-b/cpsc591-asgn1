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

out vec4 fragColor;

const float pi = 3.1415926535;
const float e = 2.7182818284;

float beckmannNDF(vec3 unitNormal, vec3 midLightCamera)
{
	float alpha = acos(dot(unitNormal, midLightCamera));	
	float beta = tan(alpha) / roughness;
	float exponent = -beta * beta;
	float num = pow(e, exponent);
	float cosAlpha = cos(alpha);
	float denom = pi * roughness * roughness * cosAlpha * cosAlpha * cosAlpha * cosAlpha;
	return num / denom;
}

float ggxNDF(vec3 unitNormal, vec3 midLightCamera)
{
	float alpha = roughness * roughness;
	float dotNormalMid = dot(unitNormal, midLightCamera);
	float b = dotNormalMid * dotNormalMid * (alpha * alpha - 1) + 1;
	return (alpha * alpha) / (pi * b * b);
}

float geometricAttenuation(vec3 unitToLight, vec3 unitToCamera, vec3 unitNormal)
{
	float dotNormalLight = dot(unitNormal, unitToLight);
	float dotNormalCamera = dot(unitNormal, unitToCamera);
	float k = (roughness + 1) * (roughness + 1) / 8.0f;
	float a = dotNormalLight / (dotNormalLight * (1 - k) + k);
	float b = dotNormalCamera / (dotNormalCamera * (1 - k) + k);
	return a * b;
}

vec3 fresnelReflectance(vec3 unitToCamera, vec3 midLightCamera)
{
	float a = (1 - dot(unitToCamera, midLightCamera));
	return fresnel + (1 - fresnel) * a * a * a * a* a;
}

void main()
{
	// Ambient color
	vec3 ambient = ambientStrength * lightColors[0];
	vec3 diffuse = vec3(0, 0, 0);
	vec3 specular = vec3(0, 0, 0);

	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitToCamera = normalize(toCamera);

	// Iterate over every light and calculate diffuse and specular
	// components of final color.
	for (int i = 0; i < 2; i++)
	{
		vec3 unitToLight = normalize(toLight[i]);
		vec3 lightEnergy = lightColors[i] * max(dot(unitNormal, unitToLight), 0);
		
		diffuse += lightEnergy * diffuseStrength / pi;

		// Beckmann NDF
		vec3 midLightCamera = normalize(unitToCamera + unitToLight);
		//float d = beckmannNDF(unitNormal, midLightCamera);
		float d = ggxNDF(unitNormal, midLightCamera);
		float g = geometricAttenuation(unitToLight, unitToCamera, unitNormal);
		vec3 f = fresnelReflectance(unitToCamera, midLightCamera);
		vec3 num = d * g * f;
		float denom = 4 * dot(unitToLight, unitNormal) * dot(unitToCamera, unitNormal);

		specular += lightEnergy * specularStrength * num / denom;  
	}
	
	vec3 finalColor = (ambient + diffuse + specular) * surfaceColor;
	fragColor = vec4(finalColor, 1.0f);
}
