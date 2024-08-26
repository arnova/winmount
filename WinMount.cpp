/*
  WinMount - Windows Mount Utility
  (C) Copyright 2002-2024 by Arno van Amersfoort

  Web             : https://github.com/arnova/winmount
  Email           : a r n o DOT v a n DOT a m e r s f o o r t AT g m a i l DOT c o m
                    (note: you must remove all spaces and substitute the @ and the . at the proper locations!)

  Target compiler : GCC/G++ or Visual Studio 2022
  C++ standard    : C++11
  Dependencies    : CmdArguments.h StringUtils.h
  Initial date    : December 10, 2002
  Last modified   : August 26, 2024
*/

#include "WinMount.h"
#include "CmdArguments.h"
#include "stringutils.h"

#include <winnetwk.h>

#include <iostream> // For std::cerr/cout
#include <conio.h>  // For _kbhit & _getch()

#pragma comment(lib, "mpr.lib")

// Number of retries (when --retry is NOT used!)
#define RETRY_COUNT 10

// Delay in ms between retries
#define RETRY_DELAY 3000

const char *VERSION = "1.50c";


void ShowHelp()
{
  std::cerr << "Usage: winmount.exe [options] [mount_file]" << std::endl << std::endl;
  std::cerr << "[mount_file]        : Path to the mount configuration file (default = \\mount.ini)" << std::endl;
  std::cerr << "-h|--help           : This screen" << std::endl;
  std::cerr << "-i|--interactive    : Force interactive mode" << std::endl;
  std::cerr << "-p|--persist        : Remember connections (persist)" << std::endl;
  std::cerr << "-u|--unmount        : Unmount (existing) drives before mount" << std::endl;
  std::cerr << "-r|--retry          : Retry until all connections are successfully mounted (if not specified, retry 10 times)" << std::endl;
}


static void ArgumentSyntaxError(const std::string& strArgument)
{
  ShowHelp();
  std::cerr << "ERROR: Syntax error in argument \"" << strArgument << "\"" << std::endl << std::endl;
}


static void ArgumentInvalidValueForOption(const std::string& strArgument)
{
  ShowHelp();
  std::cerr << "ERROR: Invalid value for option in argument \"" << strArgument << "\"" << std::endl << std::endl;
}


static void ArgumentNoValueForOption(const std::string& strArgument)
{
  ShowHelp();
  std::cerr << "ERROR: Invalid to specify value for option in argument \"" << strArgument << "\"" << std::endl << std::endl;
}


static void ArgumentValueEmpty(const std::string& strArgument)
{
  ShowHelp();
  std::cerr << "ERROR: Missing value for option in argument \"" << strArgument << "\"" << std::endl << std::endl;
}


static void ArgumentBadOption(const std::string& strArgument)
{
  ShowHelp();
  std::cerr << "ERROR: Bad option in \"" << strArgument << "\"" << std::endl << std::endl;
}


std::string ShowError(const int error_code)
{
  std::string strError;

  // Info from http://msdn.microsoft.com/en-us/library/ms681381.aspx

  switch (error_code)
  {
    case ERROR_SUCCESS                        : strError = "OK"; break;
    case ERROR_PATH_NOT_FOUND                 : strError = "Path not found (3)"; break;
    case ERROR_ACCESS_DENIED                  : strError = "Access denied (5)"; break;
    case ERROR_UNEXP_NET_ERR                  : strError = "Unexpected network error (59)"; break;
    case ERROR_NETNAME_DELETED                : strError = "The specified network name is no longer available (64)"; break;
    case ERROR_NETWORK_ACCESS_DENIED          : strError = "Network access is denied (65)"; break;
    case ERROR_BAD_DEV_TYPE                   : strError = "Incorrect network resource (66)"; break;
    case ERROR_BAD_NET_NAME                   : strError = "Bad network name (67)"; break;
    case ERROR_ALREADY_ASSIGNED               : strError = "Drive already assigned (85)"; break;
    case ERROR_INVALID_PASSWORD               : strError = "The specified network password is not correct (86)"; break;
    case ERROR_BUSY                           : strError = "The requested resource is busy (170)"; break;
    case ERROR_DEVICE_ALREADY_REMEMBERED      : strError = "Local device already in use (1202)"; break;
    case ERROR_NO_NET_OR_BAD_PATH             : strError = "Network path not found or not available (1203)"; break;
    case ERROR_SESSION_CREDENTIAL_CONFLICT    : strError = "Credential conflict (1219)"; break;
    case ERROR_NO_NETWORK                     : strError = "The network is not present or not started (1222)"; break;
    case ERROR_CANCELLED                      : strError = "User cancelled (1223)"; break;
    case ERROR_NETWORK_UNREACHABLE            : strError = "Network unreachable (1231)"; break;
    case ERROR_PORT_UNREACHABLE               : strError = "Destination port unreachable (1234)"; break;
    case ERROR_LOGON_FAILURE                  : strError = "Bad user name or password (1326)"; break;
    case ERROR_CANT_ACCESS_DOMAIN_INFO        : strError = "Cannot access domain info (1351)"; break;
    case ERROR_NOT_CONNECTED                  : strError = "Network connection does not exist (2250)"; break;
    default                                   : strError = "Unknown error (" + std::to_string(error_code) + ")";
  }

  return strError;
}


