#pragma once

#include<UIDocument.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>  

#include "UIDocumentTypes.h"

class UIRenderer
{
private:
    GxEPD_Class* display = nullptr;
    /// @brief Draws element's border. Can be disabled by element atribute "border"="false"
    /// @param e 
    void drawBoarder(const Element& e);
    /// @brief Draws element's text. Text can be set by setting atribute "type":"text" and "value":"your text"
    /// @param eReal 
    void drawText(const Element& eReal);
    /// @brief Recursive function. Draws element then calls it self on element's children
    /// @param e 
    void drawElement(const Element& e);
public:
    UIRenderer(GxEPD_Class* display);
    /// @brief Renders UIDocument on display
    /// @param doc 
    void render(const UIDocument* doc);
};