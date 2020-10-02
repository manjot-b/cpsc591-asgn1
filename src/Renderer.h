#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>

#include "Model.h"

class Renderer
{
	public:
		Renderer(std::vector<std::string> objPaths);
		~Renderer();
		void run();

	private:
		GLFWwindow* window;
		std::vector<Model> models;
		
		static const unsigned int height = 600;
		static const unsigned int width = 800;
		const float aspectRatio = float(width) / height;

		glm::vec3 rotate;
		float scale;
		glm::mat4 view;
		glm::mat4 perspective;

		float rotationSpeed;
		float scaleSpeed;
	
		std::array<glm::vec3, 2> lightColors;
		std::array<glm::vec3, 2> lightPositions;
		std::array<glm::vec3, 10> fresnels;
		Model::FragmentShaderSettings fragmentSettings;

		void initWindow();
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
