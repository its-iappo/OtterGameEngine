#include "Core/EngineCore.h"
#include "Core/Logger.h"

namespace OtterEngine {
	void EngineCore::Start()
	{
		Logger::Init();
		OTTER_CORE_LOG("EngineCore started");
		OTTER_CORE_WARNING("Development build");
	}
}
