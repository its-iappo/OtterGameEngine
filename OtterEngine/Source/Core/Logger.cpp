#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Core/Logger.h"

namespace OtterEngine {
	std::shared_ptr<spdlog::logger> Logger::sCoreLogger;
	std::shared_ptr<spdlog::logger> Logger::sClientLogger;

	void Logger::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		sCoreLogger = spdlog::stdout_color_mt("[ENGINE]");
		sClientLogger = spdlog::stdout_color_mt("[CLIENT]");
		sCoreLogger->set_level(spdlog::level::trace);
		sClientLogger->set_level(spdlog::level::trace);
	}
}
