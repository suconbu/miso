#include "miso/file_stream.hpp"

#include <stdio.h>

#include "miso/buffer.hpp"
#include "miso/stream.hpp"

namespace miso {

MISO_INLINE
FileStream::FileStream(const char *filename) :
    FileStream(fopen(filename, "rb"))
{
    if (fp_ != 0) FillBuffer();
}

MISO_INLINE
FileStream::FileStream(FileStream&& other) noexcept :
    FileStream(other.fp_)
{
    other.fp_ = 0;
    other.stream_size_ = 0;
}

MISO_INLINE
FileStream::FileStream(FILE* fp) :
    fp_(fp),
    stream_size_(GetStreamSize(fp)),
    offset_(0),
    current_(buffer_),
    begin_(buffer_),
    end_(nullptr),
    reached_to_end_(false)
{}

MISO_INLINE
FileStream::~FileStream()
{
    if (fp_ != nullptr) {
        fclose(fp_);
    }
}

MISO_INLINE bool
FileStream::CanRead(size_t size) const
{
    return fp_ != nullptr &&
        (offset_ + static_cast<size_t>((current_ - begin_)) + size) <= stream_size_;
}

MISO_INLINE uint8_t
FileStream::Read()
{
    auto one = *current_++;
    FillBuffer();
    return one;
}

MISO_INLINE size_t
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

MISO_INLINE void
FileStream::SetPosition(size_t position)
{
    if (offset_ <= position && position <= kBufferSize) {
        current_ = begin_ + (position - offset_);
    } else {
        reached_to_end_ = false;
        fseek(fp_, static_cast<long>(position), SEEK_SET);
        offset_ = position;
        end_ = nullptr;
        FillBuffer();
    }
}

MISO_INLINE size_t
FileStream::GetStreamSize(FILE *fp)
{
    if (fp == nullptr) return 0;
    fseek(fp, 0, SEEK_END);
    auto end = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto begin = ftell(fp);
    return (begin != -1L && end != -1L) ? static_cast<size_t>(end - begin) : 0;
}

MISO_INLINE void
FileStream::FillBuffer()
{
    if (current_ < end_ || reached_to_end_) return;
    size_t read_count = fread(buffer_, 1, kBufferSize, fp_);
    if (end_ != nullptr) offset_ += kBufferSize;
    current_ = buffer_;
    end_ = buffer_ + read_count;
    reached_to_end_ = (read_count < kBufferSize);
}

} // namespace miso
