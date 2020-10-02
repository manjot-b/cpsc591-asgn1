#pragma once

#include <string>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Mesh.h"

class Model
{
	public:
		Model(const std::string &objPath, Shader shader);
		~Model();

		/**
		 *	Settings that will be set in the fragment shader.
		 */
		struct FragmentShaderSettings
		{
			bool useBeckmann;
			bool useGGX;
			bool useG;
			bool useF;
			bool useDenom;
			bool usePi;
		};
		void draw() const;
		void update();
		void rotate(const glm::vec3 &rotate);
		void scale(float scale);
		void setFragmentShaderSettings(const FragmentShaderSettings& settings);

	private:
		Shader shader;
		std::vector<Mesh*> meshes;
		FragmentShaderSettings fragmentSettings;

		glm::mat4 modelMatrix;
		glm::vec3 m_rotate;			// how much to rotate along each axis
		float m_scale;				// scale to apply to model
		glm::vec3 m_translation;	// translation vector

		struct BoundingBox
		{
			float x, y, z, width, height, depth;
		} boundingBox;

		void extractDataFromNode(const aiScene* scene, const aiNode* node);
		void sendUniforms() const;
};
