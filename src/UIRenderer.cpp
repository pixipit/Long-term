#include<UIRenderer.h>

UIRenderer::UIRenderer(GxEPD_Class* display): display(display){}

void UIRenderer::drawBoarder(const Element& e){
  display->drawRect((int)e.posX, (int)e.posY, (int)e.sizeX, (int)e.sizeY, GxEPD_BLACK);
}
void UIRenderer::drawText(const Element& eReal){
  display->setTextSize(eReal.textSize);
  display->setTextColor(GxEPD_BLACK);

  int16_t tbx, tby; uint16_t tbw, tbh;
  display->getTextBounds(eReal.value, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = eReal.posX + ((eReal.sizeX - tbw) / 2) - tbx;
  uint16_t y = eReal.posY + ((eReal.sizeY - tbh) / 2) - tby;
  display->setCursor(x, y);
  display->print(eReal.value);
}

void UIRenderer::drawElement(const Element& e){
  if(e.border){
    drawBoarder(e);
  }
  if(e.type == ElementType::Text){
    drawText(e);
  }
  for(int i = 0; i < e.nOfChildren; i++){
        drawElement(e.children[i]);
  }
}
void UIRenderer::render(const UIDocument* doc){
    drawElement(doc->root);
}
