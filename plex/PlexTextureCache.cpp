#include "utils/URIUtils.h"
#include "PlexUtils.h"
#include "settings/Settings.h"
#include "PlexTextureCache.h"
#include "utils/log.h"
#include "filesystem/File.h"
#include "Utility/PlexJobs.h"
#include "utils/StringUtils.h"

using namespace XFILE;

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexTextureCache::Deinitialize()
{
  CancelJobs();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCache::GetCachedTexture(const CStdString &url, CTextureDetails &details)
{
  CStdString fileprefix = CTextureCache::GetCacheFile(url);
  CStdString path = CTextureCache::GetCachedPath(fileprefix);

  // first check if we have a jpg matching
  if (CFile::Exists(path + ".jpg"))
  {
    details.file = fileprefix + ".jpg";
    //details.hash = StringUtils::Right(fileprefix, 8); // Only set hash when the file should be recached
    return true;
  }

  // if not we might have a png
  if (CFile::Exists(path + ".png"))
  {
    details.file = fileprefix + ".png";
    //details.hash = StringUtils::Right(fileprefix, 8); // Only set hash when the file should be recached
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCache::AddCachedTexture(const CStdString &url, const CTextureDetails &details)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexTextureCache::IncrementUseCount(const CTextureDetails &details)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCache::SetCachedTextureValid(const CStdString &url, bool updateable)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCache::ClearCachedTexture(const CStdString &url, CStdString &cachedFile)
{
  CTextureDetails details;
  if (GetCachedTexture(url, details))
  {
    cachedFile = details.file;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCache::ClearCachedTexture(int id, CStdString &cachedURL)
{
  return false;
}
