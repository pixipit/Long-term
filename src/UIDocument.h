#pragma once

#include<WString.h>
#include <ArduinoJson.h>

#include <FS.h>
#include <SPIFFS.h>

#include "UIDocumentTypes.h"

class UIDocument
{
private:
  Element jsonToElement(const JsonObject& jObject);
  void printElement(Element& e);
  void alignChildren(Element& parent);
  int calculateGapXBetweenChildren(Element& parent);
  int calculateGapYBetweenChildren(Element& parent);
  Element toRealCoord(Element e, const Element& parentReal);
  void setProperties(const JsonObject& jObject, Element& e);
  const JsonArray* classes = nullptr;
public:
    Element root;
    UIDocument();
    ~UIDocument();
    UIDocument(const JsonObject& jObject, int width, int height, const JsonArray* classArray);
    UIDocument(String path, int screenWidth, int screenHeight);
    Element* find(const String& name);
    Element* find(const String& name, Element& parent);
    String GetFileAsString(String path);
};
