#include "CmdArguments.h"

// Constructor
CCmdArguments::CCmdArguments(const std::vector<std::string> vecStrArgs) : m_vecStrArgs(vecStrArgs), m_index(0)
{
  m_count = vecStrArgs.size();
}


bool CCmdArguments::ArgumentIsOption(void) const
{
  if (m_strArgument.size() && (m_strArgument[0] == '-' || m_strArgument[0] == '+' || m_strArgument[0] == '/'))
  {
    return true;
  }
  
  return false;
}


// FIXME: How to skip extra argument if --key value is used?!
bool CCmdArguments::ProcessArgument(void)
{
  if (m_index < m_count)
  {
    // Get argument and increase index
    m_strArgument = m_vecStrArgs[m_index++];

    if (ArgumentIsOption())
    {
      const size_t sep = m_strArgument.find_first_of('=');
      if (sep != std::string::npos && sep > 0)
      {
        m_strOptionName = m_strArgument.substr(0, sep);
        m_strOptionValue = m_strArgument.substr(sep + 1); // Value from key=value, if any
      }
      else
      {
        m_strOptionName = m_strArgument;
        m_strOptionValue = "";
      }
    }
    else
    {
      m_strOptionName = "";
      m_strOptionValue = "";
    }

    return true;
  }

  // No arguments left:
  return false;
}


std::string CCmdArguments::GetArgument(void) const
{
  return m_strArgument;
}


std::string CCmdArguments::GetOption(void) const
{
  return m_strOptionName;
}


bool CCmdArguments::OptionHasValue(void) const
{
  if (m_strOptionValue.size())
  {
    return true;
  }

  return false;
}


bool CCmdArguments::NextArgumentIsOption(void) const
{
  if ((m_index + 1) < m_count && m_vecStrArgs[m_index + 1].size() && (m_vecStrArgs[m_index + 1][0] == '-' || m_vecStrArgs[m_index + 1][0] == '+' || m_vecStrArgs[m_index + 1][0] == '/'))
  {
    return true;
  }
  
  return false;
}


// WARNING: Only call this once!
bool CCmdArguments::GetOptionValue(std::string& strValue)
{
  if (OptionHasValue())
  {
    strValue = m_strOptionValue;
    return true;
  }

  // This is a "hack" to get an assigned value when eg. --option value is used (without a '=')
  if ((m_index + 1) < m_count && !NextArgumentIsOption())
  {
    m_index++;
    strValue = m_vecStrArgs[m_index];
    return true;
  }

  return false;
}


bool CCmdArguments::TestOption(const std::string& strLong, const std::string& strShort /* = "" */) const
{
  if (strLong.size() && (m_strOptionName == "+" + strLong || m_strOptionName == "--" + strLong))
  {
    return true;
  }

  if (strShort.size() && (m_strOptionName == "-" + strShort || m_strOptionName == "/" + strShort))
  {
    return true;
  }

  return false;
}