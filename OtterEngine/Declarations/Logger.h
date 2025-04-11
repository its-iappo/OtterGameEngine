#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace OtterEngine {
	class Logger
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define OTTER_CORE_LOG(...) ::OtterEngine::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define OTTER_CLIENT_LOG(...) ::OtterEngine::Logger::GetClientLogger()->info(__VA_ARGS__)