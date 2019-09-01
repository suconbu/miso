#ifndef MISO_BINARY_READER_HPP_
#define MISO_BINARY_READER_HPP_

#include "miso/common.hpp"

#include "miso/buffer.hpp"
#include "miso/stream.hpp"
#include "miso/endian_utils.hpp"

namespace miso {

class BinaryReader {
public:
    BinaryReader() = delete;
    BinaryReader(const BinaryReader&) = delete;
    BinaryReader& operator=(const BinaryReader&) = delete;
    BinaryReader(BinaryReader&& other) noexcept;
    BinaryReader& operator=(BinaryReader&&) = delete;
    explicit BinaryReader(const char* filename, Endian endian = Endian::Native);
    explicit BinaryReader(const uint8_t* buffer, size_t size, Endian endian = Endian::Native);
    ~BinaryReader();

    bool CanRead(size_t size = 1) const { return stream_ != nullptr && stream_->CanRead(size); }
    size_t GetSize() const { return stream_->GetSize(); }
    Endian GetEndian() const { return target_endian_; }
    void SetEndian(Endian endian) { target_endian_ = endian; }
    size_t GetPosition() const { return stream_->GetPosition(); }
    void SetPosition(size_t position) { stream_->SetPosition(position); }
    template<typename T> T Read(T default_value = 0) { return ReadStream(default_value, true); }
    template<typename T> T Peek(T default_value = 0) { return ReadStream(default_value, false); }
    template<typename TAllocator = std::allocator<uint8_t>> Buffer<TAllocator> ReadBlock(size_t size);
    size_t ReadBlock(void* buffer_out, size_t size);

private:
    BinaryReader(IStream* stream, Endian endian = Endian::Native);

    template<typename T> T ReadStream(T default_value, bool advance);

    Endian native_endian_ = Endian::Native;
    Endian target_endian_ = Endian::Native;
    IStream* stream_ = nullptr;
};

template<typename TAllocator>
inline Buffer<TAllocator>
BinaryReader::ReadBlock(size_t size)
{
    if (!CanRead()) return Buffer<TAllocator>();
    Buffer<TAllocator> buffer(size);
    buffer.Resize(stream_->ReadBlock(buffer, size));
    return buffer;
}

template<typename T> inline T
BinaryReader::ReadStream(T default_value, bool advance)
{
    if (!CanRead()) return default_value;
    auto read_size = sizeof(T);
    auto position = stream_->GetPosition();
    T v;
    auto actual_size = stream_->ReadBlock(reinterpret_cast<uint8_t*>(std::addressof(v)), read_size);
    if (!advance) {
        stream_->SetPosition(position);
    }
    if (actual_size < read_size) return default_value;
    return (target_endian_ != native_endian_) ? EndianUtils::Flip(v) : v;
}

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "binary_reader.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_BINARY_READER_HPP_
