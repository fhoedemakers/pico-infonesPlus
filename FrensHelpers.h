#ifndef FRENSHELPERS
#define FRENSHELPERS
#include <string.h>
#include <string>
#include <algorithm>
namespace Frens
{
    bool endsWith(std::string const &str, std::string const &suffix);
    std::string str_tolower(std::string s);

    bool cstr_endswith(const char *string, const char *width);
} // namespace Frens


#endif