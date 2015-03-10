//
//  GUIDialogPlexPluginSettings.h
//  Plex
//
//  Created by Jamie Kirkpatrick on 03/02/2011.
//  Copyright 2011 Kirk Consulting Limited. All rights reserved.
//

#pragma once

#include "utils/XBMCTinyXML.h"
#include "URL.h"
#include "dialogs/GUIDialogBoxBase.h"
#include "guilib/GUIWindowManager.h"

class CPlexPluginSettings
{
public:
  void Set(const std::string& key, const std::string& value);
  std::string Get(const std::string& key);
  
  bool Load(TiXmlElement* root);
  bool Save(const std::string& strPath);
  
  TiXmlElement* GetPluginRoot();
  
private: 
  TiXmlDocument    m_userXmlDoc;
  TiXmlDocument    m_pluginXmlDoc;
};

class CGUIDialogPlexPluginSettings : public CGUIDialogBoxBase
{
public:
  CGUIDialogPlexPluginSettings();
  virtual bool OnMessage(CGUIMessage& message);
  static void ShowAndGetInput(const std::string& path, const std::string& compositeXml);
  int GetDefaultLabelID(int controlId) const;
  
private:
  void CreateControls();
  void FreeControls();
  void EnableControls();
  void SetDefaults();
  bool GetCondition(const std::string &condition, const int controlId);

  bool SaveSettings(void);
  bool ShowVirtualKeyboard(int iControl);
  bool TranslateSingleString(const std::string &strCondition, std::vector<std::string> &enableVec);
  CPlexPluginSettings* m_settings;
  std::string m_strHeading;
  bool m_okSelected; 
};
