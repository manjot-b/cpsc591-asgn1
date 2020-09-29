#version 460 core

layout (location = 0) in vec3 inPosition;
//layout (location = 1) in vec3 inColor;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform vec3 lightPosition;

out vec3 surfaceNormal;

void main()
{
	vec4 worldPosition = model * vec4(inPosition, 1.0f);
    gl_Position = perspective * view * worldPosition;

	surfaceNormal = (model * vec4(inNormal, 1.0f)).xyz;
}
