
#include "PlexDirectory.h"
#include "PlexFile.h"
#include "FileItem.h"
#include "utils/XMLUtils.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"
#include "utils/PlexXMLUtils.h"
#include "utils/log.h"
#include "video/VideoInfoTag.h"
#include "media/MediaType.h"
#include "PlexDirectoryParser.h"

using namespace XFILE;

CPlexDirectory::CPlexDirectory(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

CPlexDirectory::~CPlexDirectory(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

bool CPlexDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  if (!Exists(url))
  {
    CLog::Log(LOGINFO, "%s - directory do not exists <%s>", __FUNCTION__, url.GetRedacted().c_str());
    return false;
  }

  std::string data;
  if (!m_http.Get(url, data))
  {
    CLog::Log(LOGERROR, "%s - failed to load data from <%s>", __FUNCTION__, url.GetRedacted().c_str());
    return false;
  }

  CXBMCTinyXML xmlDoc;
  if (!xmlDoc.Parse(data, m_http.GetContentCharset()) || xmlDoc.Error())
  {
    CLog::Log(LOGERROR, "%s - failed parsing xml doc from <%s>. error: <%d>", __FUNCTION__, url.GetRedacted().c_str(), xmlDoc.ErrorId());
    return false;
  }

  if (!ReadMediaContainer(xmlDoc.RootElement(), items, url))
  {
    CLog::Log(LOGERROR, "%s - failed to read root element from <%s>", __FUNCTION__, url.GetRedacted().c_str());
    return false;
  }

  if (url.GetFileName() == "hubs/")
  {
    // Append sections if hubs requested
    CURL newUrl(url);
    newUrl.SetFileName("library/sections/");
    newUrl.SetOptions("");
    GetDirectory(newUrl, items);
  }

  return true;
}

void CPlexDirectory::CancelDirectory()
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);

  m_http.Cancel();
}

bool CPlexDirectory::Exists(const CURL& url)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  // Skip library/parts/ paths
  if (StringUtils::StartsWith(url.GetFileName(), "library/parts/"))
    return false;

  return true;
}

bool CPlexDirectory::ReadMediaContainer(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  CLog::Log(LOGDEBUG, "%s - %s", __FUNCTION__, url.GetRedacted().c_str());

  if (root == NULL)
  {
    CLog::Log(LOGWARNING, "%s - missing root element at %s", __FUNCTION__, url.GetRedacted().c_str());
    return false;
  }
  else if (root->ValueStr() != "MediaContainer")
  {
    CLog::Log(LOGWARNING, "%s - unknown root element %s at %s", __FUNCTION__, root->ValueStr().c_str(), url.GetRedacted().c_str());
    return false;
  }

  // Set file item properties
  items.SetPath(url.Get()); // TODO: remove options
  items.SetProperty("plex", true);
  items.SetProperty("plexserver", url.GetHostName());

  // Parse MediaContainer element
  if (url.HasProtocolOption("hubIdentifier")) {
    // Only parse requested hub
    for (const TiXmlElement* element = root->FirstChildElement("Hub"); element; element = element->NextSiblingElement("Hub"))
    {
      if (XMLUtils::GetAttribute(element, "hubIdentifier") == url.GetProtocolOption("hubIdentifier")) {
        PlexDirectoryParser::ParseMediaContainer(element, items, url);
        break;
      }
    }
  }
  else {
    PlexDirectoryParser::ParseMediaContainer(root, items, url);
  }

  // Use sort order from server
  items.AddSortMethod(SortByNone, 553, LABEL_MASKS());

  return true;
}
