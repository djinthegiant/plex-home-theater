/*
 *  Copyright (C) 2011 Plex, Inc.
 *
 *  Created on: Apr 10, 2011
 *      Author: Elan Feingold
 */

#pragma once

#include "settings/Settings.h"
#include "NetworkServiceAdvertiser.h"
#include "PlexUtils.h"
#include "GUIInfoManager.h"
#include "utils/StringUtils.h"

/////////////////////////////////////////////////////////////////////////////
class PlexNetworkServiceAdvertiser : public NetworkServiceAdvertiser
{
 public:
  
  /// Constructor.
  PlexNetworkServiceAdvertiser(boost::asio::io_service& ioService)
    : NetworkServiceAdvertiser(ioService, NS_BROADCAST_ADDR, NS_PLEX_MEDIA_CLIENT_PORT) {}
  
 protected:
  
  /// For subclasses to fill in.
  virtual void createReply(map<string, string>& headers) 
  {
    headers["Name"] = CSettings::Get().GetString("services.devicename");
    headers["Port"] = StringUtils::Format("%d", CSettings::Get().GetInt("services.webserverport"));
    headers["Version"] = g_infoManager.GetVersion();
    headers["Product"] = PLEX_TARGET_NAME;
    headers["Protocol"] = "plex";
    headers["Protocol-Version"] = "1";
    headers["Protocol-Capabilities"] = PLEX_HOME_THEATER_CAPABILITY_STRING;
    headers["Device-Class"] = "HTPC";
  }
  
  /// For subclasses to fill in.
  virtual string getType()
  {
    return "plex/media-player";
  }
  
  virtual string getResourceIdentifier() { return CSettings::Get().GetString("system.uuid"); }
  virtual string getBody() { return ""; }
};
