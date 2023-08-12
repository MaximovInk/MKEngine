#pragma once
#include "mkpch.h"

namespace MKEngine {
	struct WindowSettings {
	public:
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool Resizable;

		explicit WindowSettings(
			const std::string& title = "MKEngine",
			const uint32_t width = 640,
			const uint32_t height = 480
		)
			: Title(title), Width(width), Height(height), Resizable(true)
		{
		}
	};
}