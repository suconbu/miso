#ifndef MISO_CORE_BINARYREADER_H_
#define MISO_CORE_BINARYREADER_H_

#include "miso/endian.h"

#include <istream>
#include <vector>

namespace miso {

class BinaryReader
{
public:
    BinaryReader(const char* filename, Endian endian = Endian::Native);
    BinaryReader(const void* buffer, size_t size, Endian endian = Endian::Native);
    ~BinaryReader();
    bool CanRead(size_t size = 1) const { return !stream_.fail() && (position_ + size) <= size_; }
    bool IsOverrunOccurred() const { return overrun_; }
    void Close();
    size_t GetLength() const { return size_; }
    Endian GetEndian() const { return targetEndian_; }
    void SetEndian(Endian endian) { targetEndian_ = endian; }
    size_t GetPosition() const { return position_; }
    void SetPosition(size_t position);
    template<typename T> T Read(T defaultValue = 0);
    template<typename T> T Peek(T defaultValue = 0);
    std::vector<uint8_t> ReadBlock(size_t size);
    size_t ReadBlockTo(void* buffer, size_t size);

private:
    Endian nativeEndian_;
    Endian targetEndian_;
    std::istream* stream_pointer_;
    std::istream stream_;
    size_t size_;
    size_t position_;
    bool overrun_;

    BinaryReader(std::istream* stream, Endian endian);
    size_t GetLengthInside(std::istream& stream);
    template<typename T> T ReadInside(T defaultValue, bool move);
};

} // namespace miso

#endif // MISO_CORE_BINARYREADER_H_
