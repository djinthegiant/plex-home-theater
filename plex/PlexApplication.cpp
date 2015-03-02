
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "PlexApplication.h"
#include "utils/log.h"
#include "settings/Settings.h"

CPlexApplication::CPlexApplication(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

CPlexApplication::~CPlexApplication(void)
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}

void CPlexApplication::Initialize()
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);

  std::string uuid = CSettings::Get().GetString("system.uuid");
  if (uuid.empty())
  {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    CSettings::Get().SetString("system.uuid", boost::lexical_cast<std::string>(uuid));
  }
}

void CPlexApplication::Deinitialize()
{
  CLog::Log(LOGDEBUG, "%s called", __FUNCTION__);
}
