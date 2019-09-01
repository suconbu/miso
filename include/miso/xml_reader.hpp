#ifndef MISO_XML_READER_HPP_
#define MISO_XML_READER_HPP_

#include "miso/common.h"

#include <string>
#include <vector>

namespace miso {

namespace libxml {
#include "libxml/xmlreader.h"
}

class XmlAttribute {
public:
    XmlAttribute() = delete;
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
    XmlReader() = delete;
    XmlReader(const XmlReader& other) = delete;
    XmlReader& operator=(const XmlReader&) = delete;
    XmlReader(XmlReader&& other) noexcept;
    XmlReader& operator=(XmlReader&&) = delete;
    explicit XmlReader(const char* filename);
    explicit XmlReader(const char* buffer, size_t size);
    ~XmlReader();

    bool CanRead() const { return reader_ != nullptr && !reached_to_end_; }
    bool HasError() { return !errors_.empty(); }
    const std::vector<std::string>& GetErrors() { return errors_; }
    bool Read();
    bool MoveToElement(const char* element_name = nullptr, const char* attribute_name = nullptr, const char* attribute_value = nullptr);
    bool MoveToElementInCurrentLevel(const char* element_name = nullptr, const char* attribute_name = nullptr, const char* attribute_value = nullptr);
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

    bool MoveToElementInside(const char* element_name, const char* attribute_name, const char* attribute_value, bool current_level);
    bool MoveToEndElementInside(bool end_of_parent);
    static void ErrorHandler(void* arg, const char* msg, libxml::xmlParserSeverities severity, libxml::xmlTextReaderLocatorPtr locator);

    libxml::xmlParserInputBufferPtr buffer_ = nullptr;
    libxml::xmlTextReaderPtr reader_ = nullptr;
    XmlNodeType node_type_ = XmlNodeType::None;
    bool reached_to_end_ = false;
    std::vector<std::string> errors_;
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "xml_reader.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_XML_READER_HPP_
