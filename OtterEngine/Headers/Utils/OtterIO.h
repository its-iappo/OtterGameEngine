#pragma once

#include <string>
#include <vector>

namespace OtterEngine {

	class OtterIO {

	public:
		static std::vector<char> ReadFile(const std::string& fileName);
	};
}