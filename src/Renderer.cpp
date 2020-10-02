#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Renderer.h"

Renderer::Renderer(std::vector<std::string> objPaths) :
	rotate(0.0f), scale(1.0f), rotationSpeed(glm::radians(5.0f)), scaleSpeed(1.1f),
	ambientStrength(.2f), diffuseStrength(1.0f), specularStrength(0.7f),
	roughness(0.01f), surfaceColor(0.722, 0.451, 0.2), fresnel(0.95f, 0.64f, 0.54f)
{
	initWindow();
	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	shader.link();
	for(const auto& path : objPaths)
	{
		models.emplace_back(path, shader);
	}	
	
	// Setup perspective and camera matricies.
	perspective = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	view = glm::lookAt(
				glm::vec3(0.f, 0.5f, 2.f),	// camera position
				glm::vec3(0.f, 0.f, 0.f),	// camera direction
				glm::vec3(0.f, 1.f, 0.f)	// up direction
			);
	//std::cout<<glm::to_string(view)<<std::endl;

	// Set default values of various constants, coefficients and colors
	// for the fragment and vertex shader.
	lightColors = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f, 0.5f, 0.5f)
	};
	lightPositions = {
		 glm::vec3(0.f, 0.f, 2.f),
		 glm::vec3(-2.f, -1.f, 2.f) 
	};

	shader.use();
	shader.setUniformMatrix4fv("perspective", perspective);
	shader.setUniformMatrix4fv("view", view);

	// This extracts the position of the camera.
	glm::vec3 toCamera = glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.f);
	shader.setUniform3fv("toCamera", 1, &toCamera);
	//std::cout<<glm::to_string(toCamera)<<std::endl;

	shader.setUniform3fv("lightColors", lightColors.size(), lightColors.data());
	shader.setUniform3fv("lightPositions", lightPositions.size(), lightPositions.data());

	fragmentSettings.useBeckmann = true;
	fragmentSettings.useGGX = false;

	shader.setUniform3fv("surfaceColor", 1, &surfaceColor);
	shader.setUniform1f("ambientStrength", ambientStrength);
	shader.setUniform1f("diffuseStrength", diffuseStrength);
	shader.setUniform1f("specularStrength", specularStrength);
	shader.setUniform1f("roughness", roughness);
	shader.setUniform3fv("fresnel", 1, &fresnel);
	glUseProgram(0);	// unbind shader
}

Renderer::~Renderer() {}

void Renderer::initWindow()
{
	// Setup glfw context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "OpenGL Example", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window,
			[](GLFWwindow* window, int newWidth, int newHeight) {
		float aspectRatio = Renderer::width / Renderer::height;
		float viewPortHeight = (1/aspectRatio) * width;
		float viewPortWidth = newWidth;
		float xPos = 0;
		float yPos = 0;

		if(viewPortHeight > newHeight)
		{
			viewPortHeight = newHeight;
			viewPortWidth = aspectRatio * newHeight;
			xPos = (newWidth - viewPortWidth) / 2.0f;	
		}
		else
		{
			yPos = (newHeight - viewPortHeight) / 2.0f;
		}

		glViewport(xPos, yPos, viewPortWidth, viewPortHeight);
	});

	glfwSetWindowUserPointer(window, static_cast<void*>(this));
	glfwSetKeyCallback(window, keyCallback);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::run()
{

	while(!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		for(auto& model : models)
		{
			model.rotate(rotate);
			model.scale(scale);
			model.setFragmentShaderSettings(fragmentSettings);
			model.update();
			model.draw();
		}
		rotate = glm::vec3(0.0f);
		scale = 1;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

	// Close window
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if(action == GLFW_REPEAT || action == GLFW_PRESS)
	{
		switch(key)
		{
			// Rotations
			case GLFW_KEY_W:
				renderer->rotate.x -= renderer->rotationSpeed;
				break;
			case GLFW_KEY_S:
				renderer->rotate.x += renderer->rotationSpeed;
				break;
			case GLFW_KEY_E:
				renderer->rotate.y += renderer->rotationSpeed;
				break;
			case GLFW_KEY_Q:
				renderer->rotate.y -= renderer->rotationSpeed;
				break;
			case GLFW_KEY_D:
				renderer->rotate.z -= renderer->rotationSpeed;
				break;
			case GLFW_KEY_A:
				renderer->rotate.z += renderer->rotationSpeed;
				break;

			// Scaling
			case GLFW_KEY_Z:
				renderer->scale *= renderer->scaleSpeed;
				break;
			case GLFW_KEY_X:
				renderer->scale /= renderer->scaleSpeed;
				break;

			// Toggle fragment shader settings.
			case GLFW_KEY_H:
				if(!renderer->fragmentSettings.useBeckmann && !renderer->fragmentSettings.useGGX)
				{
					// Using D uses Beckmann by default.
					renderer->fragmentSettings.useBeckmann = true;
				}
				else
				{
					renderer->fragmentSettings.useBeckmann = false;
					renderer->fragmentSettings.useGGX = false;
				}
				break;
			case GLFW_KEY_N:
				renderer->fragmentSettings.useBeckmann = !renderer->fragmentSettings.useBeckmann;
				renderer->fragmentSettings.useGGX = !renderer->fragmentSettings.useBeckmann;
				break;
			case GLFW_KEY_M:
				renderer->fragmentSettings.useGGX = !renderer->fragmentSettings.useGGX;
				renderer->fragmentSettings.useBeckmann = !renderer->fragmentSettings.useGGX;
				break;
		}
	}
}

