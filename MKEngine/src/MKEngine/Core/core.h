#pragma once



namespace MKEngine {
	template<typename T>
	using UniquePointer = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr UniquePointer<T> CreateUniquePointer(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using SharedPointer = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr SharedPointer<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


}

#define BIT(x) (1 << x)

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
