#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <string>
#include <tuple>

#include "Model.h"

class Renderer
{
	public:
		Renderer(const char* modelDirectory);
		~Renderer();
		void run();

	private:
		GLFWwindow* window;
		Shader* shader;
		std::vector<std::tuple<std::string, Model*>> models;
		unsigned int modelIndex;

		const unsigned int height = 800;
		const unsigned int width = 800;
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
		void loadModels(const char* modelDirectory);
		void printSettings(bool clear);
};
