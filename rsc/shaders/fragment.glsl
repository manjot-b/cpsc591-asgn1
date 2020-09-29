#version 460 core

in vec3 surfaceNormal;
in vec3 toLight[2];

uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3 lightColors[2];
uniform vec3 surfaceColor;

out vec4 fragColor;

void main()
{
	const float pi = 3.1415926535;
	// Ambient color
	vec3 ambient = ambientStrength * lightColors[0];
	vec3 diffuse = vec3(0, 0, 0);

	vec3 unitNormal = normalize(surfaceNormal);

	// Iterate over every light and calculate diffuse and specular
	// components of final color.
	for (int i = 0; i < 2; i++)
	{
		vec3 unitToLight = normalize(toLight[i]);
		vec3 lightEnergy = lightColors[i] * max(dot(unitNormal, unitToLight), 0);
		
		diffuse += lightEnergy * diffuseStrength * surfaceColor / pi;
	}
	
	fragColor = vec4(ambient + diffuse, 1.0f);
}
