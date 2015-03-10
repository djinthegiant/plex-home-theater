//
//  PlexDirectory.h
//  Plex
//
//  Created by Tobias Hieta <tobias@plexapp.com> on 2013-04-05.
//  Copyright 2013 Plex Inc. All rights reserved.
//

#ifndef PLEXDIRECTORY_H
#define PLEXDIRECTORY_H

#include "filesystem/IDirectory.h"
#include "URL.h"
#include "XMLChoice.h"
#include "FileItem.h"

#include "PlexAttributeParser.h"

#include <map>
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>

#include "PlexTypes.h"
#include "utils/JobManager.h"

#include "utils/log.h"

#include "FileSystem/PlexFile.h"
#include "FileSystem/PlexDirectoryCache.h"

namespace XFILE
{
  class CPlexDirectory : public IDirectory
  {
  public:
    CPlexDirectory()
      : m_verb("GET")
      , m_xmlData(new char[1024])
      , m_cacheStrategy(CPlexDirectoryCache::CACHE_STRATEGY_ITEM_COUNT)
      , m_showErrors(false)
    {
    }

    // make it easy to override network access in tests.
    virtual bool GetXMLData(std::string& data);
    virtual bool GetDirectory(const CURL& url, CFileItemList& items);

    /* plexserver://shared */
    bool GetSharedServerDirectory(CFileItemList& items);

    /* plexserver://channels */
    bool GetChannelDirectory(CFileItemList& items);

    /* plexserver://channeldirectory */
    bool GetOnlineChannelDirectory(CFileItemList& items);

    /* plexserver://playqueue */
    bool GetPlayQueueDirectory(ePlexMediaType type, CFileItemList& items, bool unplayed = false);
    
    /* plexserver://playlists */
    bool GetPlaylistsDirectory(CFileItemList& items, std::string options);

    bool GetDirectory(const std::string& strPath, CFileItemList& items)
    {
      return GetDirectory(CURL(strPath), items);
    }

    virtual void CancelDirectory();

    static EPlexDirectoryType GetDirectoryType(const std::string& typeStr);
    static std::string GetDirectoryTypeString(EPlexDirectoryType typeNr);

    static std::string GetContentFromType(EPlexDirectoryType typeNr);

    void SetHTTPVerb(const std::string& verb)
    {
      m_verb = verb;
    }
    
    inline std::string getHTTPVerb() { return m_verb; };

    /* Legacy functions we need to revisit */
    void SetBody(const std::string& body)
    {
      m_body = body;
    }

    static CFileItemListPtr GetFilterList()
    {
      return CFileItemListPtr();
    }

    std::string GetData() const
    {
      return m_data;
    }

    static void CopyAttributes(XML_ELEMENT* element, CFileItem* fileItem, const CURL& url);
    static CFileItemPtr NewPlexElement(XML_ELEMENT* element, const CFileItem& parentItem,
                                       const CURL& url = CURL());

    static bool IsFolder(const CFileItemPtr& item, XML_ELEMENT* element);

    long GetHTTPResponseCode() const
    {
      return m_file.GetLastHTTPResponseCode();
    }

    bool IsTokenInvalid() const
    {
      return m_file.IsTokenInvalid();
    }

    virtual DIR_CACHE_TYPE GetCacheType(const CURL& url) const;

    virtual bool IsAllowed(const CURL& url) const
    {
      return true;
    }

    virtual bool AllowAll() const { return true; }

    static bool CachePath(const std::string& path);

    inline void SetCacheStrategy(CPlexDirectoryCache::CacheStrategies Strategy) { m_cacheStrategy = Strategy; }

    bool ReadMediaContainer(XML_ELEMENT* root, CFileItemList& mediaContainer);
    void ReadChildren(XML_ELEMENT* element, CFileItemList& container);

    inline void SetShowErrors(bool showErrors)  { m_showErrors = showErrors; }
    inline bool ShouldShowErrors()  { return m_showErrors; }

  private:
    std::string m_body;
    std::string m_data;
    boost::scoped_array<char> m_xmlData;
    CURL m_url;
    CPlexDirectoryCache::CacheStrategies m_cacheStrategy;

    CPlexFile m_file;

    std::string m_verb;
    bool m_showErrors;
  };
}

#endif // PLEXDIRECTORY_H
