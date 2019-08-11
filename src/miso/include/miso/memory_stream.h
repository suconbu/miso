#ifndef MISO_MEMORY_STREAM_H_
#define MISO_MEMORY_STREAM_H_

#include <cstdint>

#include "miso/stream.h"

namespace miso {

class MemoryStream : public IStream {
public:
    MemoryStream(const uint8_t* memory, size_t size) : current_(memory), begin_(memory), end_(memory + size) {}
    MemoryStream(MemoryStream&&) noexcept = default;

    bool CanRead(size_t size = 1) const { return begin_ != nullptr && (current_ + size) <= end_; }
    size_t GetSize() const { return static_cast<size_t>(end_ - begin_); }
    size_t GetPosition() const { return static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) { current_ = ((begin_ + position) < end_) ? (begin_ + position) : end_; }
    uint8_t Read() { return (current_ < end_) ? *current_++ : *(end_ - 1); }
    uint8_t Peek() const { return (current_ < end_) ? *current_ : *(end_ - 1); }
    size_t ReadBlock(uint8_t* buffer, size_t size);

private:
    const uint8_t *current_;
    const uint8_t *begin_;
    const uint8_t *end_;
};

inline size_t
MemoryStream::ReadBlock(uint8_t* buffer, size_t size)
{
    size_t actual_size = 0;
    if (current_ < end_) {
        size_t remain = static_cast<size_t>(end_ - current_);
        actual_size = (size < remain) ? size : remain;
        memcpy(buffer, current_, actual_size);
        current_ += actual_size;
    }
    return actual_size;
}

} // namespace miso

#endif // MISO_MEMORY_STREAM_H_
