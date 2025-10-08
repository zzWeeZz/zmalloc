#include <cassert>
#include <cstring>
#include <print>
#include <zmalloc/zmalloc.h>

void* operator new(size_t size)
{
    void* ptr = zmalloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}
void operator delete(void* ptr) noexcept
{
    zfree(ptr);
}

void Test_zrealloc_basic()
{
    // Allocate and fill memory
    size_t initial_size = 64;
    void* ptr = zmalloc(initial_size);
    assert(ptr != nullptr);

    std::memset(ptr, 0xCD, initial_size);

    // Reallocate to a larger size
    size_t new_size = 128;
    void* new_ptr = zrealloc(ptr, new_size);
    assert(new_ptr != nullptr);

    // Old data should be preserved
    unsigned char* bytes = static_cast<unsigned char*>(new_ptr);
    for (size_t i = 0; i < initial_size; ++i)
        assert(bytes[i] == 0xCD);

    // Reallocate to a smaller size (should not lose data)
    void* smaller_ptr = zrealloc(new_ptr, 32);
    assert(smaller_ptr != nullptr);
    unsigned char* small_bytes = static_cast<unsigned char*>(smaller_ptr);
    for (size_t i = 0; i < 32; ++i)
        assert(small_bytes[i] == 0xCD);

    zfree(smaller_ptr);

    std::println("Test_zrealloc_basic passed\n");
}

void Test_zcalloc_basic()
{
    size_t num = 50;
    size_t size = 4;
    void* ptr = zcalloc(num, size);
    assert(ptr != nullptr);

    // All bytes should be zero
    unsigned char* bytes = static_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < num * size; ++i)
        assert(bytes[i] == 0);

    zfree(ptr);
	std::println("Test_zcalloc_basic passed\n");
}

void Test_zrealloc_nullptr()
{
    // Should behave like zmalloc
    void* ptr = zrealloc(nullptr, 100);
    assert(ptr != nullptr);
    zfree(ptr);
	std::println("Test_zrealloc_nullptr passed\n");
}

void Test_zcalloc_zero()
{
    // Should not allocate memory (may assert)
    void* ptr = zcalloc(0, 10);
    assert(ptr == nullptr);
    std::println("Test_zrealloc_nullptr passed\n");
}

int main()
{
    {
        Test_zrealloc_basic();
        Test_zcalloc_basic();
        Test_zrealloc_nullptr();
        Test_zcalloc_zero();


        std::string aa;
        aa = "adfklasdfkla d adkf asdlkf asd flasdf lasdlf adls ";

    }
    CheckForMemoryLeaks();
	return 0;
}