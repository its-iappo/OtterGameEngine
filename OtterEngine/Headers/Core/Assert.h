#pragma once

#include <string>
#include <format>
#include <cstdlib>
#include <iostream>
#include <functional>

#include "Core/Logger.h"

#ifdef _MSC_VER
#define OTTER_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define OTTER_DEBUG_BREAK() __builtin_trap()
#else
#define OTTER_DEBUG_BREAK() std::abort() // fallback to standard library
#endif

#if !defined(NDEBUG)
#define OTTER_ENABLE_ASSERTS
#endif

namespace OtterEngine {
	using AssertHandler = std::function<void(const char* cond,
		const char* msg,
		const char* file,
		int line)>;

	class Assert {
	private:
		inline static AssertHandler sHandler = nullptr;
	public:
		static void SetHandler(AssertHandler handler) noexcept { sHandler = std::move(handler); }

		static void Invoke(const char* cond, const char* msg, const char* file, int line) {
			if (sHandler)
				sHandler(cond, msg, file, line);
			else
			{
				OTTER_CORE_CRITICAL("[OTTER ASSERTION FAILED]\nCond: {}\nMsg: {}\nFile: {}\nLine: {}",
					cond, msg, file, line);
				std::cerr << "[OTTER ASSERTION FAILED] "
					<< cond << " | " << msg
					<< " (" << file << ":" << line << ")\n";
			}

			OTTER_DEBUG_BREAK();
			std::abort();
		}
	};

	template<typename... Args>
	inline std::string FormatAssertMessage(Args&&... args) {
		std::ostringstream oss;
		(oss << ... << args);
		return oss.str();
	}
}

#ifdef OTTER_ENABLE_ASSERTS

#define OTTER_ASSERT_IMPL(condition, message, ...)													\
		do {																						\
			if (!(condition)) [[unlikely]] {													    \
				std::string output = ::OtterEngine::FormatAssertMessage(__VA_ARGS__);			    \
				::OtterEngine::Assert::Invoke(#condition, output.c_str(), __FILE__, __LINE__); \
		} } while (0)

#define OTTER_ASSERT(condition, ...) OTTER_ASSERT_IMPL(condition, ##__VA_ARGS__)

#else

#define OTTER_ASSERT(condition, ...) ((void)0)

#endif