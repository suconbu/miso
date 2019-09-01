#ifndef MISO_FILE_STREAM_HPP_
#define MISO_FILE_STREAM_HPP_

#include "miso/common.hpp"

#include "miso/buffer.hpp"
#include "miso/stream.hpp"

namespace miso {

class FileStream : public IStream {
public:
    template<typename TAllocator = std::allocator<uint8_t>>
    static Buffer<TAllocator> ReadAll(const char *filename, TAllocator &allocator = TAllocator());

    FileStream() = delete;
    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;
    FileStream(FileStream&& other) noexcept;
    FileStream& operator=(FileStream&&) = delete;
    explicit FileStream(const char *filename);
    ~FileStream();

    bool CanRead(size_t size = 1) const;
    uint8_t Read();
    uint8_t Peek() const { return *current_; }
    size_t ReadBlock(uint8_t* buffer, size_t size);
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position);

private:
    static constexpr size_t kBufferSize = 256;

    explicit FileStream(FILE* fp);

    static size_t GetStreamSize(FILE *fp);
    void FillBuffer();

    uint8_t buffer_[kBufferSize] = {};
    FILE* fp_ = nullptr;
    size_t stream_size_ = 0;
    size_t offset_ = 0;
    uint8_t* current_ = nullptr;
    uint8_t* begin_ = nullptr;
    uint8_t* end_ = nullptr;
    bool reached_to_end_ = false;
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
