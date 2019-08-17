#include "miso/memory_stream.hpp"

#include <memory>

#include "miso/stream.hpp"

namespace miso {

MISO_INLINE size_t
MemoryStream::ReadBlock(uint8_t* buffer, size_t size)
{
    size_t actual_size = 0;
    if (current_ < end_) {
        size_t remain = static_cast<size_t>(end_ - current_);
        actual_size = (size < remain) ? size : remain;
        std::memcpy(buffer, current_, actual_size);
        current_ += actual_size;
    }
    return actual_size;
}

} // namespace miso
