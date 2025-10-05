#pragma once

#include <filesystem>
#include <fmt/format.h>

/// <summary>
/// Template specialization to to instruct fmt on how to format path
/// </summary>
template <>
struct fmt::formatter<std::filesystem::path> {
	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const std::filesystem::path& path, FormatContext& ctx)
		const -> decltype(ctx.out())
	{
		// Convert path to string
		return fmt::format_to(ctx.out(), "{}", path.string());
	}
};