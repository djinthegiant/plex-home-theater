#ifndef PLEXTIMELINEMANAGER_H
#define PLEXTIMELINEMANAGER_H

#include "FileItem.h"
#include "Utility/PlexTimer.h"
#include "threads/Event.h"
#include "threads/Timer.h"
#include "utils/UrlOptions.h"
#include "FileItem.h"
#include "Remote/PlexRemoteSubscriberManager.h"
#include "utils/XBMCTinyXML.h"
#include "PlexTimeline.h"

#include <map>
#include <queue>
#include <boost/shared_ptr.hpp>

typedef std::map<ePlexMediaType, CPlexTimelinePtr> PlexTimelineMap;

class CPlexTimelineManager
{
  public:
    CPlexTimelineManager();

    void ReportProgress(const CFileItemPtr &newItem, ePlexMediaState state, uint64_t currentPosition=0, bool force=false);
    CPlexTimelineCollectionPtr GetCurrentTimeLines();

    static uint64_t GetItemDuration(CFileItemPtr item);

    void SendTimelineToSubscriber(CPlexRemoteSubscriberPtr subscriber, const CPlexTimelineCollectionPtr &timelines);
    void SendTimelineToSubscribers(const CPlexTimelineCollectionPtr &timelines, bool delay = false);

    void SetTextFieldFocused(bool focused, const std::string &name="field", const std::string &contents=std::string(), bool isSecure=false);
    void RefreshSubscribers();

    void Stop();

    std::string GetCurrentFocusedTextField() const { return m_textFieldName; }
    bool IsTextFieldFocused() const { return m_textFieldFocused; }

    std::string TimerName() const { return "timelineManager"; }

    void SendCurrentTimelineToSubscriber(CPlexRemoteSubscriberPtr subscriber);
    bool GetTextFieldInfo(std::string& name, std::string& contents, bool& secure);

  private:
    void OnTimeout();
    void ReportProgress(const CPlexTimelinePtr &timeline, bool force);

    CCriticalSection m_timelineManagerLock;
    CPlexTimelineMap m_timelines;

    CPlexTimer m_subTimer;
    CPlexTimer m_serverTimer;

    bool m_stopped;
    bool m_textFieldFocused;
    std::string m_textFieldName;
    std::string m_textFieldContents;
    bool m_textFieldSecure;

    void NotifyPollers();
    CPlexTimelinePtr ResetTimeline(ePlexMediaType type, bool continuing = false);
};

typedef boost::shared_ptr<CPlexTimelineManager> CPlexTimelineManagerPtr;

#endif // PLEXTIMELINEMANAGER_H
