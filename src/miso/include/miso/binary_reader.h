#ifndef MISO_CORE_BINARYREADER_H_
#define MISO_CORE_BINARYREADER_H_

#include "miso/endian.h"

//#include <istream>
//#include <vector>
#include <stdio.h>
#include <memory.h>

#pragma warning(disable:4996)

namespace miso {

class StandardAllocator {
public:
    static void* Malloc(size_t size) { return (size > 0) ? std::malloc(size) : nullptr; }
    static void Free(void* p) { std::free(p); }
};

class IStream {
public:
    virtual ~IStream() {}

    virtual bool CanRead(size_t size) const = 0;
    virtual unsigned char Read() = 0;
    virtual unsigned char Peek() const = 0;
    virtual size_t ReadTo(void* buffer, size_t size) = 0;
    virtual size_t GetSize() const = 0;
    virtual size_t GetPosition() const = 0;
    virtual void SetPosition(size_t position) = 0;
};

class MemoryStream : public IStream {
public:
    explicit MemoryStream(const void* memory, size_t size) :
        current_(static_cast<const unsigned char*>(memory)), begin_(static_cast<const unsigned char*>(memory)), end_(static_cast<const unsigned char*>(memory) + size) {}

    bool CanRead(size_t size = 1) const { return begin_ != nullptr && (current_ + size) <= end_; }
    unsigned char Read() { return (current_ < end_) ? *current_++ : *(end_ - 1); }
    unsigned char Peek() const { return (current_ < end_) ? *current_ : *(end_ - 1); }
    size_t ReadTo(void* buffer, size_t size) {
        size_t actual_size = 0;
        if (current_ < end_) {
            size_t remain = static_cast<size_t>(end_ - current_);
            actual_size = (size < remain) ? size : remain;
            memcpy(buffer, current_, actual_size);
            current_ += actual_size;
        }
        return actual_size;
    }
    size_t GetSize() const { return static_cast<size_t>(end_ - begin_); }
    size_t GetPosition() const { return static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) { current_ = ((begin_ + position) < end_) ? (begin_ + position) : end_; }

private:
    const unsigned char* current_;
    const unsigned char* begin_;
    const unsigned char* end_;
};

#ifndef MISO_CORE_FILESTREAM_BUFFER_SIZE
#define MISO_CORE_FILESTREAM_BUFFER_SIZE 256
#endif//MISO_CORE_FILESTREAM_BUFFER_SIZE

class FileStream : public IStream {
public:
    explicit FileStream(const char* filename) :
        fp_(fopen(filename, "rb")), stream_size_(GetStreamSize(fp_)), offset_(0),
        current_(buffer_), begin_(buffer_), end_(nullptr), reached_to_end_(false)
    { if (fp_ != nullptr) FillBuffer(); }
    ~FileStream() { if (fp_ != nullptr) fclose(fp_); }

    bool CanRead(size_t size = 1) const { return fp_ != nullptr && (offset_ + static_cast<size_t>((current_ - begin_)) + size) <= stream_size_; }
    unsigned char Read() { auto one = *current_++; FillBuffer(); return one; }
    unsigned char Peek() const { return *current_; }
    size_t ReadTo(void* buffer, size_t size) {
        size_t remain = size;
        do {
            size_t copy_size = remain;
            size_t current_to_end = static_cast<size_t>(end_ - current_);
            if (current_to_end < copy_size) {
                copy_size = current_to_end;
            }
            memcpy(buffer, current_, copy_size);
            current_ += copy_size;
            FillBuffer();
            remain -= copy_size;
        } while (0 < remain && !reached_to_end_);
        return size - remain;
    }
    size_t GetSize() const { return stream_size_; }
    size_t GetPosition() const { return offset_ + static_cast<size_t>(current_ - begin_); }
    void SetPosition(size_t position) {
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

    static size_t GetStreamSize(FILE* fp) {
        if (fp == nullptr) return 0;
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto begin = ftell(fp);
        return (begin != -1L && end != -1L) ? static_cast<size_t>(end - begin) : 0;
    }
    template<typename TAllocator = StandardAllocator>
    static unsigned char* ReadAll(const char* filename) {
        FILE* fp = fopen(filename, "rb");
        auto size = GetStreamSize(fp);
        auto buffer = (unsigned char*)TAllocator::Malloc(size);
        if (size > 0 && buffer != nullptr) {
            fread(buffer, 1, size, fp);
        }
        if (fp != nullptr) fclose(fp);
        return buffer;
    }

private:
    const size_t buffer_size_ = MISO_CORE_FILESTREAM_BUFFER_SIZE;
    unsigned char buffer_[MISO_CORE_FILESTREAM_BUFFER_SIZE];
    FILE* fp_;
    size_t stream_size_;
    size_t offset_;
    unsigned char* current_;
    unsigned char* begin_;
    unsigned char* end_;
    bool reached_to_end_;

    void FillBuffer() {
        if (current_ < end_ || reached_to_end_) return;
        size_t read_count = fread(buffer_, 1, buffer_size_, fp_);
        if (end_ != nullptr) offset_ += buffer_size_;
        current_ = buffer_;
        end_ = buffer_ + read_count;
        reached_to_end_ = (read_count < buffer_size_);
    }
};

class BinaryReader
{
public:
    explicit BinaryReader(const char* filename, Endian endian = Endian::Native) :
        reader_(new FileStream(filename)),
        native_endian_(EndianUtils::GetNativeEndian()),
        target_endian_(endian == Endian::Native ? native_endian_ : endian)
    {}
    explicit BinaryReader(const void* buffer, size_t size, Endian endian = Endian::Native) :
        reader_(new MemoryStream(static_cast<const char*>(buffer), size)),
        native_endian_(EndianUtils::GetNativeEndian()),
        target_endian_(endian == Endian::Native ? native_endian_ : endian)
    {}
    ~BinaryReader() { delete reader_; }

    bool CanRead(size_t size = 1) const { return reader_ != nullptr && reader_->CanRead(size); }
    //bool IsOverrunOccurred() const { return overrun_; }
    size_t GetSize() const { return reader_->GetSize(); }
    Endian GetEndian() const { return target_endian_; }
    void SetEndian(Endian endian) { target_endian_ = endian; }
    size_t GetPosition() const { return reader_->GetPosition(); }
    void SetPosition(size_t position) { reader_->SetPosition(position); }
    template<typename T> T Read(T default_value = 0) { return ReadStream(default_value, true); }
    template<typename T> T Peek(T default_value = 0) { return ReadStream(default_value, false); }
    std::vector<unsigned char> ReadToVector(size_t size) {
        if (!CanRead()) return std::vector<unsigned char>(0);
        std::vector<unsigned char> v(size);
        v.resize(reader_->ReadTo(v.data(), size));
        return v;
    }
    size_t ReadTo(void* buffer, size_t size) { return CanRead() ? reader_->ReadTo(static_cast<char*>(buffer), size) : 0; }

private:
    Endian native_endian_;
    Endian target_endian_;
    IStream* reader_;

    template<typename T> T ReadStream(T default_value, bool advance) {
        if (!CanRead()) return default_value;
        auto read_size = sizeof(T);
        auto position = reader_->GetPosition();
        T v;
        auto actual_size = reader_->ReadTo(reinterpret_cast<char*>(std::addressof(v)), read_size);
        if (!advance) {
            reader_->SetPosition(position);
        }
        if (actual_size < read_size) return default_value;
        return (target_endian_ != native_endian_) ? EndianUtils::Flip(v) : v;
    }
};

} // namespace miso

#endif // MISO_CORE_BINARYREADER_H_
