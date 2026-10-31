#include "buffer.h"

#include <algorithm>

namespace clessidra
{
void Buffer::Init(float* mem, size_t length)
{
    mem_    = mem;
    length_ = length;
    std::fill(mem_, mem_ + length_, 0.0f);
}

} // namespace clessidra
