#ifndef MISO_MEMORY_STREAM_HPP_
#define MISO_MEMORY_STREAM_HPP_

#include "miso/common.h"

#include "miso/stream.hpp"

namespace miso {

class MemoryStream : public IStream {
public:
    MemoryStream() = delete;
    MemoryStream(const MemoryStream&) = default;
    MemoryStream& operator=(const MemoryStream&) = default;
    explicit MemoryStream(const uint8_t* memory, size_t size) : current_(memory), begin_(memory), end_(memory + size) {}

    bool CanRead(size_t size = 1) const { return begin_ != nullptr && (current_ + size) <= end_; }
    size_t GetSize() const { return static_cast<size_t>(end_ - begin_); }
    size_t GetPosition() const { return static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) { current_ = ((begin_ + position) < end_) ? (begin_ + position) : end_; }
    uint8_t Read() { return (current_ < end_) ? *current_++ : *(end_ - 1); }
    uint8_t Peek() const { return (current_ < end_) ? *current_ : *(end_ - 1); }
    size_t ReadBlock(uint8_t* buffer, size_t size);

private:
    const uint8_t *current_ = nullptr;
    const uint8_t *begin_ = nullptr;
    const uint8_t *end_ = nullptr;
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "memory_stream.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_MEMORY_STREAM_HPP_
