#ifndef MISO_CORE_BUFFER_H_
#define MISO_CORE_BUFFER_H_

#include <memory.h>

namespace miso {

template<typename TAllocator = std::allocator<MISO_BYTE_TYPE>>
class Buffer {
public:
    using Type = MISO_BYTE_TYPE;

    explicit Buffer(size_t size = 0, TAllocator& allocator = TAllocator()) :
        allocator_(allocator), buffer_((0 < size) ? allocator.allocate(size) : nullptr), buffer_size_(size), used_size_(size) {}
    explicit Buffer(const Type* source, size_t size, TAllocator& allocator = TAllocator()) : Buffer(size, allocator) {
        memcpy(buffer_, source, sizeof(Type) * size);
    }
    Buffer(const Buffer<TAllocator>& other) : Buffer(other, other.buffer_size_, other.allocator_) {}
    Buffer(Buffer<TAllocator>&& other) :
        allocator_(other.allocator_), buffer_(other.buffer_), buffer_size_(other.buffer_size_), used_size_(other.used_size_) {
        other.buffer_ = nullptr;
        other.buffer_size_ = 0;
        other.used_size_ = 0;
    }
    ~Buffer() { allocator_.deallocate(buffer_, buffer_size_); }

    bool IsEmpty() { return buffer_ == nullptr; }
    size_t GetSize() const { return used_size_; }
    void Resize(size_t new_size, bool preserve_content = true) {
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
    Type* GetPointer() const { return buffer_; }
    operator Type*() const { return buffer_; }

    Buffer& operator=(const Buffer<TAllocator>&) = delete;

private:
    TAllocator& allocator_;
    Type* buffer_;
    size_t buffer_size_;
    size_t used_size_;
};

} // namespace miso

#endif // MISO_CORE_BUFFER_H_