/* Process the command line */
bool CWinMount::ProcessCommandLine(const std::vector<std::string>& vecArgs)
{
 CCmdArguments arguments(vecArgs);

  while (arguments.ProcessArgument())
  {
    const std::string& strArgument = arguments.GetArgument();

    if (arguments.ArgumentIsOption())
    {
      if (arguments.TestOption("help", "h"))
      {
        if (arguments.OptionHasValue())
        {
          ArgumentNoValueForOption(strArgument);
        }
        else
        {
          ShowHelp();
        }
        return false;
      }
      else if (arguments.TestOption("persist", "p"))
      {
        if (arguments.OptionHasValue())
        {
          ArgumentNoValueForOption(strArgument);
          return false;
        }
        m_dwConnectFlags |= CONNECT_UPDATE_PROFILE;
      }
      else if (arguments.TestOption("interactive", "i"))
      {
        if (arguments.OptionHasValue())
        {
          ArgumentNoValueForOption(strArgument);
          return false;
        }
        m_dwConnectFlags |= CONNECT_INTERACTIVE;
      }
      else if (arguments.TestOption("unmount", "u"))
      {
        if (arguments.OptionHasValue())
        {
          ArgumentNoValueForOption(strArgument);
          return false;
        }
        m_bUnmount = true;
      }
      else if (arguments.TestOption("retry", "r"))
      {
        if (arguments.OptionHasValue())
        {
          ArgumentNoValueForOption(strArgument);
          return false;
        }
        m_bRetryForever = true;
      }
      else
      {
        // Invalid option
        ArgumentBadOption(strArgument);
        return false;
      }
    }
    else if (!m_strIniFile.size())
    {
      m_strIniFile = StringUtils::Trim(strArgument, "\"\'");
    }
    else
    {
      ArgumentSyntaxError(strArgument);
      return false;
    }
  }

  // Success:
  return true;
}


bool CWinMount::ProcessIniFile()
{
  // If not configuration file is specified, fallback to \mount.ini
  if (!m_strIniFile.size())
    m_strIniFile = "\\mount.ini";

  m_fStream.open(m_strIniFile, std::ios::in);
  if (!m_fStream.is_open())
  {
    const std::string strMsg = "WinMount: An error occurred opening the configuration file " + m_strIniFile + ". Program aborted";
    MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
    return false;
  }

  int iLineCount = 0;
  while (!m_fStream.eof())
  {
    iLineCount++;

    // Read line from mount.ini into strLine
    std::string strLine;
    std::getline(m_fStream, strLine);

    if (m_fStream.bad())
    {
      const std::string strMsg = "WinMount: An error occurred reading config-file " + m_strIniFile + " at line " + std::to_string(iLineCount) + ". Program aborted";
      MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
      return false;
    }

    // Only process line if not a comment and not empty
    if (strLine.size() && strLine[0] != ';' && strLine.size())
    {
      std::string strLocal, strRemote;
      if (!StringUtils::Split(strLine, " ", strLocal, strRemote) || strLocal.size() != 2 || strLocal[1] != ':' || strRemote.size() < 3 || strRemote.substr(0,2) != "\\\\" || strRemote.size() > MAX_PATH)
      {
        const std::string strMsg = "WinMount: Line " + std::to_string(iLineCount) + " in config-file " + m_strIniFile + " is invalid. Program aborted";
        MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
        return false;
      }

      CNetShare netShare(strLocal,strRemote);
      m_vecNetShares.push_back(netShare);
    }
  }

  return true;
}


bool CWinMount::AllDrivesMapped() const
{
  bool bMappedAll = true;
  for (auto& netShare : m_vecNetShares)
  {
    if (!netShare.IsMapped())
      bMappedAll = false;
  }

  return bMappedAll;
}


