#include "miso/xml_reader.hpp"

#include <cstring>
#include <vector>

#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE
XmlReader::XmlReader(const char* filename) :
    XmlReader(libxml::xmlParserInputBufferCreateFilename(filename, libxml::XML_CHAR_ENCODING_UTF8))
{}

MISO_INLINE
XmlReader::XmlReader(const char* buffer, size_t size) :
    XmlReader(libxml::xmlParserInputBufferCreateStatic(buffer, static_cast<int>(size), libxml::XML_CHAR_ENCODING_UTF8))
{}

MISO_INLINE
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

MISO_INLINE
XmlReader::XmlReader(XmlReader&& other) noexcept :
    buffer_(other.buffer_), reader_(other.reader_)
{
    other.reader_ = nullptr;
    other.buffer_ = nullptr;
}

MISO_INLINE
XmlReader::~XmlReader()
{
    libxml::xmlFreeTextReader(reader_);
    libxml::xmlFreeParserInputBuffer(buffer_);
}

MISO_INLINE bool
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

MISO_INLINE bool
XmlReader::MoveToElement(const char* element_name, const char* attribute_name, const char* attribute_value)
{
    return MoveToElementInside(element_name, attribute_name, attribute_value, false);
}

MISO_INLINE bool
XmlReader::MoveToElementInCurrentLevel(const char* element_name, const char* attribute_name, const char* attribute_value)
{
    return MoveToElementInside(element_name, attribute_name, attribute_value, true);
}

MISO_INLINE bool
XmlReader::MoveToElementInside(const char* element_name, const char* attribute_name, const char* attribute_value, bool only_current_level)
{
    int count = 1;
    if (node_type_ == XmlNodeType::StartElement) {
        ++count;
    }

    while (true) {
        if (libxml::xmlTextReaderRead(reader_) != 1) {
            reached_to_end_ = true;
            return false;
        }

        auto type = libxml::xmlTextReaderNodeType(reader_);
        if (type == libxml::XML_READER_TYPE_ELEMENT) {
            bool found = false;
            do {
                if (only_current_level && count != 1) {
                    break;
                }
                if (element_name != nullptr) {
                    auto name = reinterpret_cast<const char*>(libxml::xmlTextReaderConstName(reader_));
                    if (name == nullptr || std::strcmp(element_name, name) != 0) break;
                }
                if (attribute_name != nullptr) {
                    auto value = reinterpret_cast<char*>(
                        libxml::xmlTextReaderGetAttribute(reader_, reinterpret_cast<const libxml::xmlChar*>(attribute_name)));
                    if (value == nullptr ||
                        (attribute_value != nullptr && std::strcmp(attribute_value, value) != 0)) {
                        libxml::xmlFree(value);
                        break;
                    }
                }
                found = true;
            } while (false);

            bool empty_element = (libxml::xmlTextReaderIsEmptyElement(reader_) == 1);
            if (found) {
                node_type_ = empty_element ? XmlNodeType::EmptyElement : XmlNodeType::StartElement;
                break;
            }
            if (only_current_level && !empty_element) {
                ++count;
            }
        } else if (type == libxml::XML_READER_TYPE_END_ELEMENT) {
            if (only_current_level && --count == 0) {
                node_type_ = XmlNodeType::EndElement;
                return false;
            }
        }
    }

    return true;
}

MISO_INLINE bool
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

MISO_INLINE std::string
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


MISO_INLINE std::string
XmlReader::GetContentText() const
{
    std::string text_str;
    if (node_type_ == XmlNodeType::Text) {
        auto text = reinterpret_cast<const char*>(libxml::xmlTextReaderConstValue(reader_));
        if (text != nullptr) text_str.assign(text);
    }
    return text_str;
}

MISO_INLINE const std::string
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


MISO_INLINE const std::vector<XmlAttribute>
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

MISO_INLINE void
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
