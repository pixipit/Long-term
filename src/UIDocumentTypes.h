#pragma once

#include <Arduino.h>

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
  bool border = true;
  ElementType type;
  String name;
  float posX = 0;
  float posY = 0;
  float sizeX = 1;
  float sizeY = 1;
  
  bool centered = true;
  String value;
  int textSize = 2;
  AlignType alignType = AlignType::LeftRight;
  int nOfChildren;
  Element* children;
};