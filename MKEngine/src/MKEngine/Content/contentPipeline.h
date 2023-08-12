#pragma once
#include "mkpch.h"

namespace MKEngine {
	class ContentPipeline {
	public:
	private:
		static std::vector<char> readFile(const std::string& filePath);
	};
}