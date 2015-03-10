#ifndef PLEXREMOTEPLAYHANDLER_H
#define PLEXREMOTEPLAYHANDLER_H

#include "PlexHTTPRemoteHandler.h"

class CPlexRemotePlayHandler : public IPlexRemoteHandler
{
  friend class PlexRemotePlayHandlerTests;
public:
  CPlexRemoteResponse handle(const std::string& url, const ArgMap& arguments);
  bool getKeyAndContainerUrl(const ArgMap &arguments, std::string& key, std::string& containerKey);
  virtual bool getContainer(const CURL& dirURL, CFileItemList& list);
  CFileItemPtr getItemFromContainer(const std::string &key, const CFileItemList& list, int& idx);
  int64_t getStartPosition(const ArgMap &arguments);
  CPlexRemoteResponse playPlayQueue(const CPlexServerPtr &server, const std::string &playQueueUrl,
                                    const ArgMap& arguments);
};

#endif // PLEXREMOTEPLAYHANDLER_H
