#pragma once

#include <cstddef>

namespace OtterEngine {

	/// <summary>
	/// Avoid using RTTI by creating an unique ID for the type T
	/// </summary>
	/// <returns>The unique id of the type T</returns>
	template<typename T>
	constexpr std::size_t GetTypeID() noexcept {
		static const char id{}; // Unique area of memory for type T
		return reinterpret_cast<std::size_t>(&id); // Integer representation of id mem address
	}
}