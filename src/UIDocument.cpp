#include<UIDocument.h>

UIDocument::UIDocument(/* args */){}
/// @brief Creates UIDocument based on JsonObject. Unknow keys are skipped
    /// @param jObject 
    /// @param width 
    /// @param height 
    /// @param classArray 
UIDocument::UIDocument(const JsonObject& jObject, int screenWidth, int screenHeight, const JsonArray* classArray = nullptr): classes(classArray){
  Serial.println("converting json to UIDoc");
  root = jsonToElement(jObject);
  Serial.println("json converted");
  Element parent;
  parent.posX = 0;
  parent.posY = 2;

  parent.sizeX = screenHeight;
  parent.sizeY = screenWidth;
  
  
  root = toRealCoord(root, parent);

  alignChildren(root);
  Serial.println("converted to UIdoc");
}
/// @brief Loads file and creates UIDocument based on it
/// @param path 
/// @param screenWidth 
/// @param screenHeight 
UIDocument::UIDocument(String path, int screenWidth, int screenHeight){
  String jsonString = getFileAsString(path);
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonObject jObject = doc["visualElement"].as<JsonObject>();

  jsonString = getFileAsString("/styleClasses.json");
  DynamicJsonDocument classes(1024);
  error = deserializeJson(classes, jsonString);
  if (error) {
    Serial.print(F("(styleClass.json) deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonArray classArray = classes["classes"].as<JsonArray>();

  *this =  UIDocument(jObject, screenWidth, screenHeight, &classArray);
}

UIDocument::~UIDocument(){
  //for(int i = 0; i < root.nOfChildren; i++){
  //  delete &(root.children[i]);
  //}
}
/// @brief finds element by name
/// @param name 
/// @param parent parent to search in for
/// @return pointer to element. If the element is not found return null pointer.
Element* UIDocument::find(const String& name, Element& parent){
  if(parent.name == name){
    return &parent;
  }
  for(int i = 0; i < parent.nOfChildren; i++){
    Element* e = find(name, parent.children[i]);
    if(e != nullptr){
      return e;
    }
  }
  return nullptr;
}
/// @brief finds element by name
/// @param name
/// @return pointer to element. If the element is not found return null pointer.
Element* UIDocument::find(const String& name){
  return find(name, root);
}


Element UIDocument::toRealCoord(Element e, const Element& parentReal){
    //converts size to real size
    e.sizeX = e.sizeX * parentReal.sizeX;
    e.sizeY = e.sizeY * parentReal.sizeY;
    if(e.centered){
        //parent size/2 - e size/2
        e.posX = parentReal.posX +  parentReal.sizeX/2 - e.sizeX/2;
        e.posY = parentReal.posY +  parentReal.sizeY/2 - e.sizeY/2;
    }else{
        e.posX = parentReal.posX + e.posX * parentReal.sizeX;
        e.posY = parentReal.posY + e.posY * parentReal.sizeY;
    }

  return e;
}

void UIDocument::printElement(Element& e){
    Serial.println(e.name);
    Serial.println(e.nOfChildren);
    Serial.println(String(e.posX) + ", " + String(e.posY));
    Serial.println(String(e.sizeX) + ", " + String(e.sizeY));
}

void UIDocument::setProperties(const JsonObject& jObject, Element& e){
  for (JsonPair kv : jObject) {
    if(String(kv.key().c_str()) == String("pos")){
      JsonArray array = kv.value().as<JsonArray>();
      e.posX = array[0].as<float>();
      e.posY = array[1].as<float>();
    }

     if(String(kv.key().c_str()) == String("size")){
      JsonArray array = kv.value().as<JsonArray>();
      e.sizeX = array[0].as<float>();
      e.sizeY = array[1].as<float>();
    }
    if(String(kv.key().c_str()) == String("type")){
      if(kv.value().as<String>() == String("text")){
        e.type = ElementType::Text;
      }
    }
    if(String(kv.key().c_str()) == String("value")){
      e.value = kv.value().as<String>();
    }
    if(String(kv.key().c_str()) == String("name")){
      e.name = kv.value().as<String>();
    }
    if(String(kv.key().c_str()) == String("border")){
      e.border = (kv.value().as<String>() == "true");
    }
     if(String(kv.key().c_str()) == String("textSize")){
      e.textSize = kv.value().as<int>();
    }
    if(String(kv.key().c_str()) == String("alignType")){
      String alignType = kv.value().as<String>();
      if(alignType == "TopDown"){
        e.alignType = AlignType::TopDown;
      }
      if(alignType == "DownTop"){
        e.alignType = AlignType::DownTop;
      }
      if(alignType == "LeftRight"){
        e.alignType = AlignType::LeftRight;
      }
      if(alignType == "RightLeft"){
        e.alignType = AlignType::RightLeft;
      }
    }
    if(String(kv.key().c_str()) == String("class") && classes!=nullptr){
      String className = kv.value().as<String>();
      for(JsonVariant v : *classes) {
        if(!v.containsKey("name")){
          continue;
        }
        if(v["name"].as<String>() == className){
          JsonObject data = v["data"].as<JsonObject>();
          setProperties(data, e);
          break;
        }
      }
    }
  }
}

Element UIDocument::jsonToElement(const JsonObject& jObject){
  Element e;
  setProperties(jObject, e);

  if(!jObject.containsKey("children")){
    e.nOfChildren = 0;
    return e;
  }

  JsonArray array = jObject["children"].as<JsonArray>();
  e.nOfChildren = array.size();
  e.children = new Element[e.nOfChildren];
  int i = 0;
  for(JsonVariant v : array) {
    JsonObject child = v.as<JsonObject>();
    e.children[i] = jsonToElement(child);
    //printElement(e.children[i]);
    i++;
  }
  return e;
}

int UIDocument::calculateGapXBetweenChildren(Element& parent){
    int size = 0;
    for(int i = 0; i < parent.nOfChildren; i++){
       size += (int)(toRealCoord(parent.children[i], parent).sizeX);
    }
    if(size >= parent.sizeX){
        Serial.println("Error: Element (name:" + parent.name + " ) has to much elements to fit (scale X)");
        return 0;
    }
    size = parent.sizeX - size;
    return size/(parent.nOfChildren + 1);
}
int UIDocument::calculateGapYBetweenChildren(Element& parent){
    int size = 0;
    for(int i = 0; i < parent.nOfChildren; i++){
       size += toRealCoord(parent.children[i], parent).sizeY;
    }
    if(size >= parent.sizeY){
        Serial.println("Error: To much element to fit");
        return 0;
    }
    size = parent.sizeY - size;
    return size/(parent.nOfChildren + 1);
}

void UIDocument::alignChildren(Element& parent){
    if(parent.nOfChildren == 0){
        return;
    }
    if(parent.nOfChildren == 1){
        parent.children[0] =  toRealCoord(parent.children[0], parent);
        alignChildren(parent.children[0]);
        return;
    }

    if(parent.alignType == AlignType::LeftRight){
        int padding = calculateGapXBetweenChildren(parent);
        int offset = 0;
        for(int i = 0; i < parent.nOfChildren; i++){
            offset += padding;
            parent.children[i] =  toRealCoord(parent.children[i], parent);
            parent.children[i].posX = parent.posX + offset;
            offset += parent.children[i].sizeX;
        }
    }
    if(parent.alignType == AlignType::TopDown){
        int padding = calculateGapYBetweenChildren(parent);
        
        int offset = 0;
        for(int i = 0; i < parent.nOfChildren; i++){
            offset += padding;
            parent.children[i] =  toRealCoord(parent.children[i], parent);
            parent.children[i].posY = parent.posY + offset;
            offset += parent.children[i].sizeY;
        }
    }

    for(int i = 0; i < parent.nOfChildren; i++){
        alignChildren(parent.children[i]);
    }
}

String UIDocument::getFileAsString(String path){
  if(!SPIFFS.exists(path)){
    Serial.println("file not found! path:" + path);
  }
  
  File f = SPIFFS.open(path, "r");

  if (!f) {
    Serial.println("file open failed");
  }
  String jsonMenu = f.readString();
  f.close();
  return jsonMenu;
}
