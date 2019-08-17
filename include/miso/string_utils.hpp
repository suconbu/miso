#ifndef MISO_STRING_HPP_
#define MISO_STRING_HPP_

#include "miso/common.h"

#include <string>
#include <vector>

namespace miso {

class StringUtils {
public:
    StringUtils() = delete;

    static std::string ReadFile(const std::string& filepath);
    static void WriteFile(const std::string& filepath, const std::string& content);
    static std::vector<std::string> Split(const std::string& str);
    static std::vector<std::string> Split(const std::string& str, const std::string& delim, bool trim_empty = false);
    static std::string Join(const std::vector<std::string>& tokens, const std::string& delim, const bool trim_empty = false);
    static std::string Trim(const std::string& str, const std::string& blank = kBlankChars);
    static std::string Slice(const std::string& str, int start, int end = INT_MAX);
    static std::string Repeat(const std::string& str, size_t times);
    static std::string ReplaceAll(const std::string& str, const std::string& old_sub_str, const std::string& new_sub_str);
    static std::string ToUpper(const std::string& str);
    static std::string ToLower(const std::string& str);
    static std::string Format(const char* format, ...);
    static bool StartsWith(const char* str, const char* x, bool ignore_case = false);
    static bool StartsWith(const std::string& str, const std::string& x, bool ignore_case = false);
    static bool EndsWith(const char* str, const char* x, bool ignore_case = false);
    static bool EndsWith(const std::string& str, const std::string& x, bool ignore_case = false);
    static bool Contains(const char* str, const char* x, bool ignore_case = false);
    static bool Contains(const std::string& str, const std::string& x, bool ignore_case = false);
    static int Compare(const char* a, const char* b, bool ignore_case = false);
    static int Compare(const std::string& a, const std::string& b, bool ignore_case = false);
    static int CompareN(const char* a, const char* b, size_t count, bool ignore_case = false);
    static int CompareN(const std::string& a, const std::string& b, size_t count, bool ignore_case = false);

private:
    static constexpr const char* kBlankChars = " \f\n\r\t\v";
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "string_utils.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_STRING_HPP_
