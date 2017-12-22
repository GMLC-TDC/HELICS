/*
   base64.cpp and base64.h
   Copyright (C) 2004-2008 René Nyffenegger
   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:
   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.
   3. This notice may not be removed or altered from any source distribution.
   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

   modified by Philip Top LLNL 2017:  added charMapper objects instead of find and C++ based overloads
*/

#include "base64.h"
#include "charMapper.h"
#include <iostream>
#include <vector>

namespace utilities
{
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

static const charMapper<unsigned char> b64Map = base64Mapper ();

static inline bool is_base64 (unsigned char c) { return (b64Map[c] < 0xFF); }
std::string base64_encode (unsigned char const *bytes_to_encode, int32_t in_len)
{
    std::string ret;
    int ii = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len-- != 0)
    {
        char_array_3[ii++] = *(bytes_to_encode++);
        if (ii == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (ii = 0; (ii < 4); ii++)
            {
                ret.push_back (base64_chars[char_array_4[ii]]);
            }
            ii = 0;
        }
    }

    if (ii != 0)
    {
        for (int jj = ii; jj < 3; ++jj)
        {
            char_array_3[jj] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int jj = 0; (jj < ii + 1); ++jj)
        {
            ret.push_back (base64_chars[char_array_4[jj]]);
        }

        while ((ii++ < 3))
        {
            ret.push_back ('=');
        }
    }

    return ret;
}

std::vector<unsigned char> base64_decode (std::string const &encoded_string)
{
    auto in_len = encoded_string.size ();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;
    ret.reserve (in_len);

    while (((in_len--) != 0u) && (encoded_string[in_] != '=') && is_base64 (encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            char_array_4[0] = b64Map[char_array_4[0]];
            char_array_4[1] = b64Map[char_array_4[1]];
            char_array_4[2] = b64Map[char_array_4[2]];
            char_array_4[3] = b64Map[char_array_4[3]];

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            ret.push_back (char_array_3[0]);
            ret.push_back (char_array_3[1]);
            ret.push_back (char_array_3[2]);

            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 4; j++)
        {
            char_array_4[j] = 0;
        }
        char_array_4[0] = b64Map[char_array_4[0]];
        char_array_4[1] = b64Map[char_array_4[1]];
        char_array_4[2] = b64Map[char_array_4[2]];
        char_array_4[3] = b64Map[char_array_4[3]];

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
        {
            ret.push_back (char_array_3[j]);
        }
    }

    return ret;
}
}  // namespace utilities