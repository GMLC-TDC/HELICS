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
#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

//!< alias for convenience
using stringVector = std::vector<std::string>;

/* some common functions that don't need to be the namespace*/

/** @brief convert a string to lower case as a new string
@param[in] input  the string to convert
@return the string with all upper case converted to lower case
*/
std::string convertToLowerCase (const std::string &input);
/** @brief convert a string to upper case as a new string
@param[in] input  the string to convert
@return the string with all lower case letters converted to upper case
*/
std::string convertToUpperCase (const std::string &input);
/** @brief make a string lower case
@param[in,out] input  the string to convert
*/
void makeLowerCase (std::string &input);

/** @brief make a string upper case
@param[in,out] input  the string to convert
*/
void makeUpperCase (std::string &input);

namespace stringOps
{
const unsigned int factors[] = {1, 10, 100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000};
/**@brief append the text of the integral part of a number to a string*/
template <typename X>
void appendInteger (std::string &input, X val)
{
    if (val < X (0))
    {
        input.push_back ('-');
    }
    X x = (std::is_signed<X>::value) ? ((val < X (0)) ? (X (0) - val) : val) : val;
    if (x < 10)
    {
        input.push_back (x + '0');
        return;
    }
    int digits =
      (x < 100 ?
         2 :
         (x < 1000 ?
            3 :
            (x < 10'000 ?
               4 :
               (x < 100'000 ? 5 :
                              (x < 1'000'000 ?
                                 6 :
                                 (x < 10'000'000 ? 7 : (x < 100'000'000 ? 8 : (x < 1'000'000'000 ? 9 : 500))))))));
    if (digits > 9)  // don't deal with really big numbers
    {
        input += std::to_string (x);
        return;
    }
    unsigned int rem = static_cast<int> (x);
    for (auto dig = digits - 1; dig >= 0; --dig)
    {
        unsigned int place = rem / factors[dig];
        input.push_back (place + '0');
        rem -= factors[dig] * place;
    }
}

const std::string whiteSpaceCharacters (" \t\n\r\0\v\f");

/** @brief trim whitespace characters from a string at the beginning and end of the string
@param[in,out] input  the string to convert
*/
void trimString (std::string &input, const std::string &whitespace = whiteSpaceCharacters);

/** @brief trim whitespace characters from a string
@param[in] input the string to trim;
@param[in] whitespace  the definition of whitespace characters defaults to " \t\n"
@return the trimmed string
*/

std::string trim (const std::string &input, const std::string &whitespace = whiteSpaceCharacters);

/** @brief trim whitespace from a vector of strings
@param[in] input the vector of strings to trim;
@param[in] whitespace  the definition of whitespace characters defaults to " \t\n"
*/

void trim (stringVector &input, const std::string &whitespace = whiteSpaceCharacters);

/** @brief get a string that comes after the last of a specified separator
@param[in] input  the string to separate
@param[in] sep the separation character
@return  the tail string or the string that comes after the last sep character
if not found returns the entire string
*/
std::string getTailString (const std::string &input, char sep);
/** @brief get a string that comes after the last of a specified separator
@param[in] input  the string to separate
@param[in] sep the separation character
@return  the tail string or the string that comes after the last sep character
if not found returns the entire string
*/
std::string getTailString (const std::string &input, const std::string &sep);

const std::string default_delim_chars (",;");
const std::string default_quote_chars (R"raw('"`)raw");
const std::string default_bracket_chars (R"raw([{(<'"`)raw");

enum class delimiter_compression
{
    on,
    off,
};

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters
*/
stringVector splitline (const std::string &line,
                        const std::string &delimiters = default_delim_chars,
                        delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[in] del the delimiter character
@return a vector of strings separated by the delimiters characters
*/
stringVector splitline (const std::string &line, char del);

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in] del the delimiter character

*/
void splitline (const std::string &line, stringVector &strVec, char del);

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
*/
void splitline (const std::string &line,
                stringVector &strVec,
                const std::string &delimiters = default_delim_chars,
                delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings taking into account quote characters
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into
account
@param[in] line  the string to split
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
stringVector splitlineQuotes (const std::string &line,
                              const std::string &delimiters = default_delim_chars,
                              const std::string &quoteChars = default_quote_chars,
                              delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings taking into account bracketing characters
 bracket characters include "()","{}","[]","<>" as well as quote characters ' and "
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into
account
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
stringVector splitlineBracket (const std::string &line,
                               const std::string &delimiters = default_delim_chars,
                               const std::string &bracketChars = default_bracket_chars,
                               delimiter_compression compression = delimiter_compression::off);

/** @brief extract a trailing number from a string return the number and the string without the number
@param[in] input the string to extract the information from
@param[out]  the leading string with the numbers removed
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (const std::string &input, std::string &output, int defNum = -1);

/** @brief extract a trailing number from a string
@param[in] input the string to extract the information from
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (const std::string &input, int defNum = -1);

/**@brief enumeration for string close matches
 */
enum string_match_type_t
{
    string_match_close,
    string_match_begin,
    string_match_end,
    string_match_exact
};

/** @brief find a close match in a vector of strings to a test string
 function searches for any of the testStrings in the testStrings vector based on the matchType parameter and
returns the index into the testStrings vector
@param[in] testStrings the vector of strings to search for
@param[in] iString the string library to search through
@param[in] matchType the matching type
@return the index of the match or -1 if no match is found
*/
int findCloseStringMatch (const stringVector &testStrings,
                          const stringVector &iStrings,
                          string_match_type_t matchType = string_match_close);

/** @brief remove a set of characters from a string
@param[in] source  the original string
@param[in] remchars the characters to remove
@return  the string with the specified character removed
*/
std::string removeChars (const std::string &source, const std::string &remchars);

/** @brief remove a particular character from a string
@param[in] source  the original string
@param[in] remchar the character to remove
@return  the string with the specified character removed
*/
std::string removeChar (const std::string &source, char remchar);

/** @brief remove quotes from a string
 only quotes around the edges are removed along with whitespace outside the quotes
@param[in] source  the original string
@return  the string with quotes removed
*/
std::string removeQuotes (const std::string &str);

/** @brief outer brackets from a string
Bracket characters include [({<
@param[in] source  the original string
@return  the string with brackets removed
*/
std::string removeBrackets (const std::string &str);

/** @brief replace a particular key character with a different string
@param[in] source  the original string
@param[in] key the character to replace
@param[in]  the string to replace the key with
@return  the string after the specified replacement
*/
std::string characterReplace (const std::string &source, char key, const std::string &repStr);

/** @brief replace XML character codes with the appropriate character
@param[in] str  the string to do the replacement on
@return the string with the character codes removed and replaced with the appropriate character
*/
std::string xmlCharacterCodeReplace (std::string str);
}  // namespace stringOps
