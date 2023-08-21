#pragma once
#include "vertex.h"

namespace MKEngine
{
	class Mesh
	{
	public:
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		Mesh() = default;

		void Apply();
	private:
		
	};
}
