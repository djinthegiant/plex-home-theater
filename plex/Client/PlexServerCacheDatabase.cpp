#include "PlexServerCacheDatabase.h"

#include "dbwrappers/Database.h"
#include "dbwrappers/dataset.h"
#include "utils/log.h"

#include "utils/StringUtils.h"
#include "Utility/PlexAES.h"
#include "utils/Base64.h"
#include "settings/Settings.h"
#include <boost/foreach.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexServerCacheDatabase::CreateTables()
{
  try
  {
    CLog::Log(LOGINFO, "CPlexServerCacheDatabase::CreateTables create server table");
    m_pDS->exec("CREATE TABLE server ( uuid text primary key, name text, version text, owner text, synced bool, owned bool, home bool, serverClass text, supportsDeletion bool, supportsVideoTranscoding bool, supportsAudioTranscoding bool, transcoderQualities text, transcoderBitrates text, transcoderResolutions text );\n");
    CLog::Log(LOGINFO, "CPlexServerCacheDatabase::CreateTables create connections table");
    m_pDS->exec("CREATE TABLE connections ( serverUUID text, host text, port integer, token text, type integer, scheme text );\n");
    CLog::Log(LOGINFO, "CPlexServerCacheDatabase::CreateTables create connections table index");
    m_pDS->exec("create index connectionUUID on connections ( serverUUID );\n");
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s unable to create tables", __FUNCTION__);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::UpdateOldVersion(int version)
{
  if (version < 3)
  {
    BeginTransaction();
    try
    {
      m_pDS->exec("insert into server (home) values ('1');");
    }
    catch (...)
    {
      m_pDS->exec("alter table server add column home bool");
    }
    
    if (!clearTables())
      return false;
    
    CommitTransaction();
    return true;
  }
  else if (version == 3)
  {
    BeginTransaction();
    clearTables();
    CommitTransaction();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::clearTables()
{
  CLog::Log(LOGINFO, "CPlexServerCacheDatabase::clearTables");

  try
  {
    m_pDS->exec("delete from connections");
    m_pDS->exec("delete from server");
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "CPlexServerCacheDatabase::clearTables failed to empty out the tables");
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::storeServer(const CPlexServerPtr &server)
{
  if (server->GetSynced())
    return true;

  std::string qualities = StringUtils::Join(server->GetTranscoderQualities(), ",");
  std::string bitrates = StringUtils::Join(server->GetTranscoderBitrates(), ",");
  std::string resolutions = StringUtils::Join(server->GetTranscoderResolutions(), ",");

  std::string sql = PrepareSQL("insert into server (uuid, name, version, owner, synced, owned, home, serverClass, supportsDeletion,"
                              "supportsVideoTranscoding, supportsAudioTranscoding, transcoderQualities, transcoderBitrates, transcoderResolutions) "
                              "values ('%s', '%s', '%s', '%s', %i, %i, %i, '%s', %i, %i, %i, '%s', '%s', '%s');",
                              server->GetUUID().c_str(), server->GetName().c_str(), server->GetVersion().c_str(), server->GetOwner().c_str(),
                              server->GetSynced(), server->GetOwned(), server->GetHome(), server->GetServerClass().c_str(), server->SupportsDeletion(),
                              server->SupportsVideoTranscoding(), server->SupportsAudioTranscoding(), qualities.c_str(),
                              bitrates.c_str(), resolutions.c_str());
  try
  {
    m_pDS->exec(sql);
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "CPlexServerCacheDatabase:storeServer failed to store server %s", server->GetName().c_str());
    return false;
  }

  std::vector<CPlexConnectionPtr> connections;
  server->GetConnections(connections);

  BOOST_FOREACH(CPlexConnectionPtr conn, connections)
  {
    if (!storeConnection(server->GetUUID(), conn))
      return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::storeConnection(const std::string &uuid, const CPlexConnectionPtr &connection)
{
  std::string token;
  
  // scramble the token.
  CPlexAES aes(CSettings::Get().GetString("system.uuid"));
  token = Base64::Encode(aes.encrypt(connection->GetAccessToken()));
  
  std::string sql = PrepareSQL("insert into connections (serverUUID, host, port, token, type, scheme) values ('%s', '%s', %i, '%s', %i, '%s');\n",
                              uuid.c_str(), connection->GetAddress().GetHostName().c_str(), connection->GetAddress().GetPort(),
                              token.c_str(), connection->m_type, connection->GetAddress().GetProtocol().c_str());
  try
  {
    m_pDS->exec(sql);
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "CPlexServerCacheDatabase::storeConnection failed to store connection %s", connection->GetAddress().Get().c_str());
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::getCachedServers(std::vector<CPlexServerPtr>& servers)
{
  try
  {
    if (m_pDB.get() == NULL) return false;
    if (m_pDS.get() == NULL) return false;

    m_pDS->query("select * from server;\n");
    while (!m_pDS->eof())
    {
      std::string name = m_pDS->fv("name").get_asString();
      std::string uuid = m_pDS->fv("uuid").get_asString();
      bool owned = m_pDS->fv("owned").get_asBool();
      bool synced = m_pDS->fv("synced").get_asBool();
      bool home = m_pDS->fv("home").get_asBool();

      if (name.empty() || uuid.empty())
      {
        CLog::Log(LOGERROR, "Could not read server from table, skipping");
        m_pDS->next();
        continue;
      }

      CLog::Log(LOGDEBUG, "CPlexServerCacheDatabase::getCachedServers reading server %s", name.c_str());

      CPlexServerPtr server = CPlexServerPtr(new CPlexServer(uuid, name, owned, synced));
      server->SetHome(home);
      server->SetOwner(m_pDS->fv("owner").get_asString());
      server->SetVersion(m_pDS->fv("version").get_asString());
      server->SetSupportsAudioTranscoding(m_pDS->fv("supportsAudioTranscoding").get_asBool());
      server->SetSupportsVideoTranscoding(m_pDS->fv("supportsVideoTranscoding").get_asBool());
      server->SetSupportsDeletion(m_pDS->fv("supportsDeletion").get_asBool());

      std::string resolutions = m_pDS->fv("transcoderResolutions").get_asString();
      if (!resolutions.empty())
      {
        PlexStringVector t = StringUtils::Split(resolutions, ",");
        server->SetTranscoderResolutions(t);
      }

      std::string qualities = m_pDS->fv("transcoderQualities").get_asString();
      if (!qualities.empty())
      {
        PlexStringVector t = StringUtils::Split(qualities, ",");
        server->SetTranscoderQualities(t);
      }

      std::string bitrates = m_pDS->fv("transcoderBitrates").get_asString();
      if (!bitrates.empty())
      {
        PlexStringVector t = StringUtils::Split(bitrates, ",");
        server->SetTranscoderBitrates(t);
      }

      servers.push_back(server);
      m_pDS->next();
    }

    m_pDS->close();

    /* now add connections */
    BOOST_FOREACH(CPlexServerPtr server, servers)
    {
      std::string sql = PrepareSQL("select * from connections where serverUUID='%s';\n", server->GetUUID().c_str());
      m_pDS->query(sql.c_str());

      while (!m_pDS->eof())
      {
        int type = m_pDS->fv("type").get_asInt();
        std::string address = m_pDS->fv("host").get_asString();
        std::string schema = m_pDS->fv("scheme").get_asString();
        int port = m_pDS->fv("port").get_asInt();
        std::string token = m_pDS->fv("token").get_asString();
        
        // unscramble
        CPlexAES aes(CSettings::Get().GetString("system.uuid"));
        token = aes.decrypt(Base64::Decode(token));

        CPlexConnectionPtr connection = CPlexConnectionPtr(new CPlexConnection(type, address, port, schema, token));
        server->AddConnection(connection);

        m_pDS->next();
      }

      m_pDS->close();
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "CPlexServerCacheDatabase::getCachedServers failed to read servers from database.");
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerCacheDatabase::cacheServers()
{
  PlexServerList servers = g_plexApplication.serverManager->GetAllServers();

  if (m_pDB.get() == NULL) return false;
  if (m_pDS.get() == NULL) return false;

  BeginTransaction();

  clearTables();

  BOOST_FOREACH(CPlexServerPtr server, servers)
  {
    if (!storeServer(server))
    {
      CLog::Log(LOGERROR, "CPlexServerCacheDatabase::cacheServers failed to store server, rolling back.");
      RollbackTransaction();
      return false;
    }
  }

  CommitTransaction();
  return true;
}
