#ifndef MISO_STREAM_HPP_
#define MISO_STREAM_HPP_

#include "miso/common.h"

namespace miso {

class IStream {
public:
    virtual ~IStream() = default;

    virtual bool CanRead(size_t size = 1) const = 0;
    virtual uint8_t Read() = 0;
    virtual uint8_t Peek() const = 0;
    virtual size_t ReadBlock(uint8_t *buffer, size_t size) = 0;
    virtual size_t GetSize() const = 0;
    virtual size_t GetPosition() const = 0;
    virtual void SetPosition(size_t position) = 0;

protected:
    IStream() = default;
};

} // namespace miso

#endif // MISO_STREAM_HPP_