bool CWinMount::MapDrives()
{
  for (int iRetryCount = 0; (m_bRetryForever || iRetryCount < RETRY_COUNT); iRetryCount++)
  {
    if (iRetryCount != 0)
    {
      Sleep(RETRY_DELAY);

//      std::cout << "Retry " << iRetryCount << ":" << std::endl;
    }

    for (auto& netShare : m_vecNetShares)
    {
      if (!netShare.IsMapped())
      {
        bool bTryInteractive = false;
        const std::string& strLocal = netShare.GetLocalName();
        const std::string& strRemote = netShare.GetRemoteName();

        NETRESOURCE nr;       // NETResource structure
        // Assign values to the NETRESOURCE structure
        nr.dwType = RESOURCETYPE_ANY;
        nr.lpLocalName = (LPSTR) strLocal.c_str();    // LPSTR = *char
        nr.lpRemoteName = (LPSTR) strRemote.c_str();  // LPSTR = *char
        nr.lpProvider = NULL;

        std::cout << "> Connecting " << strRemote << " to " << strLocal << "...";
        if (m_bUnmount)
        {
          if (_kbhit() && _getch() == 0x1B) // Abort on <ESC>
          {
            std::cout << "User cancelled..." << std::endl;
            return false;
          }

          // Terminate any existing mounts with this drive letter
          const DWORD result = WNetCancelConnection2(nr.lpLocalName, 0, TRUE);
          if (result != NO_ERROR && result != ERROR_NOT_CONNECTED)
          {
            std::cout << "Unable to unmount existing connection" << std::endl;

            const std::string strMsg = ShowError(result) + "\nUnable to disconnect " + strLocal;
            MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
            
            netShare.SetMapped(); // Flag as mapped, else we'll keep trying over and over again
            continue;
          }
        }

        if ( !(m_dwConnectFlags & CONNECT_INTERACTIVE) )
        {
          if (_kbhit() && _getch() == 0x1B) // Abort on <ESC>
          {
            std::cout << "User cancelled..." << std::endl;
            return false;
          }

          const DWORD result = WNetAddConnection2(&nr, NULL, NULL, m_dwConnectFlags);

          if (result == ERROR_CANCELLED || result == NO_ERROR || result == ERROR_ALREADY_ASSIGNED)
          {
            std::cout << ShowError(result) << std::endl;

            netShare.SetMapped();
            continue;
          }
          else if (result == ERROR_DEVICE_ALREADY_REMEMBERED || result == ERROR_SESSION_CREDENTIAL_CONFLICT || result == ERROR_ALREADY_ASSIGNED)
          {
            // Unable to retry in interactive mode with errors above:
            std::cout << "FATAL: " << ShowError(result) << std::endl;

            const std::string strMsg = ShowError(result) + "\nUnable to connect " + strRemote + " to " + strLocal;
            MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);

            netShare.SetMapped(); // Flag as mapped, else we'll keep trying over and over again
            continue;
          }

          // NOTE: ERROR_BAD_DEV_TYPE(66) occurs when host is unavailable so don't enable interactive for that to allow retrying
          if (result != ERROR_LOGON_FAILURE && result != ERROR_BAD_DEV_TYPE)
          {
            std::cout << "Non-fatal: " << ShowError(result) << "." << std::endl << "  Retry in interactive mode..." << std::endl;
            bTryInteractive = true;
          }
          else
          {
            std::cout << ShowError(result) << std::endl;
          }
        }

        // (Try) interactive mode?
        if ((m_dwConnectFlags & CONNECT_INTERACTIVE) || bTryInteractive)
        {
          DWORD result = 0;
          do
          {
            // (Retry) Call the WNetAddConnection2 function to assign a drive letter to the share (Prompt for username/pwd)
            // Optionally add "| CONNECT_UPDATE_PROFILE"
            result = WNetAddConnection2(&nr, NULL, NULL, CONNECT_INTERACTIVE | CONNECT_PROMPT | m_dwConnectFlags);
            if (result == ERROR_NETWORK_UNREACHABLE || result == ERROR_NO_NET_OR_BAD_PATH)
            {
//              std::cout << show_error(result) << ".";
              const std::string strMsg = ShowError(result) + "\nUnable to connect " + strRemote + " to " + strLocal;
              MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
            }
          } while (result == ERROR_NETWORK_UNREACHABLE || result == ERROR_NO_NET_OR_BAD_PATH); // Only retry on network error

          std::cout << ShowError(result) << std::endl;

          if (result == ERROR_CANCELLED)
          {
            std::cout << std::endl;
            //netShare.SetMapped(); // Flag as mapped, else we'll keep trying over and over again
            return false;
          }
          else if (result != NO_ERROR)
          {
            const std::string strMsg = ShowError(result) + "\nUnable to connect " + strRemote + " to " + strLocal;
            MessageBox(0, strMsg.c_str(), "Error", MB_OK + MB_ICONERROR);
          }
        }
      }
    }

    if (AllDrivesMapped())
      return true; // We're done
  }

  std::cout << std::endl;

  return false;
}


// ** Program entry point **
int main(int argc, char *argv[])
{
  std::cout << "WinMount v" << VERSION << " - (C) Copyright 2002-2024" << std::endl;
  std::cout << "Written by Arno van Amersfoort" << std::endl << std::endl;

 // Store arguments in a std::string std::vector
  std::vector<std::string> args(argv + 1, argv + argc);
  CWinMount WinMount;

  /* Process the command line */
  if (!WinMount.ProcessCommandLine(args))
    return EXIT_FAILURE;

  if (!WinMount.ProcessIniFile())
    return EXIT_FAILURE;

  if (!WinMount.MapDrives())
    return EXIT_FAILURE;

  return EXIT_SUCCESS; // Success :-)
}
