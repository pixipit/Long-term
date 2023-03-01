#pragma once

#include<UIDocument.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>  

#include "UIDocumentTypes.h"

class UIRenderer
{
private:
    GxEPD_Class* display = nullptr;
    void drawBoarder(const Element& e);
    void drawText(const Element& eReal);
    void drawElement(const Element& e);
public:
    UIRenderer(GxEPD_Class* display);
    void render(const UIDocument* doc);
};