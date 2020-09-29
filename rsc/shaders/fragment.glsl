#version 460 core

in vec3 surfaceNormal;

uniform float ambientStrength;
uniform vec3 lightColors[2];

out vec4 fragColor;

void main()
{
	// Ambient color
	vec3 ambient = ambientStrength * lightColors[0];
	
	vec3 unitNormal = normalize(surfaceNormal);
	
	fragColor = vec4(ambient, 1.0f);
}
