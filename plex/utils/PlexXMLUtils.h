#pragma once

#include <string>
#include "utils/XBMCTinyXML.h"
#include "video/VideoInfoTag.h"

class PlexXMLUtils
{
public:
  static bool GetString(const TiXmlElement* element, const char* tag, std::string& value);
  static bool GetString(const TiXmlElement* element, const char* tag, std::vector<std::string>& value);
  static bool GetFloat(const TiXmlElement* element, const char* tag, float& value);
  static bool GetInt(const TiXmlElement* element, const char* tag, int& value);
  static bool GetTimeStamp(const TiXmlElement* element, const char* tag, CDateTime& value);
  static bool GetDate(const TiXmlElement* element, const char* tag, CDateTime& value);
  static bool GetStringArray(const TiXmlElement* element, const char* tag, std::vector<std::string>& value);
  static bool GetActorInfoArray(const TiXmlElement* element, const char* tag, std::vector<SActorInfo>& value);
};
