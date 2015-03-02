
#include "PlexXMLUtils.h"

bool PlexXMLUtils::GetString(const TiXmlElement* element, const char* tag, std::string& value)
{
  const char *attribute = element->Attribute(tag);
  if (!attribute) return false;
  value = attribute;
  return true;
}

bool PlexXMLUtils::GetString(const TiXmlElement* element, const char* tag, std::vector<std::string>& value)
{
  const char *attribute = element->Attribute(tag);
  if (!attribute) return false;
  value.push_back(attribute);
  return true;
}

bool PlexXMLUtils::GetFloat(const TiXmlElement* element, const char* tag, float& value)
{
  const char *attribute = element->Attribute(tag);
  if (!attribute) return false;
  value = (float)atof(attribute);
  return true;
}

bool PlexXMLUtils::GetInt(const TiXmlElement* element, const char* tag, int& value)
{
  const char *attribute = element->Attribute(tag);
  if (!attribute) return false;
  value = (int)atoi(attribute);
  return true;
}

bool PlexXMLUtils::GetTimeStamp(const TiXmlElement* element, const char* tag, CDateTime& value)
{
  int i = 0;
  if (GetInt(element, tag, i) && i > 0)
  {
    value.SetFromUTCDateTime((time_t)i);
    return true;
  }
  return false;
}

bool PlexXMLUtils::GetDate(const TiXmlElement* element, const char* tag, CDateTime& value)
{
  std::string date;
  if (GetString(element, tag, date) && !date.empty())
  {
    value.SetFromDBDate(date);
    return true;
  }
  return false;
}

bool PlexXMLUtils::GetStringArray(const TiXmlElement* element, const char* tag, std::vector<std::string>& value)
{
  const TiXmlElement* node = element->FirstChildElement(tag);
  bool result = false;
  while (node)
  {
    const char *attribute = node->Attribute("tag");
    if (attribute)
    {
      value.push_back(attribute);
      result = true;
    }
    node = node->NextSiblingElement(tag);
  }
  return result;
}

bool PlexXMLUtils::GetActorInfoArray(const TiXmlElement* element, const char* tag, std::vector<SActorInfo>& value)
{
  const TiXmlElement* node = element->FirstChildElement(tag);
  bool result = false;
  while (node)
  {
    const char *attribute = node->Attribute("tag");
    if (attribute)
    {
      SActorInfo info;
      info.strName = attribute;
      GetString(node, "role", info.strRole);
      GetString(node, "thumb", info.thumb);
      value.push_back(info);
      result = true;
    }
    node = node->NextSiblingElement(tag);
  }
  return result;
}
