#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Renderer.h"

Renderer::Renderer(std::vector<std::string> objPaths) :
	rotate(0.0f), scale(1.0f), ambientStrength(0.3f), diffuseStrength(0.8f),
	specularStrength(0.7f), roughness(0.3f), surfaceColor(0.722, 0.451, 0.2),
	fresnel(1.0f, 0.71f, 0.29f)
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
				glm::vec3(0.f, 0.1f, 2.f),	// camera position
				glm::vec3(0.f, 0.f, 0.f),	// camera direction
				glm::vec3(0.f, 1.f, 0.f)	// up direction
			);
	//std::cout<<glm::to_string(view)<<std::endl;

	// Set default values of various constants, coefficients and colors
	// for the fragment and vertex shader.
	lightColors = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
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
	glEnable(GL_DEPTH_TEST);

}

void Renderer::run()
{

	while(!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		processWindowInput();

		for(auto& model : models)
		{
			model.rotate(rotate);
			model.scale(scale);
			model.update();
			model.draw();
		}
		rotate = glm::vec3(0.0f);
		scale = 1;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Renderer::processWindowInput()
{
	float rotationSpeed = glm::radians(3.0f);
	float scaleSpeed = 1.01;

	// Close window
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// Rotations
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		rotate.x -= rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		rotate.x += rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		rotate.y += rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		rotate.y -= rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		rotate.z -= rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		rotate.z += rotationSpeed;
	}

	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		scale *= scaleSpeed;
	}
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		scale /= scaleSpeed;
	}
}

