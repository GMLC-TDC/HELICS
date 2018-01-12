/*
* LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "charMapper.h"

namespace utilities
{
charMapper<bool> numericMapper ()
{
    charMapper<bool> nm (false);
    nm.addKey ('0', true);
    nm.addKey ('1', true);
    nm.addKey ('2', true);
    nm.addKey ('3', true);
    nm.addKey ('4', true);
    nm.addKey ('5', true);
    nm.addKey ('6', true);
    nm.addKey ('7', true);
    nm.addKey ('8', true);
    nm.addKey ('9', true);
    nm.addKey ('+', true);
    nm.addKey ('-', true);
    nm.addKey (' ', true);
    nm.addKey ('e', true);
    nm.addKey ('E', true);
    nm.addKey ('.', true);
    return nm;
}

charMapper<bool> numericStartMapper ()
{
    charMapper<bool> nm (false);
    nm.addKey ('0', true);
    nm.addKey ('1', true);
    nm.addKey ('2', true);
    nm.addKey ('3', true);
    nm.addKey ('4', true);
    nm.addKey ('5', true);
    nm.addKey ('6', true);
    nm.addKey ('7', true);
    nm.addKey ('8', true);
    nm.addKey ('9', true);
    nm.addKey ('+', true);
    nm.addKey ('-', true);
    nm.addKey (' ', true);
    nm.addKey ('\t', true);
    nm.addKey ('.', true);
    nm.addKey ('\n', true);
    nm.addKey ('\r', true);
    nm.addKey ('\0', true);
    return nm;
}

charMapper<bool> numericEndMapper ()
{
    charMapper<bool> nm (false);
    nm.addKey ('0', true);
    nm.addKey ('1', true);
    nm.addKey ('2', true);
    nm.addKey ('3', true);
    nm.addKey ('4', true);
    nm.addKey ('5', true);
    nm.addKey ('6', true);
    nm.addKey ('7', true);
    nm.addKey ('8', true);
    nm.addKey ('9', true);
    nm.addKey (' ', true);
    nm.addKey ('\t', true);
    nm.addKey ('\n', true);
    nm.addKey ('\r', true);
    nm.addKey ('\0', true);
    return nm;
}

charMapper<unsigned char> base64Mapper ()
{
    charMapper<unsigned char> b64 (0xFF);
    unsigned char val = 0;
    for (unsigned char c = 'A'; c <= 'Z'; ++c)
    {
        b64.addKey (c, val);
        ++val;
    }
    for (unsigned char c = 'a'; c <= 'z'; ++c)
    {
        b64.addKey (c, val);
        ++val;
    }
    for (unsigned char c = '0'; c <= '9'; ++c)
    {
        b64.addKey (c, val);
        ++val;
    }
    b64.addKey ('+', val++);
    b64.addKey ('/', val);
    return b64;
}

charMapper<unsigned char> digitMapper ()
{
    charMapper<unsigned char> dMap (0xFF);
    unsigned char val = 0;
    for (unsigned char c = '0'; c <= '9'; ++c)
    {
        dMap.addKey (c, val);
        ++val;
    }
    return dMap;
}

charMapper<unsigned char> hexMapper ()
{
    charMapper<unsigned char> dMap (0xFF);
    unsigned char val = 0;
    for (unsigned char c = '0'; c <= '9'; ++c)
    {
        dMap.addKey (c, val);
        ++val;
    }
    for (unsigned char c = 'A'; c <= 'F'; ++c)
    {
        dMap.addKey (c, val);
        ++val;
    }
    val = 10;
    for (unsigned char c = 'a'; c <= 'f'; ++c)
    {
        dMap.addKey (c, val);
        ++val;
    }
    return dMap;
}

charMapper<unsigned char> pairMapper ()
{
    charMapper<unsigned char> dMap (0);
    for (unsigned char ii = 0; ii < 255; ++ii)
    {
        dMap.addKey (ii, ii);
    }
    dMap.addKey ('(', ')');
    dMap.addKey ('<', '>');
    dMap.addKey ('[', ']');
    dMap.addKey ('{', '}');
    dMap.addKey (')', '(');
    dMap.addKey ('>', '<');
    dMap.addKey (']', '[');
    dMap.addKey ('}', '{');
    return dMap;
}

}  // namespace utilities