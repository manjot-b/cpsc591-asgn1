#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <filesystem>

#include "Renderer.h"

Renderer::Renderer(const char* modelDirectory) :
	modelIndex(0), rotate(0.0f), scale(1.0f), rotationSpeed(glm::radians(5.0f)),
	scaleSpeed(1.1f)
{
	initWindow();
	shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	shader->link();

	loadModels(modelDirectory);
	
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

	fresnels = {
		glm::vec3(0.15f, 0.15f, 0.15f), // Water
		glm::vec3(0.21f, 0.21f, 0.21f), // Plastic / glass (low)
		glm::vec3(0.24f, 0.24f, 0.24f), // Plastic high
		glm::vec3(0.31f, 0.31f, 0.31f), // Glass (high) / ruby
		glm::vec3(0.45f, 0.45f, 0.45f), // Diamond
		glm::vec3(0.77f, 0.78f, 0.78f), // Iron
		glm::vec3(0.98f, 0.82f, 0.76f), // Copper
		glm::vec3(1.00f, 0.86f, 0.57f), // Gold
		glm::vec3(0.96f, 0.96f, 0.97f), // Aluminium
		glm::vec3(0.98f, 0.97f, 0.95f), // SIlver
	};

	shader->use();
	shader->setUniformMatrix4fv("perspective", perspective);
	shader->setUniformMatrix4fv("view", view);

	// This extracts the position of the camera.
	glm::vec3 toCamera = glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.f);
	shader->setUniform3fv("toCamera", 1, &toCamera);
	//std::cout<<glm::to_string(toCamera)<<std::endl;

	shader->setUniform3fv("lightColors", lightColors.size(), lightColors.data());
	shader->setUniform3fv("lightPositions", lightPositions.size(), lightPositions.data());

	fragmentSettings.useBeckmann = true;
	fragmentSettings.useGGX = false;
	fragmentSettings.useG = true;
	fragmentSettings.useF = true;
	fragmentSettings.usePi = true;
	fragmentSettings.useDenom = true;

	fragmentSettings.roughness = 0.0f;
	fragmentSettings.ambientStrength = 0.15f;
	fragmentSettings.diffuseStrength = 1.0f;
	fragmentSettings.specularStrength = 0.7f;
	fragmentSettings.surfaceColor = glm::vec3(0.722f, 0.451f, 0.2f);
	fragmentSettings.fresnel = fresnels[6];

	glUseProgram(0);	// unbind shader
}

Renderer::~Renderer()
{
	for (auto m : models)
	{
		delete m;
	}
	delete shader;
}

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

void Renderer::loadModels(const char* modelDirectory)
{
	namespace fs = std::filesystem;
	const std::string extension = ".obj";

	unsigned int count = 1;
	for (const auto& entry : fs::directory_iterator(modelDirectory))
	{
		if (entry.is_regular_file() && entry.path().extension() == extension)
		{
			std::cout << "Loading " << entry.path() << "...";
			models.push_back(new Model(entry.path(), *shader));
			std::cout << "Done! Index: " << count << "\n";
			count++;
		}
	}
}

