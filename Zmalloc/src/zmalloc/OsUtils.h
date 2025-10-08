#pragma once
#include <expected>

namespace OSUtils
{
	enum class ErrorCode
	{
		Success,
		FailedToGetOsInfo,
		InvalidParameter,
		FailedToAllocateMemory,
		UnknownError = 9999999,
	};

	std::expected<size_t, ErrorCode> GetPageSize();

	std::expected<void*, ErrorCode> AllocateMemory(size_t size);
	ErrorCode FreeMemory(void* ptr);
}