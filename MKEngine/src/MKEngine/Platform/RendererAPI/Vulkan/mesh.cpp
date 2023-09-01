#include "mkpch.h"
#include "Mesh.h"
#include "MKEngine/Core/Log.h"
#include "VkContext.h"
#include "vkFunctions.h"

namespace MKEngine {

	void Mesh::Apply()
	{
		if (m_verticesBuffer.Resource != nullptr)
			Buffer::Destroy(m_verticesBuffer);
		if (m_indicesBuffer.Resource != nullptr)
			Buffer::Destroy(m_indicesBuffer);

		const auto vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
		BufferDescription vertexBufferDescription;
		vertexBufferDescription.Size = vertexBufferSize;
		vertexBufferDescription.Data = static_cast<void*>(Vertices.data());
		vertexBufferDescription.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferDescription.Access = DataAccess::Device;
		m_verticesBuffer = Buffer::Create(vertexBufferDescription);
		
		const auto indicesBufferSize = sizeof(Indices[0]) * Indices.size();
		BufferDescription indicesDescription;
		indicesDescription.Size = indicesBufferSize;
		indicesDescription.Data = static_cast<void*>(Indices.data());
		indicesDescription.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indicesDescription.Access = DataAccess::Device;
		m_indicesBuffer = Buffer::Create(indicesDescription);
	}

	void Mesh::Draw(const VkCommandBuffer commandBuffer) const
	{

		const VkBuffer vertexBuffers[] = { m_verticesBuffer.Resource };
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indicesBuffer.Resource, 0, VK_INDEX_TYPE_UINT16);

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
