#include "OtterPCH.h"

#include <thread>
#include <cstdlib>
#include <iostream>

#include "imgui.h"

#include "Core/OtterCrashHandler.h"

namespace OtterEngine {
	void OtterCrashReporter::Init(bool enableDetailWindow)
	{
		sWindowEnabled = enableDetailWindow;
		sCrashed = false;
		OTTER_CORE_LOG("[OTTER CRASH HANDLER] Initialized! Detail window enabled: {}", enableDetailWindow);
	}

	void OtterCrashReporter::Report(const char* condition, const char* message, const char* file, int line)
	{
		// Avoid infinite loop
		if (sCrashed)
			return;

		sCrashed = true;

		sCrashInfo.cond = condition ? condition : "???";
		sCrashInfo.msg = message ? message : "---";
		sCrashInfo.file = file ? file : "---";
		sCrashInfo.line = line;
		sCrashInfo.stackTrace = "Stack trace temporarily unavailable"; // TODO implement

		OTTER_CORE_CRITICAL("[OTTER CRASH HANDLER]\nCondition: {}\nMessage: {}\nFile: {}\nLine: {}",
			sCrashInfo.cond, sCrashInfo.msg, sCrashInfo.file, sCrashInfo.line);

		std::cerr << "\n==== OTTER CRASH REPORT ====\n"
			<< "Condition: " << sCrashInfo.cond << "\n"
			<< "Message:   " << sCrashInfo.msg << "\n"
			<< "File:      " << sCrashInfo.file << "\n"
			<< "Line:      " << sCrashInfo.line << "\n"
			<< "============================\n";

		NotifyListeners(sCrashInfo);

		if (sWindowEnabled)
			ShowWindow();
	}

	void OtterCrashReporter::ShowWindow()
	{
		// Fallback console-based crash window
		std::cout << "\n=== OTTER CRASH HANDLER ===\n";
		std::cout << "A fatal error has occurred.\n\n";
		std::cout << "Condition: " << sCrashInfo.cond << "\n";
		std::cout << "Message:   " << sCrashInfo.msg << "\n";
		std::cout << "File:      " << sCrashInfo.file << "\n";
		std::cout << "Line:      " << sCrashInfo.line << "\n";
		std::cout << "===========================\n";

		std::cout << "Press Enter to exit...\n";
		std::cin.get();

		std::exit(EXIT_FAILURE);
	}

	void OtterCrashReporter::ShowDetailedCrashWindow()
	{
		ImGui::Begin("Otter Crash Reporter", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "Fatal Error!");
		ImGui::Separator();
		ImGui::Text("Condition: %s", sCrashInfo.cond.c_str());
		ImGui::Text("Message:   %s", sCrashInfo.msg.c_str());
		ImGui::Text("File:      %s", sCrashInfo.file.c_str());
		ImGui::Text("Line:      %d", sCrashInfo.line);
		ImGui::Separator();
		ImGui::TextWrapped("Stack trace: %s", sCrashInfo.stackTrace.c_str());
		if (ImGui::Button("Exit"))
			std::exit(EXIT_FAILURE);
		ImGui::End();
	}

	void OtterCrashReporter::RegisterListener(CrashCallback cbk)
	{
		std::scoped_lock lock(sLock);
		sListeners.push_back(std::move(cbk));

	}

	void OtterCrashReporter::NotifyListeners(const CrashInfo& info)
	{
		std::scoped_lock lock(sLock);
		for (auto& cbk : sListeners)
		{
			if (cbk) {
				cbk(info);
			}
		}
	}

	bool OtterCrashReporter::HasCrashed()
	{
		std::scoped_lock lock(sLock);
		return sCrashed;
	}

	const CrashInfo& OtterCrashReporter::GetLastCrashInfo()
	{
		std::scoped_lock lock(sLock);
		return sCrashInfo;
	}
}