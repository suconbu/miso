#include "miso/string_utils.hpp"

#include <stdarg.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace miso {

MISO_INLINE std::string
StringUtils::ReadFile(const std::string& filepath)
{
    std::ifstream ifs(filepath.c_str());
    std::string content(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    ifs.close();
    return content;
}

MISO_INLINE void
StringUtils::WriteFile(const std::string& filepath, const std::string& content)
{
    std::ofstream ofs(filepath.c_str());
    ofs << content;
    ofs.close();
}

// Split string by blank chars.
MISO_INLINE std::vector<std::string>
StringUtils::Split(const std::string& str)
{
    std::vector<std::string> tokens;
    size_t str_len = str.size();
    size_t last_pos = 0;
    while (true) {
        size_t pos = str.find_first_of(kBlankChars, last_pos);
        pos = (pos == std::string::npos) ? str_len : pos;
        size_t len = pos - last_pos;
        if (len > 0) {
            tokens.push_back(str.substr(last_pos, len));
        }
        if (pos == str_len) break;
        last_pos = pos + 1;
    }
    return tokens;
}

// Split string by specific string.
// If 'delim' is empty string, the returns vector contains one element consisting of the entire 'str'.
MISO_INLINE std::vector<std::string>
StringUtils::Split(const std::string& str, const std::string& delim, bool trim_empty)
{
    std::vector<std::string> tokens;
    size_t str_len = str.size();
    size_t delim_len = delim.size();
    if (delim_len > 0) {
        size_t last_pos = 0;
        while (true) {
            size_t pos = str.find(delim, last_pos);
            pos = (pos == std::string::npos) ? str_len : pos;
            size_t len = pos - last_pos;
            if (!trim_empty || len > 0) {
                tokens.push_back(str.substr(last_pos, len));
            }
            if (pos == str_len) break;
            last_pos = pos + delim_len;
        }
    } else {
        if (!trim_empty || str_len > 0) {
            tokens.push_back(str);
        }
    }
    return tokens;
}

MISO_INLINE std::string
StringUtils::Join(const std::vector<std::string>& tokens, const std::string& delim, const bool trim_empty)
{
    std::stringstream joined;
    for (size_t i = 0; i < tokens.size() - 1; ++i) {
        if (!trim_empty || tokens[i].length() > 0) {
            joined << tokens[i] << delim;
        }
    }

    if (!trim_empty || tokens[tokens.size() - 1].length() > 0) {
        joined << tokens[tokens.size() - 1];
    }

    return joined.str();
}

MISO_INLINE std::string
StringUtils::Trim(const std::string& str, const std::string& blank)
{
    size_t s = str.size();
    for (size_t i = 0; i < str.size(); ++i) {
        if (blank.find(str[i]) == std::string::npos) {
            s = i;
            break;
        }
    }

    size_t e = 0;
    for (size_t i = str.size(); i > 0; --i) {
        if (blank.find(str[i - 1]) == std::string::npos) {
            e = i - 1;
            break;
        }
    }

    return (s <= e) ? str.substr(s, e - s + 1) : "";
}

MISO_INLINE std::string
StringUtils::Slice(const std::string& str, int start, int end)
{
    int len = static_cast<int>(str.length());
    int s = (start < 0) ? (len - std::min(-start, len)) : std::min(start, len);
    int e = (end < 0) ? (len - std::min(-end, len)) : std::min(end, len);
    size_t count = (s <= e) ? (e - s) : 0;
    return str.substr(s, count);
}

MISO_INLINE std::string
StringUtils::Repeat(const std::string& str, size_t times)
{
    std::stringstream repeated;
    for (size_t i = 0; i < times; ++i) {
        repeated << str;
    }
    return repeated.str();
}

MISO_INLINE std::string
StringUtils::ReplaceAll(const std::string& str, const std::string& old_sub_str, const std::string& new_sub_str)
{
    return Join(Split(str, old_sub_str, false), new_sub_str, false);
}

MISO_INLINE std::string
StringUtils::ToUpper(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}

MISO_INLINE std::string
StringUtils::ToLower(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

MISO_INLINE std::string
StringUtils::Format(const char* format, ...)
{
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);

    int size = vsnprintf(nullptr, 0, format, args2);
    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args1);

    va_end(args1);
    va_end(args2);

    return std::string(buffer.data());
}

MISO_INLINE bool
StringUtils::StartsWith(const char* str, const char* x, bool ignore_case)
{
    size_t x_len = (x != nullptr) ? strlen(x) : 0;
    return CompareN(str, x, x_len, ignore_case) == 0;
}

MISO_INLINE bool
StringUtils::StartsWith(const std::string& str, const std::string& x, bool ignore_case)
{
    return StartsWith(str.c_str(), x.c_str(), ignore_case);
}

MISO_INLINE bool
StringUtils::EndsWith(const char* str, const char* x, bool ignore_case)
{
    size_t str_len = (str != nullptr) ? strlen(str) : 0;
    size_t x_len = (x != nullptr) ? strlen(x) : 0;
    if (str_len < x_len) return false;
    auto start = str + str_len - x_len;
    return CompareN(start, x, x_len, ignore_case) == 0;
}

MISO_INLINE bool
StringUtils::EndsWith(const std::string& str, const std::string& x, bool ignore_case)
{
    return EndsWith(str.c_str(), x.c_str(), ignore_case);
}

MISO_INLINE bool
StringUtils::Contains(const char* str, const char* x, bool ignore_case)
{
    return Contains(std::string(str), std::string(x), ignore_case);
}

MISO_INLINE bool
StringUtils::Contains(const std::string& str, const std::string& x, bool ignore_case)
{
    return ignore_case ?
        StringUtils::ToLower(str).find(StringUtils::ToLower(x)) != std::string::npos :
        str.find(x) != std::string::npos;
}

MISO_INLINE int
StringUtils::Compare(const char* a, const char* b, bool ignore_case)
{
    return CompareN(a, b, SIZE_MAX, ignore_case);
}

MISO_INLINE int
StringUtils::Compare(const std::string& a, const std::string& b, bool ignore_case)
{
    return CompareN(a.c_str(), b.c_str(), SIZE_MAX, ignore_case);
}

MISO_INLINE int
StringUtils::CompareN(const char* a, const char* b, size_t count, bool ignore_case)
{
    if (a == nullptr) {
        return (b == nullptr) ? 0 : -1;
    } else if (b == nullptr) {
        return 1;
    }

    if (!ignore_case) return strncmp(a, b, count);

    int result = 0;
    for (size_t i = 0; i < count; ++i) {
        result = tolower(a[i]) - tolower(b[i]);
        if (result != 0 || a[i] == '\0') break;
    }
    return result;
}

MISO_INLINE int
StringUtils::CompareN(const std::string& a, const std::string& b, size_t count, bool ignore_case)
{
    return CompareN(a.c_str(), b.c_str(), count, ignore_case);
}

} // namespace miso
