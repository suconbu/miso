#ifndef MISO_XML_READER_H_
#define MISO_XML_READER_H_

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
    XmlReader(XmlReader&& other) noexcept;
    ~XmlReader();

    bool CanRead() const { return reader_ != nullptr && !reached_to_end_; }
    bool HasError() { return !errors_.empty(); }
    const std::vector<std::string>& GetErrors() { return errors_; }
    bool Read();
    bool MoveToElement(const char* element_name = nullptr, const char* attribute_name = nullptr, const char* attribute_value = nullptr);
    bool MoveToEndElement() { return MoveToEndElementInside(false); }
    bool MoveToEndOfParentElement() { return MoveToEndElementInside(true); }
    XmlNodeType GetNodeType() const { return node_type_; }
    std::string GetElementName() const;
    std::string GetContentText() const;
    const std::string GetAttributeValueString(const char* name) const;
    const std::vector<XmlAttribute> GetAllAttributes() const;
    int GetNestingLevel() const { return libxml::xmlTextReaderDepth(reader_); }

private:
    XmlReader(libxml::xmlParserInputBufferPtr buffer);
    bool MoveToEndElementInside(bool end_of_parent);
    static void ErrorHandler(void* arg, const char* msg, libxml::xmlParserSeverities severity, libxml::xmlTextReaderLocatorPtr locator);

    libxml::xmlParserInputBufferPtr buffer_;
    libxml::xmlTextReaderPtr reader_;
    XmlNodeType node_type_;
    bool reached_to_end_;
    std::vector<std::string> errors_;
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
{
    if (reader_ != nullptr) {
        libxml::xmlTextReaderSetErrorHandler(reader_, ErrorHandler, this);
    } else {
        errors_.push_back("Cannot open file");
    }
}

inline
XmlReader::XmlReader(XmlReader&& other) noexcept :
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
        if (libxml::xmlTextReaderRead(reader_) != 1) {
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

inline bool
XmlReader::MoveToElement(const char* element_name, const char* attribute_name, const char* attribute_value)
{
    while (true) {
        if (libxml::xmlTextReaderRead(reader_) != 1) {
            reached_to_end_ = true;
            return false;
        }

        auto type = libxml::xmlTextReaderNodeType(reader_);
        if (type == libxml::XML_READER_TYPE_ELEMENT) {
            if (element_name != nullptr) {
                auto name = reinterpret_cast<const char*>(libxml::xmlTextReaderConstName(reader_));
                if (name == nullptr || strcmp(element_name, name) != 0) continue;
            }
            if (attribute_name != nullptr) {
                auto value = reinterpret_cast<char*>(
                    libxml::xmlTextReaderGetAttribute(reader_, reinterpret_cast<const libxml::xmlChar*>(attribute_name)));
                if (value == nullptr ||
                    (attribute_value != nullptr && strcmp(attribute_value, value) != 0)) {
                    libxml::xmlFree(value);
                    continue;
                }
            }

            if (libxml::xmlTextReaderIsEmptyElement(reader_) == 1) {
                node_type_ = XmlNodeType::EmptyElement;
            } else {
                node_type_ = XmlNodeType::StartElement;
            }

            break;
        }
    }

    return true;
}

inline bool
XmlReader::MoveToEndElementInside(bool end_of_parent)
{
    int count = 1;
    if (end_of_parent) {
        if (node_type_ == XmlNodeType::StartElement) ++count;
    } else {
        if (node_type_ != XmlNodeType::StartElement) return false;
    }

    while (true) {
        if (libxml::xmlTextReaderRead(reader_) != 1) {
            reached_to_end_ = true;
            return false;
        }

        auto type = libxml::xmlTextReaderNodeType(reader_);
        if (type == libxml::XML_READER_TYPE_ELEMENT) {
            if (libxml::xmlTextReaderIsEmptyElement(reader_) != 1) {
                ++count;
            }
        } else if (type == libxml::XML_READER_TYPE_END_ELEMENT) {
            if (--count == 0) {
                node_type_ = XmlNodeType::EndElement;
                break;
            }
        }
    }

    return true;
}

inline std::string
XmlReader::GetElementName() const
{
    std::string name_str;
    if (node_type_ == XmlNodeType::StartElement ||
        node_type_ == XmlNodeType::EmptyElement ||
        node_type_ == XmlNodeType::EndElement) {
        auto name = reinterpret_cast<const char*>(libxml::xmlTextReaderConstName(reader_));
        if (name != nullptr) name_str.assign(name);
    }
    return name_str;
}


inline std::string
XmlReader::GetContentText() const
{
    std::string text_str;
    if (node_type_ == XmlNodeType::Text) {
        auto text = reinterpret_cast<const char*>(libxml::xmlTextReaderConstValue(reader_));
        if (text != nullptr) text_str.assign(text);
    }
    return text_str;
}

inline const std::string
XmlReader::GetAttributeValueString(const char* name) const
{
    std::string value_str;
    if (node_type_ == XmlNodeType::StartElement ||
        node_type_ == XmlNodeType::EmptyElement) {
        auto value = reinterpret_cast<char*>(
            libxml::xmlTextReaderGetAttribute(reader_, reinterpret_cast<const libxml::xmlChar*>(name)));
        if (value != nullptr) {
            value_str.assign(value);
            libxml::xmlFree(value);
        }
    }
    return value_str;
}


inline const std::vector<XmlAttribute>
XmlReader::GetAllAttributes() const
{
    std::vector<XmlAttribute> attributes;
    if (node_type_ == XmlNodeType::StartElement ||
        node_type_ == XmlNodeType::EmptyElement) {
        if (libxml::xmlTextReaderHasAttributes(reader_) == 1) {
            auto result = libxml::xmlTextReaderMoveToFirstAttribute(reader_);
            while (result) {
                attributes.push_back(XmlAttribute(
                    reinterpret_cast<const char*>(libxml::xmlTextReaderConstName(reader_)),
                    reinterpret_cast<const char*>(libxml::xmlTextReaderConstValue(reader_))));
                result = libxml::xmlTextReaderMoveToNextAttribute(reader_);
            }
        }
    }
    return attributes;
}

inline void
XmlReader::ErrorHandler(void* arg, const char* msg, libxml::xmlParserSeverities severity, libxml::xmlTextReaderLocatorPtr locator)
{
    (void)locator;
    static const char* severities[] = { "", "VALIDITY_WARNING", "VALIDITY_ERROR", "WARNING", "ERROR" };
    auto severity_label = severities[severity];
    auto message = StringUtils::Trim(msg);
    auto& reader = *static_cast<XmlReader*>(arg);
    reader.errors_.push_back(StringUtils::Format("[%s] %s", severity_label, message.c_str()));
}

} // namespace miso

#endif // MISO_XML_READER_H_
