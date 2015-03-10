//
//  PlexDirectoryTypeParserPicture.cpp
//  Plex
//
//  Created by Tobias Hieta <tobias@plexapp.com> on 2013-06-21.
//  Copyright 2013 Plex Inc. All rights reserved.
//

#include "PlexDirectoryTypeParserPicture.h"
#include "PlexAttributeParser.h"
#include "pictures/PictureInfoTag.h"
#include "utils/StringUtils.h"

void CPlexDirectoryTypeParserPicture::Process(CFileItem &item, CFileItem &mediaContainer, XML_ELEMENT *itemElement)
{
  ParseMediaNodes(item, itemElement);
  if (item.m_mediaItems.size() > 0 && item.m_mediaItems[0]->m_mediaParts.size() > 0)
  {
    CFileItemPtr media = item.m_mediaItems[0];
    /* Pass the path URL via the transcoder, because XBMC raw file reader sucks */
    CPlexAttributeParserMediaUrl mUrl;
    mUrl.Process(item.GetURL(), "picture", media->m_mediaParts[0]->GetProperty("unprocessed_key").asString(), &item);
    item.SetPath(item.GetArt("picture"));

    CPictureInfoTag* tag = item.GetPictureInfoTag();

    std::string res = StringUtils::Format("%s,%s", media->GetProperty("width").asString().c_str(), media->GetProperty("height").asString().c_str());
    tag->SetInfo(SLIDE_RESOLUTION, res);

    if (media->HasProperty("aperture"))
      tag->SetInfo(SLIDE_EXIF_APERTURE, media->GetProperty("aperture").asString());
    if (media->HasProperty("exposure"))
      tag->SetInfo(SLIDE_EXIF_EXPOSURE, media->GetProperty("exposure").asString());
    if (media->HasProperty("iso"))
      tag->SetInfo(SLIDE_EXIF_ISO_EQUIV, media->GetProperty("iso").asString());
    if (media->HasProperty("make"))
      tag->SetInfo(SLIDE_EXIF_CAMERA_MAKE, media->GetProperty("make").asString());
    if (media->HasProperty("model"))
      tag->SetInfo(SLIDE_EXIF_CAMERA_MODEL, media->GetProperty("model").asString());
  }
}
