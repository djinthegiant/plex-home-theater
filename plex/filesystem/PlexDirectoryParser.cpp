
#include "PlexDirectoryParser.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"
#include "utils/PlexXMLUtils.h"
#include "utils/StringUtils.h"
#include "URL.h"

bool PlexDirectoryParser::ParseMediaContainer(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  items.SetLabel(XMLUtils::GetAttribute(root, "title1"));
  std::string title2 = XMLUtils::GetAttribute(root, "title2");
  if (!title2.empty())
    items.SetLabel(items.GetLabel() + ": " + title2);

  int total = 0;
  PlexXMLUtils::GetInt(root, "size", total);
  items.SetProperty("total", total);
  items.Reserve(total);

  std::string type = XMLUtils::GetAttribute(root, "viewGroup");
  if (type == MediaTypeMovie)
    return ParseMovies(root, items, url);
  else if (type == MediaTypeTvShow || type == "show")
    return ParseTvShows(root, items, url);
  else if (type == MediaTypeSeason)
    return ParseSeasons(root, items, url);
  else if (type == MediaTypeEpisode)
    return ParseEpisodes(root, items, url);
  //else if (type == MediaTypeArtist)
  //	return ParseArtists(root, items, url);
  //else if (type == MediaTypeAlbum)
  //	return ParseAlbums(root, items, url);
  //else if (type == MediaTypeSong || type == "track")
  //	return ParseSongs(root, items, url);
  return ParseFiles(root, items, url);
}

bool PlexDirectoryParser::ParseArt(const TiXmlElement* element, CFileItemPtr& item, const char* type, const char* tag, const CURL& url)
{
  std::string value = XMLUtils::GetAttribute(element, tag);
  if (!value.empty()) {
    item->SetArt(type, GetFullPath(url, value));
    return true;
  }
  return false;
}

bool PlexDirectoryParser::ParseFiles(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  CLabelFormatter formatter("%H. %T", "");

  std::string morePath;
  if (root->ValueStr() == "Hub")
  {
    int more = 0;
    if (PlexXMLUtils::GetInt(root, "more", more) && more > 0)
    {
      morePath = GetFullPath(url, XMLUtils::GetAttribute(root, "key"));
      items.SetProperty("more", morePath);
    }
  }

  for (const TiXmlElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string key = XMLUtils::GetAttribute(element, "key");
    if (key.empty())
      continue;

    CFileItemPtr item(new CFileItem);
    item->m_bIsFolder = element->ValueStr() == "Directory" || element->ValueStr() == "Hub";
    item->SetProperty("plex", true);
    item->SetProperty("plexserver", url.GetHostName());
    item->SetPath(GetFullPath(url, key));
    std::string hubIdentifier = XMLUtils::GetAttribute(element, "hubIdentifier");
    if (!hubIdentifier.empty())
    {
      item->SetProperty("hubIdentifier", hubIdentifier);
    }
    std::string type = XMLUtils::GetAttribute(element, "type");
    if (type == MediaTypeMovie && element->ValueStr() == "Video")
    {
      CVideoInfoTag details = GetDetailsForMovie(element, url);
      item->SetFromVideoInfoTag(details);
      ParseArt(element, item, "poster", "thumb", url);
      ParseArt(element, item, "fanart", "art", url);
      if (item->IsResumePointSet()) {
        item->m_lStartOffset = details.m_resumePoint.timeInSeconds * 75.0;
      }
      item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0);
      item->SetProperty("type", MediaTypeMovie);
    }
    else if (type == MediaTypeEpisode && element->ValueStr() == "Video")
    {
      CVideoInfoTag details = GetDetailsForEpisode(element, element, url);
      item->SetFromVideoInfoTag(details);
      formatter.FormatLabel(item.get());
      //ParseArt(element, item, "thumb", "thumb", url);
      ParseArt(element, item, "poster", "grandparentThumb", url);
      ParseArt(element, item, "fanart", "art", url);
      if (item->IsResumePointSet()) {
        item->m_lStartOffset = details.m_resumePoint.timeInSeconds * 75.0;
      }
      item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0);
      item->m_dateTime = details.m_firstAired;
      item->SetProperty("type", MediaTypeEpisode);
    }
    else if (type == MediaTypeSeason && element->ValueStr() == "Directory")
    {
      CVideoInfoTag details = GetDetailsForSeason(element, element, url, item.get());
      item->SetFromVideoInfoTag(details);
      ParseArt(element, item, "thumb", "thumb", url);
      ParseArt(element, item, "poster", "parentThumb", url);
      ParseArt(element, item, "fanart", "art", url);
      item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0 && details.m_iEpisode > 0);
      item->SetProperty("type", MediaTypeSeason);
    }
    else
    {
      item->SetLabel(XMLUtils::GetAttribute(element, "title"));
      ParseArt(element, item, "thumb", "thumb", url);
      ParseArt(element, item, "fanart", "art", url);
    }
    // HACK: Until support for Container(id).Property(name) is added
    if (!morePath.empty())
    {
      item->SetProperty("more", morePath);
    }
    items.Add(item);
  }

  items.SetContent("files");
  return true;
}

