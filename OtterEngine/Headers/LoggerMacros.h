#pragma once

#define OTTER_CORE_LOG(...) ::OtterEngine::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define OTTER_CORE_WARNING(...) ::OtterEngine::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define OTTER_CORE_ERROR(...) ::OtterEngine::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define OTTER_CLIENT_LOG(...) ::OtterEngine::Logger::GetClientLogger()->info(__VA_ARGS__)
#define OTTER_CLIENT_WARNING(...) ::OtterEngine::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define OTTER_CLIENT_ERROR(...) ::OtterEngine::Logger::GetClientLogger()->error(__VA_ARGS__)
