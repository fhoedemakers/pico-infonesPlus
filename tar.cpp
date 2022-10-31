/*
 * author : Shuichi TAKANO
 * since  : Mon Aug 09 2021 03:01:23
 */

#include "tar.h"
#include <optional>
#include <string.h>
#include <stdio.h>

namespace
{
    std::optional<size_t>
    parseOct(const char *p, int count)
    {
        size_t v = 0;
        while (count)
        {
            char ch = *p++;
            if (ch == 0 || ch == 0x20)
            {
                return v;
            }

            if (ch < '0' || ch >= '8')
            {
                return {};
            }

            v <<= 3;
            v |= ch - '0';

            --count;
        }
        return v;
    }

    bool isAll0(const char *p, size_t s)
    {
        while (s--)
        {
            if (*p++)
            {
                return false;
            }
        }
        return true;
    }
}

std::vector<TAREntry>
parseTAR(const void *_p, bool (*validateEntry)(const uint8_t *))
{
    auto p = static_cast<const char *>(_p);
    std::vector<TAREntry> r;

    while (1)
    {
        if (isAll0(p, 1024))
        {
            return r;
        }

        TAREntry e;
        if (strcmp("ustar", p + 257) != 0)
        {
            printf("parseTAR: invalid magic\n");
            return {};
        }
        if (auto size = parseOct(p + 124, 12))
        {
            e.size = size.value();
        }
        else
        {
            printf("parseTAR: invalid size\n");
            return {};
        }
        e.filename = p;
        e.data = reinterpret_cast<const uint8_t *>(p + 512);
        if (!validateEntry || validateEntry(e.data))
        {
            r.push_back(std::move(e));
        }

        p += (e.size + 512 + 511) & ~511;
    }
}
