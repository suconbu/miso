#include "miso/string.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

namespace miso {

// https://github.com/HondaDai/StringUtils

std::string
StringUtils::ReadFile(const std::string& filepath)
{
    std::ifstream ifs(filepath.c_str());
    std::string content(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    ifs.close();
    return content;
}

void
StringUtils::WriteFile(const std::string& filepath, const std::string& content)
{
    std::ofstream ofs(filepath.c_str());
    ofs << content;
    ofs.close();
}

std::vector<std::string>
StringUtils::Split(const std::string& str, const std::string& delim, bool trim_empty)
{
    std::vector<std::string> tokens;
    size_t last_pos = 0;
    while (true)
    {
        size_t pos = str.find(delim, last_pos);
        pos = (pos == std::string::npos) ? str.size() : pos;

        size_t len = pos - last_pos;
        if (!trim_empty || len > 0)
        {
            tokens.push_back(str.substr(last_pos, len));
        }

        if (pos == str.size()) break;

        last_pos = pos + delim.size();
    }
    return tokens;
}

std::string
StringUtils::Join(const std::vector<std::string>& tokens, const std::string& delim, const bool trim_empty)
{
    std::stringstream joined;
    for (size_t i = 0; i < tokens.size() - 1; ++i)
    {
        if (!trim_empty || tokens[i].length() > 0)
        {
            joined << tokens[i] << delim;
        }
    }

    if (!trim_empty || tokens[tokens.size() - 1].length() > 0)
    {
        joined << tokens[tokens.size() - 1];
    }

    return joined.str();
}

std::string
StringUtils::Trim(const std::string& str, const std::string& blank)
{
    size_t begin = str.size();
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (blank.find(str[i]) == std::string::npos)
        {
            begin = i;
            break;
        }
    }

    size_t end = 0;
    for (size_t i = str.size(); i > 0; --i)
    {
        if (blank.find(str[i - 1]) == std::string::npos)
        {
            end = i - 1;
            break;
        }
    }

    return (begin <= end) ? str.substr(begin, end - begin + 1) : "";
}

std::string
StringUtils::Repeat(const std::string& str, unsigned int times)
{
    std::stringstream repeated;
    for (unsigned int i = 0; i < times; ++i) {
        repeated << str;
    }
    return repeated.str();
}

std::string
StringUtils::ReplaceAll(const std::string& str, const std::string& old_sub_str, const std::string& new_sub_str)
{
    return Join(Split(str, old_sub_str, false), new_sub_str, false);
}

std::string
StringUtils::ToUpper(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}

std::string
StringUtils::ToLower(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

} // namespace miso
