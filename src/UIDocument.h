#pragma once

#include<WString.h>
#include <ArduinoJson.h>


enum ElementType
{
  VisualElement,
  Text
};

enum AlignType
{
    LeftRight,
    RightLeft,
    TopDown,
    DownTop
};

class Element
{
public:
  bool boarder = true;
  ElementType type;
  String name;
  float posX = 0;
  float posY = 0;
  float sizeX = 1;
  float sizeY = 1;
  
  bool centered = true;
  String value;
  AlignType alignType = AlignType::LeftRight;
  int nOfChildren;
  Element* children;
};

class UIDocument
{
private:
  Element jsonToElement(const JsonObject& jObject);
  void printElement(Element& e);
  void displayElement(JsonObject jObject, int width, int height);
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
    Element* find(const String& name);
    Element* find(const String& name, Element& parent);
};
