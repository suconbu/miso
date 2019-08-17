#include "miso/binary_reader.hpp"

#include "miso/buffer.hpp"
#include "miso/endian_utils.hpp"
#include "miso/file_stream.hpp"
#include "miso/memory_stream.hpp"

namespace miso {

MISO_INLINE
BinaryReader::BinaryReader(const char* filename, Endian endian) :
    BinaryReader(new FileStream(filename), endian)
{}

MISO_INLINE
BinaryReader::BinaryReader(const uint8_t* buffer, size_t size, Endian endian) :
    BinaryReader(new MemoryStream(buffer, size), endian)
{}

MISO_INLINE
BinaryReader::BinaryReader(BinaryReader&& other) noexcept :
    BinaryReader(other.stream_, other.target_endian_)
{
    other.stream_ = nullptr;
}

MISO_INLINE
BinaryReader::BinaryReader(IStream* stream, Endian endian) :
    stream_(stream),
    native_endian_(EndianUtils::GetNativeEndian()),
    target_endian_(endian == Endian::Native ? native_endian_ : endian)
{}

MISO_INLINE
BinaryReader::~BinaryReader()
{
    delete stream_;
}

MISO_INLINE size_t
BinaryReader::ReadBlock(void* buffer_out, size_t size)
{
    return CanRead() ? stream_->ReadBlock(static_cast<uint8_t*>(buffer_out), size) : 0;
}

} // namespace miso
