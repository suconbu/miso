#ifndef MISO_CORE_STREAM_H_
#define MISO_CORE_STREAM_H_

#include "miso/buffer.h"

#include <stdio.h>

namespace miso {

class IStream {
public:
    using Type = MISO_BYTE_TYPE;

    virtual ~IStream() {}

    virtual bool CanRead(size_t size = 1) const = 0;
    virtual Type Read() = 0;
    virtual Type Peek() const = 0;
    virtual size_t ReadBlock(void *buffer, size_t size) = 0;
    virtual size_t GetSize() const = 0;
    virtual size_t GetPosition() const = 0;
    virtual void SetPosition(size_t position) = 0;

protected:
    IStream() = default;
};

class MemoryStream : public IStream {
public:
    using Type = MISO_BYTE_TYPE;

    MemoryStream(const void *memory, size_t size)
        : current_(static_cast<const Type *>(memory)), begin_(static_cast<const Type *>(memory)),
        end_(static_cast<const Type *>(memory) + size)
    {}
    MemoryStream(MemoryStream&&) = default;

    bool CanRead(size_t size = 1) const { return begin_ != nullptr && (current_ + size) <= end_; }
    Type Read() { return (current_ < end_) ? *current_++ : *(end_ - 1); }
    Type Peek() const { return (current_ < end_) ? *current_ : *(end_ - 1); }
    size_t ReadBlock(void *buffer, size_t size)
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
    size_t GetSize() const { return static_cast<size_t>(end_ - begin_); }
    size_t GetPosition() const { return static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position)
    {
        current_ = ((begin_ + position) < end_) ? (begin_ + position) : end_;
    }

private:
    const Type *current_;
    const Type *begin_;
    const Type *end_;
};

class FileStream : public IStream {
public:
    using Type = MISO_BYTE_TYPE;

    explicit FileStream(const char *filename) : FileStream(fopen(filename, "rb"))
    {
        if (fp_ != 0) FillBuffer();
    }
    FileStream(FileStream&& other) : FileStream(other.fp_)
    {
        other.fp_ = 0;
        other.stream_size_ = 0;
    }
    ~FileStream() { if (fp_ != 0) fclose(fp_); }

    bool CanRead(size_t size = 1) const
    {
        return fp_ != nullptr &&
            (offset_ + static_cast<size_t>((current_ - begin_)) + size) <= stream_size_;
    }
    Type Read()
    {
        auto one = *current_++;
        FillBuffer();
        return one;
    }
    Type Peek() const { return *current_; }
    size_t ReadBlock(void *buffer, size_t size)
    {
        size_t remain = size;
        do {
            size_t copy_size = remain;
            size_t current_to_end = static_cast<size_t>(end_ - current_);
            if (current_to_end < copy_size) { copy_size = current_to_end; }
            memcpy(buffer, current_, copy_size);
            current_ += copy_size;
            FillBuffer();
            remain -= copy_size;
        } while (0 < remain && !reached_to_end_);
        return size - remain;
    }
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position)
    {
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

    static size_t GetStreamSize(FILE *fp)
    {
        if (fp == nullptr) return 0;
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto begin = ftell(fp);
        return (begin != -1L && end != -1L) ? static_cast<size_t>(end - begin) : 0;
    }
    template <typename TAllocator = std::allocator<MISO_BYTE_TYPE>>
    static Buffer<TAllocator> ReadAll(const char *filename, TAllocator &allocator = TAllocator())
    {
        FILE *fp = fopen(filename, "rb");
        auto size = GetStreamSize(fp);
        Buffer<TAllocator> buffer(size, allocator);
        if (size > 0 && buffer != nullptr) { fread(buffer, 1, size, fp); }
        if (fp != nullptr) fclose(fp);
        return buffer;
    }

private:
    const size_t buffer_size_ = MISO_CORE_FILESTREAM_BUFFER_SIZE;
    Type buffer_[MISO_CORE_FILESTREAM_BUFFER_SIZE];
    FILE *fp_;
    size_t stream_size_;
    size_t offset_;
    Type *current_;
    Type *begin_;
    Type *end_;
    bool reached_to_end_;

    FileStream(FILE* fp) :
        fp_(fp), stream_size_(GetStreamSize(fp)), offset_(0),
        current_(buffer_), begin_(buffer_), end_(nullptr), reached_to_end_(false)
    {}

    void FillBuffer()
    {
        if (current_ < end_ || reached_to_end_) return;
        size_t read_count = fread(buffer_, 1, buffer_size_, fp_);
        if (end_ != nullptr) offset_ += buffer_size_;
        current_ = buffer_;
        end_ = buffer_ + read_count;
        reached_to_end_ = (read_count < buffer_size_);
    }
};

} // namespace miso

#endif // MISO_CORE_STREAM_H_
