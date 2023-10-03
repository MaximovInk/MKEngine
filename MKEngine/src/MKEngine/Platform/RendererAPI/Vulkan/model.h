#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "DescriptorSet/descriptorSet.h"

namespace MKEngine
{
	class Model {
	public:
		Model() = default;
		void Draw(VkCommandBuffer commandBuffer) const;
		static Model LoadModel(const char* filepath);
		static void DestroyModel(const Model& model);

		void AddMesh(Mesh& mesh);
		void AddTexture(Texture& texture);

		DescriptorSet CreateDescriptorSet();
	private:
		DescriptorSetLayout m_descriptorSetLayout;

		std::vector< Mesh> m_meshes;
		std::vector<Texture> m_textures;

		
	};
}
