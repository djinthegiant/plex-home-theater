#pragma once

/*
 *      Copyright (C) 2005-2012 Team XBMC
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

#include "FileItem.h"
#include "view/GUIViewControl.h"
#include "utils/JobManager.h"
#include "video/windows/GUIWindowVideoNav.h"

class CGUIWindowStartup :
      public CGUIMediaWindow, public IJobCallback, public IPlexGlobalTimeout
{
public:
  CGUIWindowStartup(void);
  virtual ~CGUIWindowStartup(void);

  virtual bool OnAction(const CAction &action);
  virtual void OnWindowLoaded();
  virtual bool OnMessage(CGUIMessage &message);
  void OnNumber(unsigned int num);
  void OnBackSpace();
  void PreviousWindow();
  inline void allowEscOut(bool allow) { m_allowEscOut = allow; }

private:
  void SelectUserByName(std::string user);
  void OnUserSelected(CFileItemPtr item);
  void setPinControlText(std::string pin);
  void OnJobComplete(unsigned int jobID, bool success, CJob *job);
  void setUsersList(CFileItemList &userlist);
  void OnTimeout();
  void notifyLoginFailed();

  CFileItemList m_users;
  std::string m_pin;
  std::string m_selectedUser;
  std::string m_selectedUserThumb;
  bool m_allowEscOut;
  std::string m_currentToken;
  int m_fetchUsersJobID;

  CGUIViewControl m_viewControl;
};
