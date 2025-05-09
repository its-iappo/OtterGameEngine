#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include "LoggerMacros.h"

namespace OtterEngine {
	class EngineCore;

	class Logger {
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

		static void Init();
		friend class EngineCore;

	public:
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	};
}
