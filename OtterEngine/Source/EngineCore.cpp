#ifdef OTTER_USE_MODULES
module OtterEngine.EngineCore;
import OtterEngine.Logger;
#else
#include "EngineCore.h"
#include "Logger.h"
#endif

#include "LoggerMacros.h"

namespace OtterEngine {
	void EngineCore::Start()
	{
		Logger::Init();
		OTTER_CORE_LOG("EngineCore started");
		OTTER_CORE_WARNING("Development build");
	}
}
