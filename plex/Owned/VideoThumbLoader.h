#pragma once
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

#include <map>
#include "ThumbLoader.h"
#include "utils/JobManager.h"
#include "FileItem.h"
#include "Utility/PlexJobs.h"

class IStreamDetailsObserver;

class CVideoThumbLoader : public CThumbLoader, public CJobQueue
{
public:
  CVideoThumbLoader();
  virtual ~CVideoThumbLoader();

  virtual void OnLoaderStart();
  virtual void OnLoaderFinish();

  virtual bool LoadItem(CFileItem* pItem);

  virtual bool FillThumb(CFileItem &item);
  static std::string GetLocalArt(const CFileItem &item, const std::string &type, bool checkFolder = false);
  static std::vector<std::string> GetArtTypes(const std::string &type);
  static void SetArt(CFileItem &item, const std::map<std::string, std::string> &artwork);
};

class CPlexThumbCacher : public CJobQueue
{
public:
  CPlexThumbCacher() : CJobQueue(true, 5, CJob::PRIORITY_LOW), m_stop(false) {};
  ~CPlexThumbCacher() {}
  void Stop() { m_stop = true; CancelJobs(); }
  void Start() { m_stop = false; }
  void Load(const CFileItemList &list)
  {
    if (m_stop)
      return;

    for (int i = 0; i < list.Size(); i++)
    {
      CFileItemPtr item = list.Get(i);
      if (item->IsPlexMediaServer())
        AddJob(new CPlexVideoThumbLoaderJob(item));
    }
  }

  bool m_stop;
};