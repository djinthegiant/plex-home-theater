#ifndef GUIDIALOGPLEXUSERSELECT_H
#define GUIDIALOGPLEXUSERSELECT_H

#include "dialogs/GUIDialogSelect.h"
#include "utils/Job.h"

class CGUIDialogPlexUserSelect : public CGUIDialogSelect// , public IJobCallback
{
public:
  CGUIDialogPlexUserSelect();
  bool OnMessage(CGUIMessage &message);
  void OnSelected();
  bool OnAction(const CAction &action);
  bool DidAuth() { return m_authed; }
  bool DidSwitchUser() { return m_userSwitched; }
  std::string getSelectedUser() { return m_selectedUser; }
  std::string getSelectedUserThumb() { return m_selectedUserThumb; }

  bool fetchUsers();
  bool m_authed;
  bool m_userSwitched;
  std::string m_selectedUser;
  std::string m_selectedUserThumb;
};

#endif // GUIDIALOGPLEXUSERSELECT_H
