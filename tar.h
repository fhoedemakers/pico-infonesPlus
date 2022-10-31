/*
 * author : Shuichi TAKANO
 * since  : Mon Aug 09 2021 02:50:15
 */
#ifndef _3A466107_0134_6471_2615_EA5DEBBBFAE6
#define _3A466107_0134_6471_2615_EA5DEBBBFAE6

#include <vector>
#include <string_view>
#include <stdint.h>

struct TAREntry
{
    std::string_view filename;
    const uint8_t *data{};
    size_t size = 0;
};

std::vector<TAREntry>
parseTAR(const void *_p, bool (*validateEntry)(const uint8_t *) = {});

#endif /* _3A466107_0134_6471_2615_EA5DEBBBFAE6 */
