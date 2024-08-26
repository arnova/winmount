#pragma once
#ifndef CMD_ARGUMENTS
#define CMD_ARGUMENTS

#include <string>
#include <vector>

class CCmdArguments
{
  public:
    CCmdArguments(const std::vector<std::string> vecStrArgs);
    ~CCmdArguments(void) {}; // Empty destructor

    bool ArgumentIsOption(void) const;
    bool ProcessArgument(void);
    std::string GetArgument(void) const;
    std::string GetOption(void) const;

    bool OptionHasValue(void) const;
    bool NextArgumentIsOption(void) const;
    bool GetOptionValue(std::string& strValue);
    bool TestOption(const std::string& strLong, const std::string& strShort = "") const;

  private:
    std::vector<std::string> m_vecStrArgs;

    size_t m_index;
    size_t m_count;

    std::string m_strArgument;
    std::string m_strOptionName;
    std::string m_strOptionValue;
};

#endif // CMD_ARGUMENTS
