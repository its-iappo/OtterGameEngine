#include "OtterPCH.h"

#include "Resources/Mesh.h"
#include "Resources/Texture.h"
#include "Resources/Resources.h"

#include "Core/EngineCore.h"

namespace OtterEngine {
	void EngineCore::Start()
	{
		Logger::Init();
		Resources::AddLoader<Mesh>();
		Resources::AddLoader<Texture>();

		OTTER_CORE_LOG("EngineCore started");
		OTTER_CORE_WARNING("Development build");
	}
}
