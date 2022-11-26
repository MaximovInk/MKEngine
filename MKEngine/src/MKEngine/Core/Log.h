#pragma once
#include "core.h"

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
		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
#endif
	};

}

#define MK_LOG_INIT() MKEngine::Log::Initialize()
#define MK_LOG_SPACE() MKEngine::Log::Space()

#ifdef MK_ENABLE_LOG
	#ifdef MK_ENGINE
		#define MK_LOG_TRACE(...) MKEngine::Log::getCoreLogger()->trace(__VA_ARGS__)
		#define MK_LOG_INFO(...)  MKEngine::Log::getCoreLogger()->info(__VA_ARGS__)
		#define MK_LOG_DEBUG(...)  MKEngine::Log::getCoreLogger()->debug(__VA_ARGS__)
		#define MK_LOG_WARN(...)  MKEngine::Log::getCoreLogger()->warn(__VA_ARGS__)
		#define MK_LOG_ERROR(...) MKEngine::Log::getCoreLogger()->error(__VA_ARGS__)
		#define MK_LOG_CRITICAL(...) MKEngine::Log::getCoreLogger()->critical(__VA_ARGS__)
	#else
		#define MK_LOG_TRACE(...) MKEngine::Log::getClientLogger()->trace(__VA_ARGS__)
		#define MK_LOG_INFO(...)  ::MKEngine::Log::getClientLogger()->info(__VA_ARGS__)
		#define MK_LOG_DEBUG(...)  MKEngine::Log::getClientLogger()->debug(__VA_ARGS__)
		#define MK_LOG_WARN(...)  MKEngine::Log::getClientLogger()->warn(__VA_ARGS__)
		#define MK_LOG_ERROR(...) MKEngine::Log::getClientLogger()->error(__VA_ARGS__)
		#define MK_LOG_CRITICAL(...) MKEngine::Log::getClientLogger()->critical(__VA_ARGS__)
	#endif 
#else // ifndef MK_ENABLE_LOG

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