#pragma once
#include "vertex.h"
#include "Vulkan/buffer.h"

namespace MKEngine
{
	class Mesh
	{
	public:
		std::vector<Vertex> Vertices;
		std::vector<uint16_t> Indices;

		Mesh() = default;

		void Apply();
		static Mesh LoadObjMesh(std::string filename);
		void Draw(VkCommandBuffer commandBuffer) const;
		static void Destroy(const Mesh& mesh);

	private:
		Buffer m_indicesBuffer;
		Buffer m_verticesBuffer;
	};
}
