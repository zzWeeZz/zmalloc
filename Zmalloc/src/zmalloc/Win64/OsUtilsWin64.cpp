#include <zmalloc/OsUtils.h>
#include <Windows.h>
namespace OSUtils
{
	std::expected<size_t, ErrorCode> GetPageSize()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);

		return static_cast<size_t>(info.dwPageSize);
	}

	std::expected<void*, ErrorCode> AllocateMemory(size_t size)
	{
		void* allocated_page = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (allocated_page == nullptr)
		{
			return std::unexpected(ErrorCode::FailedToAllocateMemory);
		}

		return allocated_page;
	}

	ErrorCode FreeMemory(void* ptr)
	{
		if (ptr == nullptr)
		{
			return ErrorCode::InvalidParameter;
		}
		if (VirtualFree(ptr, 0, MEM_RELEASE) == 0)
		{
			return ErrorCode::FailedToAllocateMemory;
		}
		return ErrorCode::Success;
	}
}