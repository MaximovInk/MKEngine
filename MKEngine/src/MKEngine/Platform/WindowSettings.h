#pragma once
#include "mkpch.h"

namespace MKEngine {
	struct WindowSettings {
	public:
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool resizable;

		WindowSettings(
			std::string title = "MKEngine",
			uint32_t width = 640,
			uint32_t height = 480
		)
			: Title(title), Width(width), Height(height)
		{}
	};
}