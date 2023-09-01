#pragma once
#include "Mesh.h"

namespace MKEngine
{
	class Model {
	public:
		Model() = default;
		void Draw(VkCommandBuffer commandBuffer) const;
		static Model LoadModel(const char* filepath);
		static void DestroyModel(const Model& model);

		void AddMesh(Mesh& mesh);

	private:
		std::vector< Mesh> m_meshes;
	};
}
