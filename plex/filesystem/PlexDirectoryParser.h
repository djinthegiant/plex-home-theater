#pragma once

#include "FileItem.h"
#include "video/VideoInfoTag.h"
#include "utils/XBMCTinyXML.h"

class PlexDirectoryParser
{
public:
  static bool ParseMediaContainer(const TiXmlElement* root, CFileItemList& items, const CURL& url);
private:
  static bool ParseFiles(const TiXmlElement* root, CFileItemList& items, const CURL& url);
  static bool ParseMovies(const TiXmlElement* root, CFileItemList& items, const CURL& url);
  static bool ParseTvShows(const TiXmlElement* root, CFileItemList& items, const CURL& url);
  static bool ParseSeasons(const TiXmlElement* root, CFileItemList& items, const CURL& url);
  static bool ParseEpisodes(const TiXmlElement* root, CFileItemList& items, const CURL& url);

  static CVideoInfoTag GetDetailsForMovie(const TiXmlElement* element, const CURL& url);
  static CVideoInfoTag GetDetailsForTvShow(const TiXmlElement* element, const CURL& url, CFileItem* item);
  static CVideoInfoTag GetDetailsForSeason(const TiXmlElement* root, const TiXmlElement* element, const CURL& url, CFileItem* item);
  static CVideoInfoTag GetDetailsForEpisode(const TiXmlElement* root, const TiXmlElement* element, const CURL& url);

  static bool GetStreamDetails(const TiXmlElement* element, CVideoInfoTag& tag, const CURL& url);

  static bool ParseArt(const TiXmlElement* element, CFileItemPtr& item, const char* type, const char* tag, const CURL& url);

  static std::string GetFullPath(const CURL& url, const std::string& path);
};