void Renderer::run()
{

	while(!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		models[modelIndex]->rotate(rotate);
		models[modelIndex]->scale(scale);
		models[modelIndex]->setFragmentShaderSettings(fragmentSettings);
		models[modelIndex]->update();
		models[modelIndex]->draw();

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
		Model::FragmentShaderSettings& fragmentSettings = renderer->fragmentSettings;
		float change = 0.05f;

		if(!(mods & GLFW_MOD_SHIFT))
		{
			switch(key)
			{
				// Select model
				case GLFW_KEY_1:
				case GLFW_KEY_2:
				case GLFW_KEY_3:
				case GLFW_KEY_4:
				case GLFW_KEY_5:
				case GLFW_KEY_6:
				case GLFW_KEY_7:
					renderer->modelIndex = key - GLFW_KEY_1;
					break;
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
					if(!fragmentSettings.useBeckmann && !fragmentSettings.useGGX)
					{
						// Using D uses Beckmann by default.
						fragmentSettings.useBeckmann = true;
					}
					else
					{
						fragmentSettings.useBeckmann = false;
						fragmentSettings.useGGX = false;
					}
					break;
				case GLFW_KEY_M:
					fragmentSettings.useBeckmann = !fragmentSettings.useBeckmann;
					fragmentSettings.useGGX = !fragmentSettings.useBeckmann;
					break;
				case GLFW_KEY_J:
					fragmentSettings.useG = !fragmentSettings.useG;
					break;
				case GLFW_KEY_K:
					fragmentSettings.useF = !fragmentSettings.useF;
					break;
				case GLFW_KEY_P:
					fragmentSettings.usePi = !fragmentSettings.usePi;
					break;
				case GLFW_KEY_O:
					fragmentSettings.useDenom = !fragmentSettings.useDenom;
					break;

				// Change scalar values
				case GLFW_KEY_T:
					fragmentSettings.roughness = glm::min(
							fragmentSettings.roughness + change*0.5f,
							1.0f);
					break;
				case GLFW_KEY_Y:
					fragmentSettings.ambientStrength = glm::min(
							fragmentSettings.ambientStrength + change,
							1.0f);
					break;
				case GLFW_KEY_U:
					fragmentSettings.specularStrength = glm::min(
							fragmentSettings.specularStrength + change,
							1.0f);
					break;
				case GLFW_KEY_I:
					fragmentSettings.diffuseStrength = glm::min(
							fragmentSettings.diffuseStrength + change,
							1.0f);
					break;
				case GLFW_KEY_R:
					fragmentSettings.surfaceColor.r = glm::min(
							fragmentSettings.surfaceColor.r + change,
							1.0f);
					break;
				case GLFW_KEY_G:
					fragmentSettings.surfaceColor.g = glm::min(
							fragmentSettings.surfaceColor.g + change,
							1.0f);
					break;
				case GLFW_KEY_B:
					fragmentSettings.surfaceColor.b = glm::min(
							fragmentSettings.surfaceColor.b + change,
							1.0f);
					break;
			}
		}
		else // Shift key pressed
		{
			switch(key)
			{
				// Change scalar values
				case GLFW_KEY_T:
					fragmentSettings.roughness = glm::max(
							fragmentSettings.roughness - change*0.5f,
							0.0f);
					break;
				case GLFW_KEY_Y:
					fragmentSettings.ambientStrength = glm::max(
							fragmentSettings.ambientStrength - change,
							0.0f);
					break;
				case GLFW_KEY_U:
					fragmentSettings.specularStrength = glm::max(
							fragmentSettings.specularStrength - change,
							0.0f);
					break;
				case GLFW_KEY_I:
					fragmentSettings.diffuseStrength = glm::max(
							fragmentSettings.diffuseStrength - change,
							0.0f);
					break;
				case GLFW_KEY_R:
					fragmentSettings.surfaceColor.r = glm::max(
							fragmentSettings.surfaceColor.r - change,
							0.0f);
					break;
				case GLFW_KEY_G:
					fragmentSettings.surfaceColor.g = glm::max(
							fragmentSettings.surfaceColor.g - change,
							0.0f);
					break;
				case GLFW_KEY_B:
					fragmentSettings.surfaceColor.b = glm::max(
							fragmentSettings.surfaceColor.b - change,
							0.0f);
					break;
				case GLFW_KEY_0: // 0 is last number on keyboard.
					fragmentSettings.fresnel = renderer->fresnels.back(); 
					break;
				case GLFW_KEY_1:
				case GLFW_KEY_2:
				case GLFW_KEY_3:
				case GLFW_KEY_4:
				case GLFW_KEY_5:
				case GLFW_KEY_6:
				case GLFW_KEY_7:
				case GLFW_KEY_8:
				case GLFW_KEY_9:
					unsigned int index = key - GLFW_KEY_0 - 1;
					fragmentSettings.fresnel = renderer->fresnels[index];
					break;
			}
		}
	}
}

