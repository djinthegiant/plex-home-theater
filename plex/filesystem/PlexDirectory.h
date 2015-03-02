#pragma once

#include "filesystem/IDirectory.h"
#include "filesystem/PlexFile.h"
#include "utils/XBMCTinyXML.h"

namespace XFILE
{
  class CPlexDirectory : public IDirectory
  {
  public:
    CPlexDirectory(void);
    virtual ~CPlexDirectory(void);
    virtual bool AllowAll() const { return true; }
    virtual bool GetDirectory(const CURL& url, CFileItemList &items);
    virtual void CancelDirectory();
    virtual bool Exists(const CURL& url);
  private:
    static bool ReadMediaContainer(const TiXmlElement* root, CFileItemList& items, const CURL& url);
    CPlexFile m_http;
  };
}
