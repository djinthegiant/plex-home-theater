#include "PlexRemoteNavigationHandler.h"
#include "PlexApplication.h"
#include "ApplicationMessenger.h"
#include "Application.h"
#include "guilib/GUIAudioManager.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/Key.h"
#include <vector>

#define LEGACY 1

////////////////////////////////////////////////////////////////////////////////////////
CPlexRemoteResponse CPlexRemoteNavigationHandler::handle(const std::string &url, const ArgMap &arguments)
{
  int action = ACTION_NONE;
  int activeWindow = g_windowManager.GetFocusedWindow();

  std::string navigation = url.substr(19, url.length() - 19);

  if (navigation == "moveRight")
    action = ACTION_MOVE_RIGHT;
  else if (navigation == "moveLeft")
    action = ACTION_MOVE_LEFT;
  else if (navigation == "moveDown")
    action = ACTION_MOVE_DOWN;
  else if (navigation == "moveUp")
    action = ACTION_MOVE_UP;
  else if (navigation == "select")
    action = ACTION_SELECT_ITEM;
  else if (navigation == "music")
  {
    if (g_application.m_pPlayer->IsPlayingAudio() && activeWindow != WINDOW_VISUALISATION)
      action = ACTION_SHOW_GUI;
  }
  else if (navigation == "home")
  {
    std::vector<std::string> args;
    g_application.WakeUpScreenSaverAndDPMS();
    CApplicationMessenger::Get().ActivateWindow(WINDOW_HOME, args, false);
    return CPlexRemoteResponse();
  }
  else if (navigation == "back")
  {
    if (g_application.IsPlayingFullScreenVideo() &&
        (activeWindow != WINDOW_DIALOG_AUDIO_OSD_SETTINGS &&
         activeWindow != WINDOW_DIALOG_VIDEO_OSD_SETTINGS &&
         activeWindow != WINDOW_DIALOG_PLEX_AUDIO_PICKER &&
         activeWindow != WINDOW_DIALOG_PLEX_SUBTITLE_PICKER))
      action = ACTION_STOP;
    else
      action = ACTION_NAV_BACK;
  }

#ifdef LEGACY
  else if (navigation == "contextMenu")
    action = ACTION_CONTEXT_MENU;
  else if (navigation == "toggleOSD")
    action = ACTION_SHOW_OSD;
  else if (navigation == "pageUp")
    action = ACTION_PAGE_UP;
  else if (navigation == "pageDown")
    action = ACTION_PAGE_DOWN;
  else if (navigation == "nextLetter")
    action = ACTION_NEXT_LETTER;
  else if (navigation == "previousLetter")
    action = ACTION_PREV_LETTER;
#endif

  if (action != ACTION_NONE)
  {
    CAction actionId(action);

    g_application.ResetScreenSaverTimer();
    g_application.WakeUpScreenSaverAndDPMS();
    g_application.ResetSystemIdleTimer();

    if (!g_application.m_pPlayer->IsPlaying())
      g_audioManager.PlayActionSound(actionId);

    CApplicationMessenger::Get().SendAction(actionId, WINDOW_INVALID, false);
  }

  return CPlexRemoteResponse();
}
