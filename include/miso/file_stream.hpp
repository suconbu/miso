#ifndef MISO_FILE_STREAM_HPP_
#define MISO_FILE_STREAM_HPP_

#include "miso/common.h"

#include "miso/buffer.hpp"
#include "miso/stream.hpp"

namespace miso {

class FileStream : public IStream {
public:
    template<typename TAllocator = std::allocator<uint8_t>>
    static Buffer<TAllocator> ReadAll(const char *filename, TAllocator &allocator = TAllocator());

    explicit FileStream(const char *filename);
    FileStream(FileStream&& other) noexcept;
    ~FileStream();

    bool CanRead(size_t size = 1) const;
    uint8_t Read();
    uint8_t Peek() const { return *current_; }
    size_t ReadBlock(uint8_t* buffer, size_t size);
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position);

private:
    static size_t GetStreamSize(FILE *fp);

    static constexpr size_t kBufferSize = 256;

    explicit FileStream(FILE* fp);

    void FillBuffer();

    uint8_t buffer_[kBufferSize];
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

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "file_stream.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_FILE_STREAM_HPP_
