#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <functional>

namespace OtterEngine {

	struct CrashInfo {
		std::string cond;
		std::string msg;
		std::string file;
		int line{0};
		std::string stackTrace;
	};

	class OtterCrashReporter
	{
		using CrashCallback = std::function<void(const CrashInfo&)>;

	private:
		static inline bool sWindowEnabled = false;
		static inline bool sCrashed = false;
		static inline CrashInfo sCrashInfo;
		static inline std::vector<CrashCallback> sListeners;
		static inline std::mutex sLock;

		static void NotifyListeners(const CrashInfo& info);

	public:
		static bool HasCrashed();
		static const CrashInfo& GetLastCrashInfo();

		static void Init(bool enableDetailWindow = true);
		static void Report(const char* condition, const char* message, const char* file, int line);
		static void ShowWindow();
		static void ShowDetailedCrashWindow();
		static void RegisterListener(CrashCallback cbk);
	};
}