#ifndef MISO_CORE_STRING_H_
#define MISO_CORE_STRING_H_

#include <string>
#include <vector>

namespace miso {

class StringUtils
{
public:
    static std::string ReadFile(const std::string& filepath);
    static void WriteFile(const std::string& filepath, const std::string& content);
    static std::vector<std::string> Split(const std::string& str, const std::string& delim, bool trim_empty = false);
    static std::string Join(const std::vector<std::string>& tokens, const std::string& delim, const bool trim_empty = false);
    static std::string Trim(const std::string& str, const std::string& blank = "\r\n\t ");
    static std::string Repeat(const std::string& str, size_t times);
    static std::string ReplaceAll(const std::string& str, const std::string& old_sub_str, const std::string& new_sub_str);
    static std::string ToUpper(const std::string& str);
    static std::string ToLower(const std::string& str);
    static std::string Format(const char* format, ...);
};

} // namespace miso

#endif // MISO_CORE_STRING_H_
