#pragma once

#include "filesystem/CurlFile.h"
#include "URL.h"

namespace XFILE
{
  class CPlexFile : public CCurlFile
  {
  public:
    CPlexFile(void);
    virtual ~CPlexFile(void);
    virtual bool Open(const CURL& url);
    virtual bool Exists(const CURL& url);
    virtual int Stat(const CURL& url, struct __stat64* buffer);

    bool Get(const CURL &url, std::string& data);
    bool Post(const CURL &url, const std::string& postData, std::string& data);
    bool Delete(const CURL &url, std::string& data);

    static bool BuildHTTPURL(CURL& url);
  private:
    static bool IsLocalArt(const CURL &url);
  };
}
