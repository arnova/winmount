#pragma once

#include <string>
#include <vector>
#include <fstream> // For std::ifstream

#include <inttypes.h>
#include <windows.h>

class CNetShare
{
  public:
    CNetShare(const std::string& strLocal, const std::string& strRemote) { m_strLocalName = strLocal; m_strRemoteName = strRemote; }; // Constructor
    ~CNetShare(void) {}; // Destructor (empty)

    bool IsMapped() const { return m_bMapped; };
    void SetMapped(const bool bMapped = true) { m_bMapped = bMapped; };
    std::string GetLocalName() const { return m_strLocalName; };
    std::string GetRemoteName() const { return m_strRemoteName; };

  private:
    std::string m_strLocalName;
    std::string m_strRemoteName;
    bool m_bMapped = false;
};


class CWinMount
{
  public:
    CWinMount(void) {};   // Constructor (empty)
    ~CWinMount(void) {};  // Destructor (empty)
    bool ProcessCommandLine(const std::vector<std::string>& vecArgs);
    bool ProcessIniFile();
    bool AllDrivesMapped() const;
    bool MapDrives();

  private:
    bool m_bUnmount = false;
    bool m_bRetryForever = false;
    DWORD m_dwConnectFlags = 0;

    std::string m_strIniFile;                 // Location of the (mount) ini-file
    std::ifstream m_fStream;
    std::vector<CNetShare> m_vecNetShares;
};
