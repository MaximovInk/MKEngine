#pragma once

#ifdef MK_ENGINE
	#define MK_ENABLE_LOG
#endif

#ifdef MK_ENABLE_LOG
#include "spdlog/spdlog.h"
#endif

namespace MKEngine {
	class Log
	{
	public:
		static void Initialize();

		static void Space();

#ifdef MK_ENABLE_LOG
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return m_clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> m_coreLogger;
		static std::shared_ptr<spdlog::logger> m_clientLogger;
#endif
	};

}

#define MK_LOG_INIT() MKEngine::Log::Initialize()
#define MK_LOG_SPACE() MKEngine::Log::Space()

#ifdef MK_ENABLE_LOG
	#ifdef MK_ENGINE
		#define MK_LOG_TRACE(...) MKEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
		#define MK_LOG_INFO(...)  MKEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
		#define MK_LOG_DEBUG(...)  MKEngine::Log::GetCoreLogger()->debug(__VA_ARGS__)
		#define MK_LOG_WARN(...)  MKEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
		#define MK_LOG_ERROR(...) MKEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
		#define MK_LOG_CRITICAL(...) MKEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)
	#else
		#define MK_LOG_TRACE(...) MKEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
		#define MK_LOG_INFO(...)  ::MKEngine::Log::GetClientLogger()->info(__VA_ARGS__)
		#define MK_LOG_DEBUG(...)  MKEngine::Log::GetClientLogger()->debug(__VA_ARGS__)
		#define MK_LOG_WARN(...)  MKEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
		#define MK_LOG_ERROR(...) MKEngine::Log::GetClientLogger()->error(__VA_ARGS__)
		#define MK_LOG_CRITICAL(...) MKEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
	#endif 
#else

#define MK_LOG_TRACE(...) 
#define MK_LOG_INFO(...)  
#define MK_LOG_WARN(...)  
#define MK_LOG_ERROR(...) 
#define MK_LOG_CRITICAL(...)

#endif //MK_ENABLE_LOG

#ifdef MK_ENABLE_ASSERTS
	#ifdef MK_ENGINE
		#define MK_ASSERT(x,...) { if(!(x)) { MK_LOG_ERROR("Assertion failed {0}", __VA_ARGS__); __debugbreak(); } }
	#else	
		#define MK_ASSERT(x,...) { if(!(x)) { MK_LOG_ERROR("Assertion failed {0}", __VA_ARGS__); __debugbreak(); } }
	#endif
#else
#define MK_ASSERT(x,...) 
#define MK_CORE_ASSERT(x,...)
#endif

#if _DEBUG
#define VULKAN_VALIDATION 1
#elif
#define VULKAN_VALIDATION 0
#endif
