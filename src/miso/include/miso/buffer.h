#ifndef MISO_BUFFER_H_
#define MISO_BUFFER_H_

#include <memory.h>
#include <cstdint>
#include <memory>

namespace miso {

template<typename TAllocator = std::allocator<uint8_t>>
class Buffer {
public:
    using Allocator = TAllocator;

    explicit Buffer(size_t size = 0, Allocator& allocator = Allocator());
    Buffer(const uint8_t* source, size_t size, Allocator& allocator = Allocator());
    Buffer(const Buffer<Allocator>& other) : Buffer(other, other.buffer_size_, other.allocator_) {}
    Buffer(Buffer<Allocator>&& other) noexcept;
    ~Buffer() { allocator_.deallocate(buffer_, buffer_size_); }

    Buffer& operator=(const Buffer<Allocator>&) = delete;
    operator uint8_t*() const { return buffer_; }

    bool IsEmpty() { return buffer_ == nullptr; }
    size_t GetSize() const { return used_size_; }
    void Resize(size_t new_size, bool preserve_content = true);
    uint8_t* GetPointer() const { return buffer_; }

private:
    Allocator& allocator_;
    uint8_t* buffer_;
    size_t buffer_size_;
    size_t used_size_;
};

template<typename TAllocator> inline
Buffer<TAllocator>::Buffer(size_t size, Allocator& allocator) :
    allocator_(allocator),
    buffer_((0 < size) ? allocator.allocate(size) : nullptr),
    buffer_size_(size),
    used_size_(size)
{}

template<typename TAllocator> inline
Buffer<TAllocator>::Buffer(const uint8_t* source, size_t size, Allocator& allocator) :
    Buffer(size, allocator)
{
    memcpy(buffer_, source, sizeof(uint8_t) * size);
}

template<typename TAllocator> inline
Buffer<TAllocator>::Buffer(Buffer<Allocator>&& other) noexcept :
    allocator_(other.allocator_),
    buffer_(other.buffer_),
    buffer_size_(other.buffer_size_),
    used_size_(other.used_size_)
{
    other.buffer_ = nullptr;
    other.buffer_size_ = 0;
    other.used_size_ = 0;
}

template<typename TAllocator> inline void
Buffer<TAllocator>::Resize(size_t new_size, bool preserve_content)
{
    // if new size is smaller than buffer size, it will remain intact.
    if (buffer_size_ < new_size) {
        auto new_buffer_size = new_size;
        auto new_buffer = allocator_.allocate(new_buffer_size);
        if (preserve_content && buffer_ != nullptr) {
            memcpy(new_buffer, buffer_, buffer_size_);
        }
        allocator_.deallocate(buffer_, buffer_size_);
        buffer_ = new_buffer;
        buffer_size_ = new_buffer_size;
    }
    used_size_ = new_size;
}

} // namespace miso

#endif // MISO_BUFFER_H_
