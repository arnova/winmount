#pragma once
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <stdint.h>

class StringUtils
{
  public:
    static std::vector<std::string> Tokenize(const std::string& strLine, const std::string& strDelimiters);
    static bool Split(const std::string& str, const std::string& strDelim, std::string& strLeft, std::string& strRight);
    static bool GetKeyValue(const std::string& str, const std::string& strKey, std::string& strValue, const std::string& strToken = " ");
    static std::string Left(const std::string& str, size_t count);
    static std::string Mid(const std::string& str, size_t first, size_t count = std::string::npos);
    static std::string Right(const std::string& str, size_t count);
    static bool StringToInt32(const std::string& str, int32_t& iValue);
    static bool StringToDouble(const std::string& str, double& iValue);
    static bool EqualsNoCase(const char *s1, const char *s2);
    static bool EqualsNoCase(const std::string& str1, const std::string& str2);
    static bool StartsWith(const std::string& str1, const std::string& str2);
    static void Replace(std::string& str, const char& cOld, const char& cNew);
    static void Replace(std::string& str, const std::string& strOld, const std::string& strNew);
    static std::string ToUpper(const std::string& str);
    static std::string ToLower(const std::string& str);
    static std::string IntToHex(const uint32_t& iDecimal, const int8_t& iMinLen = 0);
    static bool HexToByte(const std::string& strHex, uint8_t& Byte);
    static bool HexToWord(const std::string& strHex, uint16_t& Word);
    static std::string TrimLeft(const std::string& str, const char *chars = " \t\n\r");
    static std::string TrimRight(const std::string& str, const char *chars = " \t\n\r");
    static std::string Trim(const std::string& str, const char *chars = " \t\n\r");
    static std::string Int32ToString(const int32_t& iValue);
};

#endif // STRING_UTILS_H
