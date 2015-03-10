#include "GUIWindowPlexMyChannels.h"
#include "interfaces/Builtins.h"
#include "utils/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
CGUIWindowPlexMyChannels::CGUIWindowPlexMyChannels()
  : CGUIPlexMediaWindow(WINDOW_PLEX_MYCHANNELS, "MyChannels.xml")
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIWindowPlexMyChannels::OnSelect(int iItem)
{
  if (iItem < 0 || iItem >= (int)m_vecItems->Size())
    return true;
  CFileItemPtr pItem = m_vecItems->Get(iItem);

  if (!pItem->HasProperty("mediaWindow"))
    return CGUIPlexMediaWindow::OnClick(iItem);

  std::string window = pItem->GetProperty("mediaWindow").asString();
  std::string action = "XBMC.ActivateWindow(" + window + "," + pItem->GetPath() + ",return)";

  CLog::Log(LOGDEBUG, "CGUIWindowPlexMyChannels::OnClick executing action %s", action.c_str());

  CBuiltins::Execute(action);

  return true;
}
