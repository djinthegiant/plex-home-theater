#pragma once

#include "utils/GlobalsHandling.h"

class CPlexApplication
{
public:
  CPlexApplication(void);
  virtual ~CPlexApplication(void);
  void Initialize();
  void Deinitialize();
};

XBMC_GLOBAL_REF(CPlexApplication, g_plexApplication);
#define g_plexApplication XBMC_GLOBAL_USE(CPlexApplication)
