#pragma once

namespace MKEngine
{

	class Texture
	{
	public:
		uint32_t GetWidth();
		uint32_t GetHeight();

		Texture(const char* path);

	private :
		uint32_t width;
		uint32_t height;
	};

}