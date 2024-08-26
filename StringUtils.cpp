/*
  (C) Copyright 2014-2018 ELD/LION, Leiden University

  Description       : String Utilities library
  Written by        : Arno van Amersfoort
  Target Compiler   : G++ for Win32 (MSYS)
  Initial date      : September 24, 2014
  Last Modified     : August 21, 2018
*/

#include "StringUtils.h"

#include <stdlib.h>
#include <string>
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <iomanip>

std::vector<std::string> StringUtils::Tokenize(const std::string& strLine, const std::string& strDelimiters)
{
  std::vector<std::string> vecStrings;
  for (unsigned int pos = 0, last = 0; pos < strLine.size(); pos++)
  {
    // Look for a token or end of line
    if (strDelimiters.find_first_of(strLine[pos]) != std::string::npos)
    {
      if (pos >= last + 1) // Skip multiple delims
        vecStrings.push_back(strLine.substr(last, pos - last));

      last = pos + 1;
    }
    else if (pos == strLine.size() - 1) // Last char
    {
      vecStrings.push_back(strLine.substr(last));
    }
  }

  return vecStrings;
}


bool StringUtils::Split(const std::string& str, const std::string& strDelim, std::string& strLeft, std::string& strRight)
{
  const size_t iFindDel = (str.find(strDelim));

  if (iFindDel == std::string::npos)
    return false;

  strLeft = str.substr(0, iFindDel);
  strRight = str.substr(iFindDel + 1);

  return true;
}


bool StringUtils::GetKeyValue(const std::string& str, const std::string& strKey, std::string& strValue, const std::string& strToken /* = " " */)
{
  std::vector<std::string> vecTokens = Tokenize(str, strToken);
  for (std::vector<std::string>::const_iterator it = vecTokens.begin(); it != vecTokens.end(); ++it)
  {
    std::string strGetKey;
    std::string strGetVal;
    const bool bRet = Split(*it, "=", strGetKey, strGetVal);
    if (bRet && strGetKey == strKey)
    {
      strValue = strGetVal;
      return true;
    }
  }

  return false;
}


std::string StringUtils::Left(const std::string& str, size_t count)
{
  count = std::max((size_t) 0, std::min(count, str.size()));
  return str.substr(0, count);
}


std::string StringUtils::Mid(const std::string& str, size_t first, size_t count /* = std::string::npos */)
{
  if (first + count > str.size())
    count = str.size() - first;

  if (first > str.size())
    return std::string();

  return str.substr(first, count);
}


std::string StringUtils::Right(const std::string& str, size_t count)
{
  count = std::max((size_t) 0, std::min(count, str.size()));
  return str.substr(str.size() - count);
}


bool StringUtils::StringToInt32(const std::string& str, int32_t& iValue)
{
  char *end;
  iValue = strtol(str.c_str(), &end, 0);

  // Make sure it's not zero-length and it's a proper number (not suffixed with anything): 
  if (*end || end == str.c_str())
    return false;

  return true;
}


bool StringUtils::StringToDouble(const std::string& str, double& dblValue)
{
  char *end;
  dblValue = strtod(str.c_str(), &end);

  // Make sure it's not zero-length and it's a proper number (not suffixed with anything): 
  if (*end || end == str.c_str())
    return false;

  return true;
}


bool StringUtils::EqualsNoCase(const char *s1, const char *s2)
{
  while (1)
  {
    const char c1 = *s1++; // const local variable should help compiler to optimize
    const char c2 = *s2++;
    if (tolower(c1) != tolower(c2))
      return false;

    // Until end of strings:
    if (c1 == '\0')
      break;
  }
  return true;
}


bool StringUtils::EqualsNoCase(const std::string& str1, const std::string& str2)
{
  return EqualsNoCase(str1.c_str(), str2.c_str());
}


bool StringUtils::StartsWith(const std::string& str1, const std::string& str2)
{
  return (str1.size() >= str2.size() && Left(str1, str2.size()) == str2);
}

void StringUtils::Replace(std::string& str, const char& cOld, const char& cNew)
{
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
  {
    if (*it == cOld)
    {
      *it = cNew;
    }
  }
}

void StringUtils::Replace(std::string& str, const std::string& strOld, const std::string& strNew)
{
  if (strOld.empty())
    return;

  size_t pos = 0;
  
  while (pos < str.size() && (pos = str.find(strOld, pos)) != std::string::npos)
  {
    str.replace(pos, strOld.size(), strNew);
    pos += strNew.size();
  }
}


std::string StringUtils::ToUpper(const std::string& str)
{
  std::string strResult = str;

  for (std::string::iterator it = strResult.begin(); it != strResult.end(); ++it)
  {
    *it = toupper(*it);
  }

  return strResult;
}

std::string StringUtils::ToLower(const std::string& str)
{
  std::string strResult = str;

  for (std::string::iterator it = strResult.begin(); it != strResult.end(); ++it)
  {
    *it = tolower(*it);
  }

  return strResult;
}


std::string StringUtils::IntToHex(const uint32_t& iDecimal, const int8_t& iMinLen /* = 0 */)
{
  std::stringstream stream;
  if (iMinLen > 0)
    stream  << std::setfill ('0') << std::setw(iMinLen) << std::hex << iDecimal;
  else
    stream  << std::hex << iDecimal;

  std::string strHex = stream.str();

  return StringUtils::ToUpper(strHex);
}


bool StringUtils::HexToByte(const std::string& strHex, uint8_t& Byte)
{
  char *endptr;
  Byte = (uint8_t) strtol(strHex.c_str(), &endptr, 16) & 0xFF;

  return !(*endptr || endptr != strHex);
}


bool StringUtils::HexToWord(const std::string& strHex, uint16_t& Word)
{
  char *endptr;
  Word = (uint16_t) strtol(strHex.c_str(), &endptr, 16) & 0xFFFF;

  return !(*endptr || endptr != strHex);
}


/**
 * Trim leading characters provided in chars (spaces, tabs and newline (linefeed and carriage return), etc.).
 */
std::string StringUtils::TrimLeft(const std::string& str, const char *chars /* = " \t\n\r" */)
{
  if (str.length() == 0)
    return str;

  const size_t pos = str.find_first_not_of(chars);
  std::string str2(str);
  if (pos > 0 && pos != std::string::npos)
  {
    str2.erase(0, pos);
  }

  return str2;
}


/**
 * Trim trailing characters provided in chars (spaces, tabs and newline (linefeed and carriage return), etc.).
 */
std::string StringUtils::TrimRight(const std::string& str, const char *chars /* = " \t\n\r" */)
{
  if (str.length() == 0)
    return str;

  const size_t pos = str.find_last_not_of(chars);
  std::string str2(str);
  if (pos < str.length() && pos != std::string::npos)
  {
    str2.erase(pos + 1, std::string::npos);
  }

  return str2;
}


/**
 * Trim trailing/leading characters provided in chars (spaces, tabs and newline (linefeed and carriage return), etc.).
 */
std::string StringUtils::Trim(const std::string& str, const char *chars /* = " \t\n\r" */)
{
  return TrimRight(TrimLeft(str, chars), chars);
}


/**
 * Convert an int32_t to a std::string (like to_string() in C++11)
 */
std::string StringUtils::Int32ToString(const int32_t& iValue)
{
  std::stringstream s;

  // Convert int to string
  s << iValue;

  return s.str();
}