bool PlexDirectoryParser::ParseMovies(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  for (const TiXmlElement* element = root->FirstChildElement("Video"); element; element = element->NextSiblingElement("Video"))
  {
    CVideoInfoTag details = GetDetailsForMovie(element, url);
    CFileItemPtr item(new CFileItem(details));
    item->SetProperty("plex", true);
    item->SetProperty("plexserver", url.GetHostName());
    ParseArt(element, item, "thumb", "thumb", url);
    ParseArt(element, item, "fanart", "art", url);
    if (item->IsResumePointSet()) {
      item->m_lStartOffset = details.m_resumePoint.timeInSeconds * 75.0;
    }
    item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0);
    items.Add(item);
  }

  items.SetContent("movies");
  return true;
}

bool PlexDirectoryParser::ParseTvShows(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  for (const TiXmlElement* element = root->FirstChildElement("Directory"); element; element = element->NextSiblingElement("Directory"))
  {
    CFileItemPtr item(new CFileItem());
    CVideoInfoTag details = GetDetailsForTvShow(element, url, item.get());
    item->SetFromVideoInfoTag(details);
    item->SetProperty("plex", true);
    item->SetProperty("plexserver", url.GetHostName());
    ParseArt(element, item, "poster", "thumb", url);
    ParseArt(element, item, "fanart", "art", url);
    ParseArt(element, item, "banner", "banner", url);
    item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0 && details.m_iEpisode > 0);
    items.Add(item);
  }

  items.SetContent("tvshows");
  return true;
}

bool PlexDirectoryParser::ParseSeasons(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  for (const TiXmlElement* element = root->FirstChildElement("Directory"); element; element = element->NextSiblingElement("Directory"))
  {
    CFileItemPtr item(new CFileItem());
    CVideoInfoTag details = GetDetailsForSeason(root, element, url, item.get());
    item->SetFromVideoInfoTag(details);
    item->SetProperty("plex", true);
    item->SetProperty("plexserver", url.GetHostName());
    ParseArt(element, item, "thumb", "thumb", url);
    ParseArt(root, item, "poster", "parentThumb", url);
    ParseArt(root, item, "fanart", "art", url);
    ParseArt(root, item, "banner", "banner", url);
    item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0 && details.m_iEpisode > 0);
    items.Add(item);
  }

  items.SetContent("seasons");
  return true;
}

bool PlexDirectoryParser::ParseEpisodes(const TiXmlElement* root, CFileItemList& items, const CURL& url)
{
  CLabelFormatter formatter("%H. %T", "");

  for (const TiXmlElement* element = root->FirstChildElement("Video"); element; element = element->NextSiblingElement("Video"))
  {
    CVideoInfoTag details = GetDetailsForEpisode(root, element, url);
    CFileItemPtr item(new CFileItem(details));
    formatter.FormatLabel(item.get());
    item->SetProperty("plex", true);
    item->SetProperty("plexserver", url.GetHostName());
    ParseArt(element, item, "thumb", "thumb", url);
    ParseArt(element, item, "poster", "grandparentThumb", url) ||
      ParseArt(root, item, "poster", "thumb", url);
    ParseArt(element, item, "fanart", "art", url) ||
      ParseArt(root, item, "fanart", "art", url);
    if (item->IsResumePointSet()) {
      item->m_lStartOffset = details.m_resumePoint.timeInSeconds * 75.0;
    }
    item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, details.m_playCount > 0);
    item->m_dateTime = details.m_firstAired;
    items.Add(item);
  }

  items.SetContent("episodes");
  return true;
}

std::string PlexDirectoryParser::GetFullPath(const CURL& url, const std::string& path)
{
  if (StringUtils::StartsWith(path, "http://") ||
    StringUtils::StartsWith(path, "https://"))
  {
    return path;
  }

  if (StringUtils::StartsWith(path, "/"))
  {
    std::string basePath(url.GetWithoutFilename());
    URIUtils::RemoveSlashAtEnd(basePath);
    return basePath + path;
  }

  if (url.HasProtocolOption("hubTarget"))
  {
    std::string hubTarget = url.GetProtocolOption("hubTarget");
    CURL newUrl(url);
    newUrl.SetProtocolOptions("");
    std::string basePath(newUrl.Get());
    URIUtils::RemoveSlashAtEnd(basePath);
    return basePath + "/" + path + "/" + hubTarget;
  }

  std::string basePath(url.Get());
  URIUtils::RemoveSlashAtEnd(basePath);
  return basePath + "/" + path;
}

