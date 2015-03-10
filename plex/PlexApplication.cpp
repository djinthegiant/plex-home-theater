/*
 *  PlexApplication.cpp
 *  XBMC
 *
 *  Created by Jamie Kirkpatrick on 20/01/2011.
 *  Copyright 2014 Plex Inc. All rights reserved.
 *
 */

#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Client/PlexNetworkServiceBrowser.h"
#include "PlexApplication.h"
#include "GUIUserMessages.h"
#include "Client/MyPlex/MyPlexManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "Client/PlexServerManager.h"
#include "Client/PlexServerDataLoader.h"
#include "Remote/PlexRemoteSubscriberManager.h"
#include "Client/PlexMediaServerClient.h"
#include "interfaces/AnnouncementManager.h"
#include "Client/PlexTimelineManager.h"
#include "PlexThemeMusicPlayer.h"
#include "Owned/VideoThumbLoader.h"
#include "Filters/PlexFilterManager.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "guilib/GUIWindowManager.h"
#include "Utility/PlexProfiler.h"
#include "Client/PlexTranscoderClient.h"
#include "FileSystem/PlexDirectoryCache.h"
#include "GUI/GUIPlexDefaultActionHandler.h"
#include "Client/PlexExtraInfoLoader.h"
#include "Playlists/PlexPlayQueueManager.h"
#include "GUI/GUIWindowStartup.h"
#include "utils/log.h"

////////////////////////////////////////////////////////////////////////////////
void PlexApplication::Start()
{
  std::string uuid = CSettings::Get().GetString("system.uuid");
  if (uuid.empty())
  {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    CSettings::Get().SetString("system.uuid", boost::lexical_cast<std::string>(uuid));
  }

  timer = CPlexGlobalTimerPtr(new CPlexGlobalTimer);

  myPlexManager = new CMyPlexManager;

  dataLoader = CPlexServerDataLoaderPtr(new CPlexServerDataLoader);
  serverManager = CPlexServerManagerPtr(new CPlexServerManager);
  remoteSubscriberManager = new CPlexRemoteSubscriberManager;
  mediaServerClient = CPlexMediaServerClientPtr(new CPlexMediaServerClient);
  timelineManager = CPlexTimelineManagerPtr(new CPlexTimelineManager);
  themeMusicPlayer = CPlexThemeMusicPlayerPtr(new CPlexThemeMusicPlayer);
  thumbCacher = new CPlexThumbCacher;
  filterManager = CPlexFilterManagerPtr(new CPlexFilterManager);
  profiler = CPlexProfilerPtr(new CPlexProfiler);
  extraInfo = new CPlexExtraInfoLoader;
  playQueueManager = CPlexPlayQueueManagerPtr(new CPlexPlayQueueManager);
  directoryCache = CPlexDirectoryCachePtr(new CPlexDirectoryCache);
  defaultActionHandler = CGUIPlexDefaultActionHandlerPtr(new CGUIPlexDefaultActionHandler);

  serverManager->load();

  ANNOUNCEMENT::CAnnouncementManager::Get().AddAnnouncer(this);

  if (g_advancedSettings.m_bEnableGDM)
    m_serviceListener = CPlexServiceListenerPtr(new CPlexServiceListener);

  // Add the manual server if it exists and is enabled.
  if (CSettings::Get().GetBool("plexmediaserver.manualaddress"))
  {
    std::string address = CSettings::Get().GetString("plexmediaserver.address");
    if (PlexUtils::IsValidIP(address))
    {
      PlexServerList list;
      CPlexServerPtr server = CPlexServerPtr(new CPlexServer("", address, 32400));
      list.push_back(server);
      g_plexApplication.serverManager->UpdateFromConnectionType(list,
                                                                CPlexConnection::CONNECTION_MANUAL);
    }
  }

  myPlexManager->Create();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlexApplication::preShutdown()
{
  ANNOUNCEMENT::CAnnouncementManager::Get().RemoveAnnouncer(this);

  remoteSubscriberManager->Stop();
  timer->StopAllTimers();
  themeMusicPlayer->stop();
  if (m_serviceListener)
  {
    m_serviceListener->Stop();
    m_serviceListener.reset();
  }
  myPlexManager->Stop();
  serverManager->Stop();
  dataLoader->Stop();
  timelineManager->Stop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlexApplication::Shutdown()
{
  CLog::Log(LOGINFO, "CPlexApplication shutting down!");

  SAFE_DELETE(extraInfo);

  SAFE_DELETE(myPlexManager);

  timer.reset();

  serverManager.reset();
  dataLoader.reset();

  timelineManager.reset();

  mediaServerClient->CancelJobs();
  mediaServerClient.reset();

  profiler->Clear();
  profiler.reset();

  filterManager->saveFiltersToDisk();
  filterManager.reset();

  CPlexTranscoderClient::DeleteInstance();

  directoryCache.reset();
  defaultActionHandler.reset();

  themeMusicPlayer.reset();
  playQueueManager.reset();

  SAFE_DELETE(remoteSubscriberManager);

  SAFE_DELETE(thumbCacher);
}

////////////////////////////////////////////////////////////////////////////////////////
void PlexApplication::Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char* sender,
                               const char* message, const CVariant& data)
{
  CLog::Log(LOGDEBUG, "PlexApplication::Announce got message %s:%s", sender, message);

  if (flag == ANNOUNCEMENT::Player && stricmp(sender, "xbmc") == 0)
  {
    if (stricmp(message, "OnPlay") == 0)
    {
      m_triedToRestart = false;
    }
    else if (stricmp(message, "OnStop") == 0)
    {
      CPlexPlayQueuePtr pq = g_plexApplication.playQueueManager->getPlayQueueOfType(PLEX_MEDIA_TYPE_VIDEO);
      if (pq)
      {
        CFileItemList list;
        CFileItemPtr lastItem;

        if (pq->get(list) && list.Get(list.Size() - 1))
          lastItem = list.Get(list.Size() - 1);

        if (lastItem && lastItem->HasMusicInfoTag() && g_application.CurrentFileItemPtr() &&
            lastItem->GetProperty("playQueueItemID").asInteger() ==
            g_application.CurrentFileItemPtr()->GetProperty("playQueueItemID").asInteger(-1))
        {
          CLog::Log(LOGDEBUG, "PlexApplication::Announce clearing video playQueue");
          g_plexApplication.playQueueManager->clear();
        }
      }
    }
  }

  if ((stricmp(message, "OnScreensaverDeactivated") == 0) && (stricmp(sender, "xbmc") == 0))
  {
    if (!g_application.m_pPlayer->IsPlaying() && g_plexApplication.myPlexManager->IsPinProtected() && !CSettings::Get().GetBool("myplex.automaticlogin"))
    {
      CLog::Log(LOGDEBUG, "PlexApplication::Announce resuming from screensaver");
      g_windowManager.ActivateWindow(WINDOW_STARTUP_ANIM);

      CGUIWindowStartup *window = (CGUIWindowStartup*)g_windowManager.GetWindow(WINDOW_STARTUP_ANIM);
      if (window)
        window->allowEscOut(false);
    }
  }
  else if (flag == ANNOUNCEMENT::System && !strcmp(sender, "xbmc") && !strcmp(message, "OnWake"))
  {
    /* Scan servers */
    if (m_serviceListener)
      m_serviceListener->ScanNow();
    myPlexManager->Poke();
  }
}
