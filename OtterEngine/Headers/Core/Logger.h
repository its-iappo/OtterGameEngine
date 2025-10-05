#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace OtterEngine {
	class EngineCore;

	class Logger {
	private:
		static std::shared_ptr<spdlog::logger> sCoreLogger;
		static std::shared_ptr<spdlog::logger> sClientLogger;

		static void Init();
		friend class EngineCore;

	public:
		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return sCoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return sClientLogger; }
	};
}

#define OTTER_CORE_LOG(...)			::OtterEngine::Logger::getCoreLogger()->info(__VA_ARGS__);
#define OTTER_CORE_WARNING(...)		::OtterEngine::Logger::getCoreLogger()->warn(__VA_ARGS__);
#define OTTER_CORE_ERROR(...)		::OtterEngine::Logger::getCoreLogger()->error(__VA_ARGS__);
#define OTTER_CORE_CRITICAL(...)	::OtterEngine::Logger::getCoreLogger()->critical(__VA_ARGS__);
#define OTTER_CORE_EXCEPT(...)		\
									do { \
									    ::OtterEngine::Logger::getCoreLogger()->critical(__VA_ARGS__); \
									    throw std::runtime_error(fmt::format(__VA_ARGS__)); \
									} while(0)
#define OTTER_CORE_CRASH(...)		::OtterEngine::Logger::getCoreLogger()->critical(__VA_ARGS__); \
									return EXIT_FAILURE;

#define OTTER_CLIENT_LOG(...)		::OtterEngine::Logger::getClientLogger()->info(__VA_ARGS__);
#define OTTER_CLIENT_WARNING(...)	::OtterEngine::Logger::getClientLogger()->warn(__VA_ARGS__);
#define OTTER_CLIENT_ERROR(...)		::OtterEngine::Logger::getClientLogger()->error(__VA_ARGS__);
#define OTTER_CLIENT_CRITICAL(...)	::OtterEngine::Logger::getClientLogger()->critical(__VA_ARGS__);
#define OTTER_CLIENT_EXCEPT(...)	\
									do { \
									    ::OtterEngine::Logger::getClientLogger()->critical(__VA_ARGS__); \
									    throw std::runtime_error(fmt::format(__VA_ARGS__)); \
									} while(0)
#define OTTER_CLIENT_CRASH(...)		::OtterEngine::Logger::getClientLogger()->critical(__VA_ARGS__); \
									return EXIT_FAILURE;