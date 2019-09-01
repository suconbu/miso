#ifndef MISO_BUFFER_HPP_
#define MISO_BUFFER_HPP_

#include "miso/common.h"

#include <memory>

namespace miso {

template<typename TAllocator = std::allocator<uint8_t>>
class Buffer {
public:
    using Allocator = TAllocator;

    Buffer() : Buffer(0) {}
    Buffer(const Buffer<Allocator>& other);
    Buffer(Buffer<Allocator>&& other) noexcept;
    explicit Buffer(size_t size, Allocator& allocator = Allocator());
    explicit Buffer(const uint8_t* source, size_t size, Allocator& allocator = Allocator());
    ~Buffer();

    Buffer& operator=(Buffer<Allocator> other);
    operator uint8_t*() const { return buffer_; }

    bool IsEmpty() { return buffer_ == nullptr; }
    size_t GetSize() const { return used_size_; }
    void Resize(size_t new_size, bool preserve_content = true);
    uint8_t* GetPointer() const { return buffer_; }

private:
    Allocator& allocator_;
    uint8_t* buffer_ = nullptr;
    size_t buffer_size_ = 0;
    size_t used_size_ = 0;
};

template<typename TAllocator> inline
Buffer<TAllocator>::Buffer(const Buffer<Allocator>& other) :
    Buffer(other, other.buffer_size_, other.allocator_)
{}

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
    std::memcpy(buffer_, source, sizeof(uint8_t) * size);
}

template<typename TAllocator> inline
Buffer<TAllocator>::~Buffer()
{
    allocator_.deallocate(buffer_, buffer_size_);
}

template<typename TAllocator> inline Buffer<TAllocator>&
Buffer<TAllocator>::operator=(Buffer<Allocator> other)
{
    std::swap(allocator_, other.allocator_);
    std::swap(buffer_, other.buffer_);
    std::swap(buffer_size_, other.buffer_size_);
    std::swap(used_size_, other.used_size_);
    return *this;
}

template<typename TAllocator> inline void
Buffer<TAllocator>::Resize(size_t new_size, bool preserve_content)
{
    // if new size is smaller than buffer size, it will remain intact.
    if (buffer_size_ < new_size) {
        auto new_buffer_size = new_size;
        auto new_buffer = allocator_.allocate(new_buffer_size);
        if (preserve_content && buffer_ != nullptr) {
            std::memcpy(new_buffer, buffer_, buffer_size_);
        }
        allocator_.deallocate(buffer_, buffer_size_);
        buffer_ = new_buffer;
        buffer_size_ = new_buffer_size;
    }
    used_size_ = new_size;
}

} // namespace miso

#endif // MISO_BUFFER_HPP_