bool PlexDirectoryParser::GetStreamDetails(const TiXmlElement* element, CVideoInfoTag& tag, const CURL& url)
{
  // TODO: support for multiple Media elements
  const TiXmlElement* media = element->FirstChildElement("Media");
  if (!media) {
    return false;
  }

  CStreamDetails& details = tag.m_streamDetails;
  details.Reset();

  CStreamDetailAudio *p1 = new CStreamDetailAudio();
  PlexXMLUtils::GetString(media, "audioCodec", p1->m_strCodec);
  PlexXMLUtils::GetInt(media, "audioChannels", p1->m_iChannels);
  //PlexXMLUtils::GetString(media, "languageCode", p1->m_strLanguage);
  StringUtils::ToLower(p1->m_strCodec);
  tag.m_streamDetails.AddStream(p1);

  CStreamDetailVideo *p2 = new CStreamDetailVideo();
  PlexXMLUtils::GetString(media, "videoCodec", p2->m_strCodec);
  PlexXMLUtils::GetFloat(media, "aspectRatio", p2->m_fAspect);
  PlexXMLUtils::GetInt(media, "width", p2->m_iWidth);
  PlexXMLUtils::GetInt(media, "height", p2->m_iHeight);
  PlexXMLUtils::GetInt(media, "duration", p2->m_iDuration);
  if (p2->m_iDuration) {
    p2->m_iDuration = p2->m_iDuration / 1000;
  }
  StringUtils::ToLower(p2->m_strCodec);
  tag.m_streamDetails.AddStream(p2);

  const TiXmlElement* part = media->FirstChildElement("Part");
  if (part) {
    PlexXMLUtils::GetInt(part, "id", tag.m_iFileId);
    tag.m_strFileNameAndPath = GetFullPath(url, XMLUtils::GetAttribute(part, "key"));
  }

  details.DetermineBestStreams();

  if (details.GetVideoDuration() > 0)
    tag.m_duration = details.GetVideoDuration();

  return true;
}

CVideoInfoTag PlexDirectoryParser::GetDetailsForMovie(const TiXmlElement* element, const CURL& url)
{
  CVideoInfoTag details;
  details.m_type = MediaTypeMovie;
  PlexXMLUtils::GetInt(element, "ratingKey", details.m_iDbId);
  PlexXMLUtils::GetString(element, "key", details.m_strUniqueId);
  PlexXMLUtils::GetString(element, "title", details.m_strTitle);
  PlexXMLUtils::GetString(element, "originalTitle", details.m_strOriginalTitle);
  PlexXMLUtils::GetString(element, "titleSort", details.m_strSortTitle);
  PlexXMLUtils::GetString(element, "summary", details.m_strPlot);
  PlexXMLUtils::GetString(element, "tagline", details.m_strTagLine);
  PlexXMLUtils::GetFloat(element, "rating", details.m_fRating);
  PlexXMLUtils::GetInt(element, "year", details.m_iYear);
  PlexXMLUtils::GetString(element, "contentRating", details.m_strMPAARating);
  PlexXMLUtils::GetInt(element, "duration", details.m_duration);
  if (details.m_duration) {
    details.m_duration = details.m_duration / 1000;
  }

  PlexXMLUtils::GetDate(element, "originallyAvailableAt", details.m_premiered);
  PlexXMLUtils::GetInt(element, "viewCount", details.m_playCount);
  PlexXMLUtils::GetTimeStamp(element, "lastViewedAt", details.m_lastPlayed);
  PlexXMLUtils::GetTimeStamp(element, "addedAt", details.m_dateAdded);

  int resumePoint = 0;
  PlexXMLUtils::GetInt(element, "viewOffset", resumePoint);
  if (resumePoint) {
    details.m_resumePoint.timeInSeconds = resumePoint / 1000.0;
    PlexXMLUtils::GetInt(element, "duration", resumePoint);
    details.m_resumePoint.totalTimeInSeconds = resumePoint / 1000.0;
    details.m_resumePoint.type = CBookmark::RESUME;
  }

  PlexXMLUtils::GetString(element, "studio", details.m_studio);
  PlexXMLUtils::GetStringArray(element, "Genre", details.m_genre);
  PlexXMLUtils::GetStringArray(element, "Writer", details.m_writingCredits);
  PlexXMLUtils::GetStringArray(element, "Director", details.m_director);
  PlexXMLUtils::GetStringArray(element, "Country", details.m_country);
  PlexXMLUtils::GetActorInfoArray(element, "Role", details.m_cast);

  GetStreamDetails(element, details, url);
  return details;
}

