
#include "PlexFile.h"
#include "utils/StringUtils.h"
#include "utils/log.h"
#include "settings/Settings.h"
#include "GUIInfoManager.h"
#include "LangInfo.h"

using namespace XFILE;

CPlexFile::CPlexFile(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

CPlexFile::~CPlexFile(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

bool CPlexFile::Get(const CURL &url, std::string& data)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  m_customrequest.clear();
  m_postdata.clear();
  m_postdataset = false;
  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Service(newUrl.Get(), data);
  return false;
}

bool CPlexFile::Post(const CURL &url, const std::string& postData, std::string& data)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  m_customrequest.clear();
  m_postdata = postData;
  m_postdataset = true;
  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Service(newUrl.Get(), data);
  return false;
}

bool CPlexFile::Delete(const CURL &url, std::string& data)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  m_customrequest = "DELETE";
  m_postdata.clear();
  m_postdataset = false;
  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Service(newUrl.Get(), data);
  return false;
}

bool CPlexFile::BuildHTTPURL(CURL& url)
{
  // Only rewrite plexserver:// urls
  if (!url.IsProtocol("plexserver"))
    return true;

  // Replace hostname with name/ip from settings
  if (url.GetHostName() == "plex.server") {
    std::string hostname = CSettings::Get().GetString("plex.server");
    if (hostname.empty())
      return false;
    url.SetHostName(hostname);
  }

  // Change from plexserver://<hostname>/ to http://<hostname>:32400/
  url.SetProtocol("http");
  url.SetPort(32400);

  // Remove special hubIdentifier protocol option used to parse a single hub
  url.RemoveProtocolOption("hubIdentifier");

  // Remove special hubTarget protocol option used to modify target path
  url.RemoveProtocolOption("hubTarget");
  
  // Add plex headers
  url.SetProtocolOption("X-Plex-Client-Identifier", CSettings::Get().GetString("system.uuid"));
  url.SetProtocolOption("X-Plex-Device-Name", CSettings::Get().GetString("services.devicename"));
  url.SetProtocolOption("X-Plex-Product", g_infoManager.GetAppName());
  url.SetProtocolOption("X-Plex-Version", g_infoManager.GetVersionShort());
  url.SetProtocolOption("X-Plex-Language", g_langInfo.GetLanguageLocale());

  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());
  return true;
}

bool CPlexFile::Open(const CURL &url)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  if (IsLocalArt(url)) {
    return false;
  }

  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Open(newUrl);
  return false;
}

int CPlexFile::Stat(const CURL& url, struct __stat64* buffer)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  if (IsLocalArt(url)) {
    return -1;
  }

  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Stat(newUrl, buffer);
  return -1;
}

bool CPlexFile::Exists(const CURL &url)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  if (IsLocalArt(url)) {
    return false;
  }

  CURL newUrl(url);
  if (BuildHTTPURL(newUrl))
    return CCurlFile::Exists(newUrl);
  return false;
}

bool CPlexFile::IsLocalArt(const CURL &url)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  // Skip art files that will not exist
  const std::string& fileName = url.GetFileName();
  if (StringUtils::EndsWith(fileName, "/folder.jpg") ||
    StringUtils::EndsWith(fileName, "/folder.png") ||
    StringUtils::EndsWith(fileName, "/thumb.jpg") ||
    StringUtils::EndsWith(fileName, "/thumb.png") ||
    StringUtils::EndsWith(fileName, "/poster.jpg") ||
    StringUtils::EndsWith(fileName, "/poster.png") ||
    StringUtils::EndsWith(fileName, "/fanart.jpg") ||
    StringUtils::EndsWith(fileName, "/fanart.png") ||
    StringUtils::EndsWith(fileName, "/banner.jpg") ||
    StringUtils::EndsWith(fileName, "/banner.png")) {
    return true;
  }

  return false;
}
