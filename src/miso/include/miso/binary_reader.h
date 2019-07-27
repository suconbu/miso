#ifndef MISO_CORE_BINARYREADER_H_
#define MISO_CORE_BINARYREADER_H_

#include "miso/endian.h"

#include <stdio.h>
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

class IStream {
public:
    using Type = MISO_BYTE_TYPE;

    virtual ~IStream() {}

    virtual bool CanRead(size_t size) const = 0;
    virtual Type Read() = 0;
    virtual Type Peek() const = 0;
    virtual size_t ReadBlock(void* buffer, size_t size) = 0;
    virtual size_t GetSize() const = 0;
    virtual size_t GetPosition() const = 0;
    virtual void SetPosition(size_t position) = 0;
};

class MemoryStream : public IStream {
public:
    using Type = MISO_BYTE_TYPE;

    explicit MemoryStream(const void* memory, size_t size) :
        current_(static_cast<const Type*>(memory)), begin_(static_cast<const Type*>(memory)), end_(static_cast<const Type*>(memory) + size) {}

    bool CanRead(size_t size = 1) const { return begin_ != nullptr && (current_ + size) <= end_; }
    Type Read() { return (current_ < end_) ? *current_++ : *(end_ - 1); }
    Type Peek() const { return (current_ < end_) ? *current_ : *(end_ - 1); }
    size_t ReadBlock(void* buffer, size_t size) {
        size_t actual_size = 0;
        if (current_ < end_) {
            size_t remain = static_cast<size_t>(end_ - current_);
            actual_size = (size < remain) ? size : remain;
            memcpy(buffer, current_, actual_size);
            current_ += actual_size;
        }
        return actual_size;
    }
    size_t GetSize() const { return static_cast<size_t>(end_ - begin_); }
    size_t GetPosition() const { return static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) { current_ = ((begin_ + position) < end_) ? (begin_ + position) : end_; }

private:
    const Type* current_;
    const Type* begin_;
    const Type* end_;
};

class FileStream : public IStream {
public:
    using Type = MISO_BYTE_TYPE;

    explicit FileStream(const char* filename) :
        fp_(fopen(filename, "rb")), stream_size_(GetStreamSize(fp_)), offset_(0),
        current_(buffer_), begin_(buffer_), end_(nullptr), reached_to_end_(false)
    { if (fp_ != nullptr) FillBuffer(); }
    ~FileStream() { if (fp_ != nullptr) fclose(fp_); }

    bool CanRead(size_t size = 1) const { return fp_ != nullptr && (offset_ + static_cast<size_t>((current_ - begin_)) + size) <= stream_size_; }
    Type Read() { auto one = *current_++; FillBuffer(); return one; }
    Type Peek() const { return *current_; }
    size_t ReadBlock(void* buffer, size_t size) {
        size_t remain = size;
        do {
            size_t copy_size = remain;
            size_t current_to_end = static_cast<size_t>(end_ - current_);
            if (current_to_end < copy_size) {
                copy_size = current_to_end;
            }
            memcpy(buffer, current_, copy_size);
            current_ += copy_size;
            FillBuffer();
            remain -= copy_size;
        } while (0 < remain && !reached_to_end_);
        return size - remain;
    }
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) {
        if (offset_ <= position && position <= buffer_size_) {
            current_ = begin_ + (position - offset_);
        } else {
            reached_to_end_ = false;
            fseek(fp_, static_cast<long>(position), SEEK_SET);
            offset_ = position;
            end_ = nullptr;
            FillBuffer();
        }
    }

    static size_t GetStreamSize(FILE* fp) {
        if (fp == nullptr) return 0;
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto begin = ftell(fp);
        return (begin != -1L && end != -1L) ? static_cast<size_t>(end - begin) : 0;
    }
    template<typename TAllocator = std::allocator<MISO_BYTE_TYPE>>
    static Buffer<TAllocator> ReadAll(const char* filename, TAllocator& allocator = TAllocator()) {
        FILE* fp = fopen(filename, "rb");
        auto size = GetStreamSize(fp);
        Buffer<TAllocator> buffer(size, allocator);
        if (size > 0 && buffer != nullptr) {
            fread(buffer, 1, size, fp);
        }
        if (fp != nullptr) fclose(fp);
        return buffer;
    }

private:
    const size_t buffer_size_ = MISO_CORE_FILESTREAM_BUFFER_SIZE;
    Type buffer_[MISO_CORE_FILESTREAM_BUFFER_SIZE];
    FILE* fp_;
    size_t stream_size_;
    size_t offset_;
    Type* current_;
    Type* begin_;
    Type* end_;
    bool reached_to_end_;

    void FillBuffer() {
        if (current_ < end_ || reached_to_end_) return;
        size_t read_count = fread(buffer_, 1, buffer_size_, fp_);
        if (end_ != nullptr) offset_ += buffer_size_;
        current_ = buffer_;
        end_ = buffer_ + read_count;
        reached_to_end_ = (read_count < buffer_size_);
    }
};

class BinaryReader
{
public:
    explicit BinaryReader(const char* filename, Endian endian = Endian::Native) :
        reader_(new FileStream(filename)),
        native_endian_(EndianUtils::GetNativeEndian()),
        target_endian_(endian == Endian::Native ? native_endian_ : endian)
    {}
    explicit BinaryReader(const void* buffer, size_t size, Endian endian = Endian::Native) :
        reader_(new MemoryStream(static_cast<const char*>(buffer), size)),
        native_endian_(EndianUtils::GetNativeEndian()),
        target_endian_(endian == Endian::Native ? native_endian_ : endian)
    {}
    ~BinaryReader() { delete reader_; }

    bool CanRead(size_t size = 1) const { return reader_ != nullptr && reader_->CanRead(size); }
    size_t GetSize() const { return reader_->GetSize(); }
    Endian GetEndian() const { return target_endian_; }
    void SetEndian(Endian endian) { target_endian_ = endian; }
    size_t GetPosition() const { return reader_->GetPosition(); }
    void SetPosition(size_t position) { reader_->SetPosition(position); }
    template<typename T> T Read(T default_value = 0) { return ReadStream(default_value, true); }
    template<typename T> T Peek(T default_value = 0) { return ReadStream(default_value, false); }
    template<typename TAllocator = std::allocator<MISO_BYTE_TYPE>>
    Buffer<TAllocator> ReadBlock(size_t size) {
        if (!CanRead()) return Buffer<TAllocator>();
        Buffer<TAllocator> buffer(size);
        buffer.Resize(reader_->ReadBlock(buffer, size));
        return buffer;
    }
    size_t ReadBlock(void* buffer, size_t size) { return CanRead() ? reader_->ReadBlock(static_cast<char*>(buffer), size) : 0; }

private:
    Endian native_endian_;
    Endian target_endian_;
    IStream* reader_;

    template<typename T> T ReadStream(T default_value, bool advance) {
        if (!CanRead()) return default_value;
        auto read_size = sizeof(T);
        auto position = reader_->GetPosition();
        T v;
        auto actual_size = reader_->ReadBlock(reinterpret_cast<char*>(std::addressof(v)), read_size);
        if (!advance) {
            reader_->SetPosition(position);
        }
        if (actual_size < read_size) return default_value;
        return (target_endian_ != native_endian_) ? EndianUtils::Flip(v) : v;
    }
};

} // namespace miso

#endif // MISO_CORE_BINARYREADER_H_
