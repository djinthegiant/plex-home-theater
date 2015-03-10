#pragma once

#include "filesystem/CurlFile.h"
#include "URL.h"

#include <string>
#include <vector>

namespace XFILE
{
  class CPlexFile : public CCurlFile
  {
  public:

    CPlexFile(void);
    virtual ~CPlexFile() {};

    /* overloaded from CCurlFile */
    virtual bool Open(const CURL& url);
    virtual bool Exists(const CURL& url);
    virtual int Stat(const CURL& url, struct __stat64* buffer);
    virtual int IoControl(EIoControl request, void* param);

    bool Get(const CURL &url, std::string& data);
    bool Put(const CURL &url, std::string& data);
    bool Post(const CURL &url, const std::string& postData, std::string& data);
    bool Delete(const CURL &url, std::string& data);

    static std::vector<std::pair<std::string, std::string> > GetHeaderList();
    static bool BuildHTTPURL(CURL& url);

    /* Returns false if the server is missing or
     * there is something else wrong */
    static bool CanBeTranslated(const CURL &url);
    static std::string GetMimeType(const CURL &url);
    bool IsTokenInvalid() const
    {
      return m_tokenInvalid;
    }

  protected:
    bool m_tokenInvalid;
    virtual bool Service(const std::string &strURL, std::string &strHTML);
  };
}
