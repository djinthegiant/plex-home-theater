#pragma once

#include "URL.h"
#include "PlexUtils.h"

#include "Client/PlexServer.h"
#include "Client/MyPlex/MyPlexManager.h"

#include <boost/shared_ptr.hpp>

#include "PlexApplication.h"
#include "filesystem/CurlFile.h"
#include "utils/StringUtils.h"

class CPlexConnection;
typedef boost::shared_ptr<CPlexConnection> CPlexConnectionPtr;

class CPlexConnection
{
public:
  enum ConnectionType
  {
    CONNECTION_MANUAL = 0x01,
    CONNECTION_DISCOVERED = 0x02,
    CONNECTION_MYPLEX = 0x04
  };

  enum ConnectionState
  {
    CONNECTION_STATE_UNKNOWN,
    CONNECTION_STATE_UNREACHABLE,
    CONNECTION_STATE_REACHABLE,
    CONNECTION_STATE_UNAUTHORIZED
  };

  CPlexConnection() {}
  CPlexConnection(int type, const std::string& host, int port, const std::string& schema = "http", const std::string& token = "");
  virtual ~CPlexConnection() {}

  static std::string ConnectionTypeName(ConnectionType type);
  static std::string ConnectionStateName(ConnectionState state);

  virtual ConnectionState TestReachability(CPlexServerPtr server);
  CURL BuildURL(const std::string& path) const;

  bool IsLocal() const
  {
    return PlexUtils::IsLocalNetworkIP(m_url.GetHostName());
  }

  CURL GetAddress() const
  {
    return m_url;
  }

  virtual std::string GetAccessToken() const
  {
    return m_token;
  }

  virtual std::string GetAccessTokenParameter() const
  {
    return "X-Plex-Token";
  }
  
  void SetAccessToken(const std::string& token)
  {
    m_token = token;
  }

  std::string toString() const
  {
    std::string fmt;
    fmt = StringUtils::Format("Connection: %s token used: %s type: %02x state: %s",
               m_url.Get().empty() ? "NONE" : m_url.Get().c_str(),
               m_token.empty() ? "NO" : "YES",
               m_type,
               ConnectionStateName(m_state).c_str());
    return fmt;
  }

  void Merge(CPlexConnectionPtr otherConnection);

  /* Setters, getters - Is this java? */
  void SetRefreshed(bool r) { m_refreshed = r; }
  bool GetRefreshed() const { return m_refreshed; }

  bool Equals(const CPlexConnectionPtr &other);

  int m_type;

  XFILE::CCurlFile m_http;

private:
  ConnectionState m_state;

  CURL m_url;
  std::string m_token;

  bool m_refreshed;

};

class CMyPlexConnection : public CPlexConnection
{
  public:
    CMyPlexConnection()
    : CPlexConnection(CPlexConnection::CONNECTION_MYPLEX, "plex.tv", 443, "https") {}

    virtual std::string GetAccessToken() const
    {
      return g_plexApplication.myPlexManager->GetAuthToken();
    }
};
