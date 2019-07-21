#include "miso/binary_reader.h"

#include "miso/endian.h"

#include <algorithm>
#include <istream>
#include <fstream>
#include <sstream>
#include <vector>

namespace miso {

BinaryReader::BinaryReader(const char* filename, Endian endian) :
    BinaryReader(new std::ifstream(filename, std::ios::binary), endian)
{
}

BinaryReader::BinaryReader(const void* buffer, size_t size, Endian endian) :
    BinaryReader(
        new std::istringstream(
            std::string(static_cast<const char*>(buffer), static_cast<const char*>(buffer) + size), 
            std::istringstream::binary),
        endian)
{
}

BinaryReader::BinaryReader(std::istream* stream, Endian endian) :
    nativeEndian_(EndianUtils::GetNativeEndian()),
    targetEndian_(endian == Endian::Native ? nativeEndian_ : endian),
    stream_pointer_(stream),
    stream_(stream_pointer_ != nullptr ? stream_pointer_->rdbuf() : nullptr),
    size_(GetLengthInside(stream_)),
    position_(0),
    overrun_(false)
{
}

BinaryReader::~BinaryReader()
{
    delete stream_pointer_;
}

void
BinaryReader::Close()
{
    stream_.set_rdbuf(nullptr);
    stream_.setstate(std::ios_base::failbit);
    delete stream_pointer_;
    stream_pointer_ = nullptr;
    size_ = 0;
    position_ = 0;
}

void
BinaryReader::SetPosition(size_t position)
{
    position_ = std::min(position, size_);
    stream_.seekg(position_, std::fstream::beg);
}

template<typename T> T
BinaryReader::Read(T defaultValue)
{
    return ReadInside(defaultValue, true);
}

template<typename T> T
BinaryReader::Peek(T defaultValue)
{
    return ReadInside(defaultValue, false);
}

std::vector<uint8_t>
BinaryReader::ReadBlock(size_t size)
{
    auto readSize = size;
    overrun_ = false;
    if (size_ < position_ + readSize)
    {
        readSize = size_ - position_;
        overrun_ = true;
    }
    std::vector<uint8_t> v(readSize);
    stream_.read(reinterpret_cast<char*>(v.data()), readSize);
    position_ += readSize;
    return v;
}

size_t
BinaryReader::ReadBlockTo(void* buffer, size_t size)
{
    auto readSize = size;
    overrun_ = false;
    if (size_ < position_ + readSize)
    {
        readSize = size_ - position_;
        overrun_ = true;
    }
    stream_.read(static_cast<char*>(buffer), readSize);
    position_ += readSize;
    return readSize;
}

size_t
BinaryReader::GetLengthInside(std::istream& stream)
{
    stream.seekg(0, std::istream::end);
    auto end = stream.tellg();
    stream.seekg(0, std::istream::beg);
    auto beg = stream.tellg();
    return static_cast<size_t>(end - beg);
}

template<typename T> T
BinaryReader::ReadInside(T defaultValue, bool move)
{
    auto v = defaultValue;
    auto readSize = sizeof(T);
    overrun_ = false;
    if (size_ < position_ + readSize)
    {
        if (move) position_ = size_;
        overrun_ = true;
        return v;
    }
    stream_.read(reinterpret_cast<char*>(std::addressof(v)), readSize);
    if (move)
    {
        position_ += readSize;
    }
    else
    {
        stream_.seekg(position_, std::fstream::beg);
    }
    return (targetEndian_ != nativeEndian_) ? EndianUtils::Flip(v) : v;
}

#define MAKE_TEMPLATE_INSTANCE_WITH(T) \
    template T BinaryReader::Read(T); \
    template T BinaryReader::Peek(T);

MAKE_TEMPLATE_INSTANCE_WITH(char);
MAKE_TEMPLATE_INSTANCE_WITH(short);
MAKE_TEMPLATE_INSTANCE_WITH(int);
MAKE_TEMPLATE_INSTANCE_WITH(long);
MAKE_TEMPLATE_INSTANCE_WITH(long long);
MAKE_TEMPLATE_INSTANCE_WITH(signed char);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned char);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned short);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned int);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned long);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned long long);
MAKE_TEMPLATE_INSTANCE_WITH(float);
MAKE_TEMPLATE_INSTANCE_WITH(double);
MAKE_TEMPLATE_INSTANCE_WITH(long double);
MAKE_TEMPLATE_INSTANCE_WITH(int8_t);
MAKE_TEMPLATE_INSTANCE_WITH(int16_t);
MAKE_TEMPLATE_INSTANCE_WITH(int32_t);
MAKE_TEMPLATE_INSTANCE_WITH(int64_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint8_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint16_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint32_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint64_t);

} // namespace miso
