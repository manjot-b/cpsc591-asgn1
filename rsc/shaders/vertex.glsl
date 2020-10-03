#version 330 core

layout (location = 0) in vec3 inPosition;
//layout (location = 1) in vec3 inColor;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform vec3 lightPositions[2];

out vec3 surfaceNormal;
out vec3 toLight[2];

void main()
{
	vec4 worldPosition = model * vec4(inPosition, 1.0f);
    gl_Position = perspective * view * worldPosition;

	surfaceNormal = (model * vec4(inNormal, 1.0f)).xyz;

	for(int i = 0; i < 2; i++)
	{
		toLight[i] = lightPositions[i] - worldPosition.xyz;
	}
}
