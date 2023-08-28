#include "mkpch.h"
#include "Mesh.h"
#include "rapidobj.hpp"
#include "MKEngine/Core/Log.h"
#include "Vulkan/VkContext.h"
#include "Vulkan/vkFunctions.h"

namespace MKEngine {

	void Mesh::Apply()
	{
		if (m_verticesBuffer.Resource != nullptr)
			Buffer::Destroy(m_verticesBuffer);
		if (m_indicesBuffer.Resource != nullptr)
			Buffer::Destroy(m_indicesBuffer);

		MK_LOG_INFO("CREATING VERTEX BUFFER");
		const auto vertexBufferSize = sizeof(VERTICES[0]) * VERTICES.size();
		BufferDescription vertexBufferDescription;
		vertexBufferDescription.Size = vertexBufferSize;
		vertexBufferDescription.Data = (void*)VERTICES.data();
		vertexBufferDescription.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferDescription.Access = DataAccess::Device;
		m_verticesBuffer = Buffer::Create(vertexBufferDescription);
		
		MK_LOG_INFO("CREATING INDICES BUFFER");
		const auto indicesBufferSize = sizeof(INDICES[0]) * INDICES.size();
		BufferDescription indicesDescription;
		indicesDescription.Size = indicesBufferSize;
		indicesDescription.Data = (void*)INDICES.data();
		indicesDescription.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indicesDescription.Access = DataAccess::Device;
		m_indicesBuffer = Buffer::Create(indicesDescription);
	}

	Mesh Mesh::LoadObjMesh(std::string filename)
	{
		Mesh mesh{};
		auto [attributes, shapes, materials, error] = ParseFile(filename, rapidobj::MaterialLibrary::Ignore());

		if (shapes.size() > 1)
			MK_LOG_WARN("Obj shapes > 1, loaded first");

		if (shapes.empty())
			MK_LOG_ERROR("Failed to loading mesh");

		size_t numTriangles{};

		for (const auto& [name, mesh, lines, points] : shapes) {
			numTriangles += mesh.num_face_vertices.size();
		}
		MK_LOG_INFO("Loaded model:{0} | Triangles: {1}", filename, numTriangles);

		const auto& indices = shapes[0].mesh.indices;

		for (size_t i = 0; i < attributes.positions.size(); i++)
		{
			Vertex vertex{};
			vertex.Position = glm::vec3(
				attributes.positions[i*3 + 0],
				attributes.positions[i*3 + 1],
				attributes.positions[i*3 + 2]
			);

			mesh.Vertices.emplace_back(vertex);
		}

		for (size_t i = 0; i < indices.size(); i++)
		{
			mesh.Indices.emplace_back(indices[i].position_index);
		}

		return mesh;
	}

	void Mesh::Draw(const VkCommandBuffer commandBuffer) const
	{

		const VkBuffer vertexBuffers[] = { m_verticesBuffer.Resource };
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indicesBuffer.Resource, 0, VK_INDEX_TYPE_UINT16);

		//vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indicesBuffer.Size()), 1, 0, 0, 0);

		const auto indicesCount = Indices.size();

		vkCmdDrawIndexed(commandBuffer, indicesCount, 1, 0, 0, 0);
	}

	void Mesh::Destroy(const Mesh& mesh)
	{
		if (mesh.m_verticesBuffer.Resource != nullptr)
			Buffer::Destroy(mesh.m_verticesBuffer);
		if (mesh.m_indicesBuffer.Resource != nullptr)
			Buffer::Destroy(mesh.m_indicesBuffer);
	}

}