CVideoInfoTag PlexDirectoryParser::GetDetailsForTvShow(const TiXmlElement* element, const CURL& url, CFileItem* item)
{
  CVideoInfoTag details;
  details.m_type = MediaTypeTvShow;
  details.m_strPath = GetFullPath(url, XMLUtils::GetAttribute(element, "key"));
  PlexXMLUtils::GetInt(element, "ratingKey", details.m_iDbId);
  PlexXMLUtils::GetInt(element, "ratingKey", details.m_iIdShow);
  PlexXMLUtils::GetString(element, "key", details.m_strUniqueId);
  PlexXMLUtils::GetString(element, "title", details.m_strShowTitle);
  PlexXMLUtils::GetString(element, "title", details.m_strTitle);
  PlexXMLUtils::GetString(element, "originalTitle", details.m_strOriginalTitle);
  PlexXMLUtils::GetString(element, "titleSort", details.m_strSortTitle);
  PlexXMLUtils::GetString(element, "summary", details.m_strPlot);
  PlexXMLUtils::GetFloat(element, "rating", details.m_fRating);
  PlexXMLUtils::GetInt(element, "year", details.m_iYear);
  PlexXMLUtils::GetString(element, "contentRating", details.m_strMPAARating);
  PlexXMLUtils::GetInt(element, "duration", details.m_duration);
  if (details.m_duration) {
    details.m_duration = details.m_duration / 1000;
  }

  PlexXMLUtils::GetInt(element, "leafCount", details.m_iEpisode);
  PlexXMLUtils::GetInt(element, "viewedLeafCount", details.m_playCount);

  PlexXMLUtils::GetDate(element, "originallyAvailableAt", details.m_premiered);
  PlexXMLUtils::GetTimeStamp(element, "lastViewedAt", details.m_lastPlayed);
  PlexXMLUtils::GetTimeStamp(element, "addedAt", details.m_dateAdded);

  PlexXMLUtils::GetString(element, "studio", details.m_studio);
  PlexXMLUtils::GetStringArray(element, "Genre", details.m_genre);
  PlexXMLUtils::GetStringArray(element, "Writer", details.m_writingCredits);
  PlexXMLUtils::GetStringArray(element, "Director", details.m_director);
  PlexXMLUtils::GetStringArray(element, "Country", details.m_country);
  PlexXMLUtils::GetActorInfoArray(element, "Role", details.m_cast);

  if (item != NULL)
  {
    item->m_dateTime = details.m_premiered;
    int childCount = 0;
    PlexXMLUtils::GetInt(element, "childCount", childCount);
    item->SetProperty("totalseasons", childCount);
    item->SetProperty("totalepisodes", details.m_iEpisode);
    item->SetProperty("numepisodes", details.m_iEpisode); // will be changed later to reflect watchmode setting
    item->SetProperty("watchedepisodes", details.m_playCount);
    item->SetProperty("unwatchedepisodes", details.m_iEpisode - details.m_playCount);
  }
  details.m_playCount = (details.m_iEpisode <= details.m_playCount) ? 1 : 0;

  return details;
}

