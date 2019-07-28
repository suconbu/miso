#ifndef MISO_CORE_XML_READER_H_
#define MISO_CORE_XML_READER_H_

#include <string>
#include <map>
#include <vector>

namespace miso {

namespace libxml {
#include "libxml/xmlreader.h"
}

class XmlAttribute {
public:
    XmlAttribute(const char* name, const char* value) : name_(name), value_(value) {}

    const std::string& GetName() const { return name_; }
    const std::string& GetValue() const { return value_; }

private:
    std::string name_;
    std::string value_;
};

enum class XmlNodeType { None, StartElement, EmptyElement, EndElement, Text };

class XmlReader {
public:
    explicit XmlReader(const char* filename);
    XmlReader(const char* buffer, size_t size);
    XmlReader(XmlReader&& other);
    ~XmlReader();

    bool CanRead() const { return reader_ == nullptr || reached_to_end_; }
    const std::vector<std::string>& GetErrors() { return errors_; }
    bool Read();
    void MoveToEndElement();
    XmlNodeType GetNodeType() const { return node_type_; }
    std::string GetElementName() const;
    std::string GetContentText() const;
    const std::string GetAttributeValueString(const char* name) const;
    const std::vector<XmlAttribute> GetAllAttributes() const;
    int GetNestingLevel() const { return libxml::xmlTextReaderDepth(reader_); }

private:
    libxml::xmlParserInputBufferPtr buffer_;
    libxml::xmlTextReaderPtr reader_;
    XmlNodeType node_type_;
    bool reached_to_end_;
    std::vector<std::string> errors_;

    XmlReader(libxml::xmlParserInputBufferPtr buffer);
};

inline
XmlReader::XmlReader(const char* filename) :
    XmlReader(libxml::xmlParserInputBufferCreateFilename(filename, libxml::XML_CHAR_ENCODING_UTF8))
{}

inline
XmlReader::XmlReader(const char* buffer, size_t size) :
    XmlReader(libxml::xmlParserInputBufferCreateStatic(buffer, static_cast<int>(size), libxml::XML_CHAR_ENCODING_UTF8))
{}

inline
XmlReader::XmlReader(libxml::xmlParserInputBufferPtr buffer) :
    buffer_(buffer),
    reader_(libxml::xmlNewTextReader(buffer, nullptr)),
    node_type_(XmlNodeType::None),
    reached_to_end_(false)
{}

inline
XmlReader::XmlReader(XmlReader&& other) :
    buffer_(other.buffer_), reader_(other.reader_)
{
    other.reader_ = nullptr;
    other.buffer_ = nullptr;
}

inline
XmlReader::~XmlReader()
{
    libxml::xmlFreeTextReader(reader_);
    libxml::xmlFreeParserInputBuffer(buffer_);
}

inline bool
XmlReader::Read()
{
    if (reader_ == nullptr) return false;

    while (true) {
        auto result = libxml::xmlTextReaderRead(reader_);
        if (result != 1) {
            if (result == -1) errors_.push_back("xmlTextReaderRead returns -1");
            reached_to_end_ = true;
            return false;
        }

        auto type = libxml::xmlTextReaderNodeType(reader_);
        if (type == libxml::XML_READER_TYPE_ELEMENT) {
            if (libxml::xmlTextReaderIsEmptyElement(reader_) == 1) {
                node_type_ = XmlNodeType::EmptyElement;
            } else {
                node_type_ = XmlNodeType::StartElement;
            }
        } else if (type == libxml::XML_READER_TYPE_END_ELEMENT) {
            node_type_ = XmlNodeType::EndElement;
        } else if (type == libxml::XML_READER_TYPE_TEXT) {
            node_type_ = XmlNodeType::Text;
        } else {
            continue;
        }
        break;
    }
    return true;
}

inline void
XmlReader::MoveToEndElement()
{
    if (node_type_ == XmlNodeType::EmptyElement ||
        node_type_ == XmlNodeType::EndElement) return;
    int count = 1;
    while (true) {
        auto result = libxml::xmlTextReaderRead(reader_);
        if (result != 1) {
            if (result == -1) errors_.push_back("xmlTextReaderRead returns -1");
            reached_to_end_ = true;
            return;
        }

        auto type = libxml::xmlTextReaderNodeType(reader_);
        if (type == libxml::XML_READER_TYPE_ELEMENT) {
            ++count;
        } else if (type == libxml::XML_READER_TYPE_END_ELEMENT) {
            if (--count == 0) {
                node_type_ = XmlNodeType::EndElement;
                break;
            }
        }
    }
}

inline std::string
XmlReader::GetElementName() const
{
    std::string name_str;
    auto name = libxml::xmlTextReaderName(reader_);
    if (name != nullptr) name_str.assign(reinterpret_cast<const char*>(name));
    return name_str;
}


inline std::string
XmlReader::GetContentText() const
{
    std::string text_str;
    auto text = libxml::xmlTextReaderConstValue(reader_);
    if (text != nullptr) text_str.assign(reinterpret_cast<const char*>(text));
    return text_str;
}

inline const std::string
XmlReader::GetAttributeValueString(const char* name) const
{
    std::string value_str;
    auto value = libxml::xmlTextReaderGetAttribute(reader_, reinterpret_cast<const libxml::xmlChar*>(name));
    if (value != nullptr) {
        value_str.assign(reinterpret_cast<const char*>(value));
        libxml::xmlFree(value);
    }
    return value_str;
}


inline const std::vector<XmlAttribute>
XmlReader::GetAllAttributes() const
{
    std::vector<XmlAttribute> attributes;
    if (libxml::xmlTextReaderHasAttributes(reader_) == 1) {
        auto result = libxml::xmlTextReaderMoveToFirstAttribute(reader_);
        while (result) {
            attributes.push_back(XmlAttribute(
                reinterpret_cast<const char*>(libxml::xmlTextReaderConstName(reader_)),
                reinterpret_cast<const char*>(libxml::xmlTextReaderConstValue(reader_))));
            result = libxml::xmlTextReaderMoveToNextAttribute(reader_);
        }
    }
    return attributes;
}

} // namespace miso

#endif // MISO_CORE_XML_READER_H_
