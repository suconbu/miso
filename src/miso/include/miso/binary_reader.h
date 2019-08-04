#ifndef MISO_BINARY_READER_H_
#define MISO_BINARY_READER_H_

#include "miso/buffer.h"
#include "miso/endian.h"
#include "miso/stream.h"

namespace miso {

class BinaryReader {
public:
    BinaryReader(const char* filename, Endian endian = Endian::Native) :
        BinaryReader(new FileStream(filename), endian)
    {}
    BinaryReader(const void* buffer, size_t size, Endian endian = Endian::Native) :
        BinaryReader(new MemoryStream(static_cast<const char*>(buffer), size), endian)
    {}
    BinaryReader(BinaryReader&& other) : BinaryReader(other.stream_, other.target_endian_)
    {
        other.stream_ = nullptr;
    }
    ~BinaryReader() { delete stream_; }

    bool CanRead(size_t size = 1) const { return stream_ != nullptr && stream_->CanRead(size); }
    size_t GetSize() const { return stream_->GetSize(); }
    Endian GetEndian() const { return target_endian_; }
    void SetEndian(Endian endian) { target_endian_ = endian; }
    size_t GetPosition() const { return stream_->GetPosition(); }
    void SetPosition(size_t position) { stream_->SetPosition(position); }
    template<typename T> T Read(T default_value = 0) { return ReadStream(default_value, true); }
    template<typename T> T Peek(T default_value = 0) { return ReadStream(default_value, false); }
    template<typename TAllocator = std::allocator<MISO_BYTE_TYPE>>
    Buffer<TAllocator> ReadBlock(size_t size)
    {
        if (!CanRead()) return Buffer<TAllocator>();
        Buffer<TAllocator> buffer(size);
        buffer.Resize(stream_->ReadBlock(buffer, size));
        return buffer;
    }
    size_t ReadBlock(void* buffer, size_t size) { return CanRead() ? stream_->ReadBlock(static_cast<char*>(buffer), size) : 0; }

private:
    Endian native_endian_;
    Endian target_endian_;
    IStream* stream_;

    BinaryReader(IStream* stream, Endian endian = Endian::Native) :
        stream_(stream),
        native_endian_(EndianUtils::GetNativeEndian()),
        target_endian_(endian == Endian::Native ? native_endian_ : endian)
    {}

    template<typename T> T ReadStream(T default_value, bool advance)
    {
        if (!CanRead()) return default_value;
        auto read_size = sizeof(T);
        auto position = stream_->GetPosition();
        T v;
        auto actual_size = stream_->ReadBlock(reinterpret_cast<char*>(std::addressof(v)), read_size);
        if (!advance) {
            stream_->SetPosition(position);
        }
        if (actual_size < read_size) return default_value;
        return (target_endian_ != native_endian_) ? EndianUtils::Flip(v) : v;
    }
};

} // namespace miso

#endif // MISO_BINARY_READER_H_
