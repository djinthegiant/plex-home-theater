#pragma once

/*
 *      Copyright (C) 2005-2008 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <map>
#include <string>

#include "guilib/GUIWindow.h"
#include "guilib/GUIStaticItem.h"
#include "utils/Job.h"
#include <boost/timer.hpp>

#include "Owned/VideoThumbLoader.h"
#include "Owned/MusicThumbLoader.h"
#include "Owned/PictureThumbLoader.h"

#include "Utility/PlexTimer.h"
#include "Utility/PlexNavigationHelper.h"
#include "Utility/PlexGlobalTimer.h"
#include "PlexSectionFanout.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "GUI/GUIPlexWindowFocusSaver.h"

#include "listproviders/StaticProvider.h"

// List IDs.
#define CONTEXT_BUTTON_SLEEP (CONTEXT_BUTTON_RATING + 1)
#define CONTEXT_BUTTON_QUIT (CONTEXT_BUTTON_RATING + 2)
#define CONTEXT_BUTTON_SHUTDOWN (CONTEXT_BUTTON_RATING + 3)

class CGUIWindowHome;

class CGUIWindowHome : public CGUIWindow, public IPlexGlobalTimeout, public IJobCallback
{
public:
  CGUIWindowHome(void);
  virtual ~CGUIWindowHome(void) {}
  virtual bool OnMessage(CGUIMessage& message);
  void Render() { CGUIWindow::Render(); }
  void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions) { CGUIWindow::Process(currentTime, dirtyregions); }

private:
  virtual bool OnAction(const CAction &action);
  virtual bool CheckTimer(const std::string& strExisting, const std::string& strNew, int title, int line1, int line2);
  virtual CFileItemPtr GetCurrentListItem(int offset = 0);

  void HideAllLists();
  void RestoreSection();
  void RefreshSection(const std::string& url, CPlexSectionFanout::SectionTypes type);
  void RefreshAllSections(bool force = true);
  void RefreshSectionsForServer(const std::string &uuid);
  void RemoveSectionsForServer(const std::string &uuid);
  void AddSection(const std::string& url, CPlexSectionFanout::SectionTypes sectionType, bool useGlobalSlideshow);
  void RemoveSection(const std::string& url);
  bool ShowSection(const std::string& url);
  bool ShowCurrentSection();
  bool GetContentTypesFromSection(const std::string& url, std::vector<int> &types);
  bool GetContentListFromSection(const std::string& url, int contentType, CFileItemList &list);
  void SectionNeedsRefresh(const std::string& url);
  void OpenItem(CFileItemPtr item);
  bool OnClick(const CGUIMessage& message);
  void OnSectionLoaded(const CGUIMessage& message);
  void OnWatchStateChanged(const CGUIMessage& message);

  void AddPlaylists(std::vector<CGUIListItemPtr>& list, bool& updated);
  void AddPlayQueues(std::vector<CGUIListItemPtr>& list,  bool& updated);
  int GetPlayQueueType();

  void OnJobComplete(unsigned int jobID, bool success, CJob *job);

  void OnTimeout();
  std::string TimerName() const { return "windowHome"; }

  std::string GetCurrentItemName(bool onlySections=false);
  CFileItem* GetCurrentFileItem();
  CFileItemPtr GetCurrentFanoutItem();

  // Context menu
  virtual bool OnPopupMenu();
  void GetContextMenu(CContextButtons &buttons);
  void GetSleepContextMenu(CContextButtons &buttons);
  void GetItemContextMenu(CContextButtons &buttons, const CFileItem &item);
  void GetPlayQueueContextMenu(CContextButtons &buttons, const CFileItem &item);
  void ChangeWatchState(int choice);
  void HandleItemDelete();
  void RemoveFromPlayQueue();

  void UpdateSections();
  CGUIStaticItemPtr ItemToSection(CFileItemPtr item);
    
  bool                       m_globalArt;
  
  CGUIListItemPtr            m_videoChannelItem;
  CGUIListItemPtr            m_musicChannelItem;
  CGUIListItemPtr            m_photoChannelItem;
  CGUIListItemPtr            m_applicationChannelItem;

  std::map<std::string, CPlexSectionFanout*> m_sections;
  
  std::string                 m_lastSelectedItem;
  std::string                 m_currentFanArt;
  std::string                 m_lastSelectedSubItem;
  CEvent                     m_loadNavigationEvent;
  bool                       m_cacheLoadFail;
  CPlexNavigationHelper      m_navHelper;

  // focus saving members
  CGUIPlexWindowFocusSaver  m_focusSaver;
};

