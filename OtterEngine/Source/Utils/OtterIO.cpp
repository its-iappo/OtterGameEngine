#include "OtterPCH.h"

#include <fstream>

#include "Utils/OtterIO.h"

namespace OtterEngine {

	std::vector<char> OtterIO::ReadFile(const std::string& fileName) {
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);
		
		if (!file.is_open()) {
			OTTER_CORE_CRITICAL("Failed to open file: {}", fileName);
		}

		std::vector<char> buffer((size_t)file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();

		return buffer;
	}
}
