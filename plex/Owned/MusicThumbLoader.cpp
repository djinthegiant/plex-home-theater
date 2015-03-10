/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "MusicThumbLoader.h"
#include "FileItem.h"
#include "TextureDatabase.h"
#include "music/tags/MusicInfoTag.h"
#include "music/tags/MusicInfoTagLoaderFactory.h"
#include "music/infoscanner/MusicInfoScanner.h"
#include "music/Artist.h"
#include "video/VideoThumbLoader.h"

using namespace std;
using namespace MUSIC_INFO;

CMusicThumbLoader::CMusicThumbLoader() : CThumbLoader()
{
}

CMusicThumbLoader::~CMusicThumbLoader()
{
}

void CMusicThumbLoader::OnLoaderStart()
{
  m_albumArt.clear();
}

void CMusicThumbLoader::OnLoaderFinish()
{
  m_albumArt.clear();
}

bool CMusicThumbLoader::LoadItem(CFileItem* pItem)
{
  if (pItem->m_bIsShareOrDrive)
    return false;

  if (pItem->HasMusicInfoTag() && pItem->GetArt().empty())
  {
    if (FillLibraryArt(*pItem))
      return true;
      
    if (pItem->GetMusicInfoTag()->GetType() == MediaTypeArtist)
      return false; // No fallback
  }

  if (!pItem->HasArt("thumb"))
  {
    // Look for embedded art
    if (pItem->HasMusicInfoTag() && !pItem->GetMusicInfoTag()->GetCoverArtInfo().empty())
    {
      // The item has got embedded art but user thumbs overrule, so check for those first
      if (!FillThumb(*pItem, false)) // Check for user thumbs but ignore folder thumbs
      {
        // No user thumb, use embedded art
        CStdString thumb = CTextureUtils::GetWrappedImageURL(pItem->GetPath(), "music");
        pItem->SetArt("thumb", thumb);
      }
    }
    else
    {
      // Check for user thumbs
      FillThumb(*pItem, true);
    }
  }

  return true;
}

bool CMusicThumbLoader::FillThumb(CFileItem &item, bool folderThumbs /* = true */)
{
  if (item.HasArt("thumb"))
    return true;
  CStdString thumb = GetCachedImage(item, "thumb");
  if (thumb.empty())
  {
    thumb = item.GetUserMusicThumb(false, folderThumbs);
    if (!thumb.empty())
      SetCachedImage(item, "thumb", thumb);
  }
  if (!thumb.empty())
    item.SetArt("thumb", thumb);
  return !thumb.empty();
}

bool CMusicThumbLoader::FillLibraryArt(CFileItem &item)
{
  return false;
}

bool CMusicThumbLoader::GetEmbeddedThumb(const std::string &path, EmbeddedArt &art)
{
  auto_ptr<IMusicInfoTagLoader> pLoader (CMusicInfoTagLoaderFactory::CreateLoader(path));
  CMusicInfoTag tag;
  if (NULL != pLoader.get())
    pLoader->Load(path, tag, &art);

  return !art.empty();
}
