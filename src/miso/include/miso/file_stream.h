#ifndef MISO_FILE_STREAM_H_
#define MISO_FILE_STREAM_H_

#include <stdio.h>
#include <cstdint>

#include "miso/buffer.h"
#include "miso/stream.h"

#ifndef MISO_FILESTREAM_BUFFER_SIZE
#define MISO_FILESTREAM_BUFFER_SIZE 256
#endif // MISO_FILESTREAM_BUFFER_SIZE

namespace miso {

class FileStream : public IStream {
public:
    template<typename TAllocator = std::allocator<uint8_t>>
    static Buffer<TAllocator> ReadAll(const char *filename, TAllocator &allocator = TAllocator());

    explicit FileStream(const char *filename);
    FileStream(FileStream&& other) noexcept;
    ~FileStream() { if (fp_ != 0) fclose(fp_); }

    bool CanRead(size_t size = 1) const;
    uint8_t Read();
    uint8_t Peek() const { return *current_; }
    size_t ReadBlock(uint8_t* buffer, size_t size);
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position);

private:
    static size_t GetStreamSize(FILE *fp);

    explicit FileStream(FILE* fp);

    void FillBuffer();

    const size_t buffer_size_ = MISO_FILESTREAM_BUFFER_SIZE;
    uint8_t buffer_[MISO_FILESTREAM_BUFFER_SIZE];
    FILE *fp_;
    size_t stream_size_;
    size_t offset_;
    uint8_t *current_;
    uint8_t *begin_;
    uint8_t *end_;
    bool reached_to_end_;
};

template<typename TAllocator>
inline Buffer<TAllocator>
FileStream::ReadAll(const char *filename, TAllocator &allocator)
{
    FILE *fp = fopen(filename, "rb");
    auto size = GetStreamSize(fp);
    Buffer<TAllocator> buffer(size, allocator);
    if (size > 0 && buffer != nullptr) { fread(buffer, 1, size, fp); }
    if (fp != nullptr) fclose(fp);
    return buffer;
}

inline
FileStream::FileStream(const char *filename) :
    FileStream(fopen(filename, "rb"))
{
    if (fp_ != 0) FillBuffer();
}

inline
FileStream::FileStream(FileStream&& other) noexcept :
    FileStream(other.fp_)
{
    other.fp_ = 0;
    other.stream_size_ = 0;
}

inline
FileStream::FileStream(FILE* fp) :
    fp_(fp),
    stream_size_(GetStreamSize(fp)),
    offset_(0),
    current_(buffer_),
    begin_(buffer_),
    end_(nullptr),
    reached_to_end_(false)
{}

inline bool
FileStream::CanRead(size_t size) const
{
    return fp_ != nullptr &&
        (offset_ + static_cast<size_t>((current_ - begin_)) + size) <= stream_size_;
}

inline uint8_t
FileStream::Read()
{
    auto one = *current_++;
    FillBuffer();
    return one;
}

inline size_t
FileStream::ReadBlock(uint8_t* buffer, size_t size)
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

inline void
FileStream::SetPosition(size_t position)
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

inline size_t
FileStream::GetStreamSize(FILE *fp)
{
    if (fp == nullptr) return 0;
    fseek(fp, 0, SEEK_END);
    auto end = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto begin = ftell(fp);
    return (begin != -1L && end != -1L) ? static_cast<size_t>(end - begin) : 0;
}

inline void
FileStream::FillBuffer()
{
    if (current_ < end_ || reached_to_end_) return;
    size_t read_count = fread(buffer_, 1, buffer_size_, fp_);
    if (end_ != nullptr) offset_ += buffer_size_;
    current_ = buffer_;
    end_ = buffer_ + read_count;
    reached_to_end_ = (read_count < buffer_size_);
}

} // namespace miso

#endif // MISO_FILE_STREAM_H_
