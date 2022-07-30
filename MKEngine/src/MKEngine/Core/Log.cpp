#include "mkpch.h"
#include "Log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace MKEngine {
	std::shared_ptr<spdlog::logger> Log::s_coreLogger;
	std::shared_ptr<spdlog::logger> Log::s_clientLogger;

	void Log::Initialize()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_coreLogger = spdlog::stdout_color_mt("MKEngine");
		s_coreLogger->set_level(spdlog::level::trace);

		s_clientLogger = spdlog::stdout_color_mt("App");
		s_clientLogger->set_level(spdlog::level::trace);

		MK_LOG_INFO("Log initalized!");
		MK_LOG_SPACE();
		MK_LOG_INFO("Info");
		MK_LOG_TRACE("Trace");
		MK_LOG_DEBUG("Debug");
		MK_LOG_WARN("Warn");
		MK_LOG_ERROR("Error");
		MK_LOG_CRITICAL("Critical");
	}

	void Log::Space()
	{
		std::cout << "" << std::endl;
	}
}