CVideoInfoTag PlexDirectoryParser::GetDetailsForSeason(const TiXmlElement* root, const TiXmlElement* element, const CURL& url, CFileItem* item)
{
  CVideoInfoTag details;
  details.m_type = MediaTypeSeason;
  details.m_strPath = GetFullPath(url, XMLUtils::GetAttribute(element, "key"));
  PlexXMLUtils::GetInt(element, "ratingKey", details.m_iDbId);
  PlexXMLUtils::GetInt(element, "parentRatingKey", details.m_iIdShow) ||
    PlexXMLUtils::GetInt(root, "key", details.m_iIdShow);
  PlexXMLUtils::GetString(element, "key", details.m_strUniqueId);
  PlexXMLUtils::GetString(root, "parentTitle", details.m_strShowTitle);
  PlexXMLUtils::GetString(element, "title", details.m_strTitle);
  PlexXMLUtils::GetString(element, "originalTitle", details.m_strOriginalTitle);
  PlexXMLUtils::GetString(element, "titleSort", details.m_strSortTitle);
  PlexXMLUtils::GetString(element, "summary", details.m_strPlot);
  PlexXMLUtils::GetInt(root, "parentYear", details.m_iYear);

  // TODO: m_premiered, m_genre, m_studio, m_strMPAARating

  PlexXMLUtils::GetInt(element, "index", details.m_iSeason);
  PlexXMLUtils::GetInt(element, "leafCount", details.m_iEpisode);
  PlexXMLUtils::GetInt(element, "viewedLeafCount", details.m_playCount);

  PlexXMLUtils::GetTimeStamp(element, "lastViewedAt", details.m_lastPlayed);
  PlexXMLUtils::GetTimeStamp(element, "addedAt", details.m_dateAdded);

  if (item != NULL)
  {
    item->SetProperty("totalepisodes", details.m_iEpisode);
    item->SetProperty("numepisodes", details.m_iEpisode); // will be changed later to reflect watchmode setting
    item->SetProperty("watchedepisodes", details.m_playCount);
    item->SetProperty("unwatchedepisodes", details.m_iEpisode - details.m_playCount);
    if (details.m_iSeason == 0)
      item->SetProperty("isspecial", true);
  }
  details.m_playCount = (details.m_iEpisode <= details.m_playCount) ? 1 : 0;

  return details;
}

CVideoInfoTag PlexDirectoryParser::GetDetailsForEpisode(const TiXmlElement* root, const TiXmlElement* element, const CURL& url)
{
  CVideoInfoTag details;
  details.m_type = MediaTypeEpisode;
  PlexXMLUtils::GetInt(element, "ratingKey", details.m_iDbId);
  //PlexXMLUtils::GetInt(element, "grandparentRatingKey", details.m_iIdShow);
  PlexXMLUtils::GetInt(element, "parentRatingKey", details.m_iIdSeason);
  PlexXMLUtils::GetString(element, "key", details.m_strUniqueId);
  PlexXMLUtils::GetString(element, "grandparentTitle", details.m_strShowTitle) ||
    PlexXMLUtils::GetString(root, "grandparentTitle", details.m_strShowTitle);
  PlexXMLUtils::GetString(element, "title", details.m_strTitle);
  PlexXMLUtils::GetString(element, "originalTitle", details.m_strOriginalTitle);
  PlexXMLUtils::GetString(element, "titleSort", details.m_strSortTitle);
  PlexXMLUtils::GetString(element, "summary", details.m_strPlot);
  PlexXMLUtils::GetFloat(element, "rating", details.m_fRating);
  PlexXMLUtils::GetInt(element, "year", details.m_iYear);
  PlexXMLUtils::GetString(element, "contentRating", details.m_strMPAARating) ||
    PlexXMLUtils::GetString(root, "grandparentContentRating", details.m_strMPAARating);
  PlexXMLUtils::GetInt(element, "duration", details.m_duration);
  if (details.m_duration) {
    details.m_duration = details.m_duration / 1000;
  }

  PlexXMLUtils::GetInt(element, "parentIndex", details.m_iSeason) ||
    PlexXMLUtils::GetInt(root, "parentIndex", details.m_iSeason);
  PlexXMLUtils::GetInt(element, "index", details.m_iEpisode);

  PlexXMLUtils::GetDate(element, "originallyAvailableAt", details.m_firstAired);
  PlexXMLUtils::GetInt(element, "viewCount", details.m_playCount);
  PlexXMLUtils::GetTimeStamp(element, "lastViewedAt", details.m_lastPlayed);
  PlexXMLUtils::GetTimeStamp(element, "addedAt", details.m_dateAdded);

  int resumePoint = 0;
  PlexXMLUtils::GetInt(element, "viewOffset", resumePoint);
  if (resumePoint) {
    details.m_resumePoint.timeInSeconds = resumePoint / 1000.0;
    PlexXMLUtils::GetInt(element, "duration", resumePoint);
    details.m_resumePoint.totalTimeInSeconds = resumePoint / 1000.0;
    details.m_resumePoint.type = CBookmark::RESUME;
  }

  PlexXMLUtils::GetString(element, "studio", details.m_studio);
  PlexXMLUtils::GetStringArray(element, "Genre", details.m_genre);
  PlexXMLUtils::GetStringArray(element, "Writer", details.m_writingCredits);
  PlexXMLUtils::GetStringArray(element, "Director", details.m_director);
  PlexXMLUtils::GetStringArray(element, "Country", details.m_country);
  PlexXMLUtils::GetActorInfoArray(element, "Role", details.m_cast);

  GetStreamDetails(element, details, url);
  return details;
}
