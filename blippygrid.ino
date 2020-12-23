#include <Adafruit_NeoPixel.h>
#include <AceButton.h>
#include <SPIFFS.h>
using namespace ace_button;

//#define CALIBRATE

// Global types
typedef enum {
  CategRow12,
  CategRow34,
  CategRot56,
  CategRow78,
  CategFunc,
  CategSlot,

  CategCount
} ButtonCateg;

typedef enum {
  FuncNone,
  FuncSave,
  FuncLoad
} Func;

typedef struct {
  uint32_t color;
  uint16_t effect;
  uint32_t reserved;  
} Pixel;

typedef enum {
  AnimNone,
  AnimSave,
  AnimLoad,
  AnimBoot,
  
  AnimCount
} Animation;

// Global state
const char *funcName[] = {"None", "Save", "Load"};
const char *saveSlotName[] = {"/pixels-00.bin", "/pixels-01.bin", "/pixels-02.bin", "/pixels-03.bin"};
const char *animationName[] = {"/animation-none.bin", "/animation-save.bin", "/animation-load.bin", "/animation-boot.bin"};
const int saveSlotCount = 4;
Func currentFunc;

Pixel pixels[64];
Pixel animPixels[64];
int heldGridIndex = -1;

static const int ledPin = 21;
static const int ledCount = 64;
static const int pixelsByteSize = sizeof(Pixel)*ledCount;

AceButton buttons[CategCount][16];
AceButton *buttonPtrs[CategCount][16] = {
  {&buttons[0][0], &buttons[0][1], &buttons[0][2], &buttons[0][3], &buttons[0][4], &buttons[0][5], &buttons[0][6], &buttons[0][7], &buttons[0][8], &buttons[0][9], &buttons[0][10], &buttons[0][11], &buttons[0][12], &buttons[0][13], &buttons[0][14], &buttons[0][15]},
  {&buttons[1][0], &buttons[1][1], &buttons[1][2], &buttons[1][3], &buttons[1][4], &buttons[1][5], &buttons[1][6], &buttons[1][7], &buttons[1][8], &buttons[1][9], &buttons[1][10], &buttons[1][11], &buttons[1][12], &buttons[1][13], &buttons[1][14], &buttons[1][15]},
  {&buttons[2][0], &buttons[2][1], &buttons[2][2], &buttons[2][3], &buttons[2][4], &buttons[2][5], &buttons[2][6], &buttons[2][7], &buttons[2][8], &buttons[2][9], &buttons[2][10], &buttons[2][11], &buttons[2][12], &buttons[2][13], &buttons[2][14], &buttons[2][15]},
  {&buttons[3][0], &buttons[3][1], &buttons[3][2], &buttons[3][3], &buttons[3][4], &buttons[3][5], &buttons[3][6], &buttons[3][7], &buttons[3][8], &buttons[3][9], &buttons[3][10], &buttons[3][11], &buttons[3][12], &buttons[3][13], &buttons[3][14], &buttons[3][15]},
  {&buttons[4][0], &buttons[4][1], &buttons[4][2], &buttons[4][3], &buttons[4][4], &buttons[4][5], &buttons[4][6], &buttons[4][7], &buttons[4][8], &buttons[4][9], &buttons[4][10], &buttons[4][11], &buttons[4][12], &buttons[4][13], &buttons[4][14], &buttons[4][15]},
  {&buttons[5][0], &buttons[5][1], &buttons[5][2], &buttons[5][3], &buttons[5][4], &buttons[5][5], &buttons[5][6], &buttons[5][7], &buttons[5][8], &buttons[5][9], &buttons[5][10], &buttons[5][11], &buttons[5][12], &buttons[5][13], &buttons[5][14], &buttons[5][15]},
};
uint16_t buttonLevels[CategCount][18] = {
  {0, 144, 185, 322, 505, 692, 725, 756, 800, 1040, 1420, 2047, 2260, 3155, 3440, 3850, 4095, 4100},
  {0, 370, 540, 712, 920, 1078, 1253, 1480, 1780, 1981, 2226, 2527, 2783, 3119, 3605, 3987, 4095, 4100},
  {0, 370, 540, 712, 920, 1078, 1253, 1480, 1780, 1981, 2226, 2527, 2783, 3119, 3605, 3987, 4095, 4100},
  {0, 370, 540, 712, 920, 1078, 1253, 1480, 1780, 1981, 2226, 2527, 2783, 3119, 3605, 3987, 4095, 4100},
  {0, 1180, 4095, 4100},
  {0, 1350, 2350, 3450, 4095, 4100}
};

LadderButtonConfig row12(A0, 18, buttonLevels[0], 16, buttonPtrs[0], LOW);
LadderButtonConfig row34(A1, 18, buttonLevels[1], 16, buttonPtrs[1], LOW);
LadderButtonConfig row56(A2, 18, buttonLevels[2], 16, buttonPtrs[2], LOW);
LadderButtonConfig row78(A3, 18, buttonLevels[3], 16, buttonPtrs[3], LOW);
LadderButtonConfig funcs(A8, 4, buttonLevels[4], 2, buttonPtrs[4], LOW);
LadderButtonConfig slots(A9, 6, buttonLevels[5], 4, buttonPtrs[5], LOW);
LadderButtonConfig *buttonConfigs[CategCount] = {&row12, &row34, &row56, &row78, &funcs, &slots};


Adafruit_NeoPixel strip(ledCount + 1, ledPin, NEO_GRB + NEO_KHZ800);

void gridEvent(AceButton* button, uint8_t eventType, uint8_t buttonState);
void funcEvent(AceButton* button, uint8_t eventType, uint8_t buttonState);
void slotEvent(AceButton* button, uint8_t eventType, uint8_t buttonState);

#ifdef CALIBRATE

void setup() 
{
  Serial.begin(9600)
}

void loop() {
  delay(100);
  int buttonValue1 = analogRead(A0);
  int buttonValue2 = analogRead(A1);
  int buttonValue3 = analogRead(A2);
  int buttonValue4 = analogRead(A3);
  
  int potValue1 = analogRead(A4);
  int potValue2 = analogRead(A5);
  int potValue3 = analogRead(A6);
  int potValue4 = analogRead(A7);

  int functionButtonValue = analogRead(A8);
  int saveSlotButtonValue = analogRead(A9);
  
  Serial.print(buttonValue1);
  Serial.print(" ");
  Serial.print(buttonValue2);
  Serial.print(" ");
  Serial.print(buttonValue3);
  Serial.print(" ");
  Serial.print(buttonValue4);
  Serial.print(" ");
  Serial.print(potValue1);
  Serial.print(" ");
  Serial.print(potValue2);
  Serial.print(" ");
  Serial.print(potValue3);
  Serial.print(" ");
  Serial.print(potValue4);
  Serial.print(" ");
  Serial.print(functionButtonValue);
  Serial.print(" ");
  Serial.println(saveSlotButtonValue);
}

#else

void setup() 
{
  Serial.begin(9600); // for debugging state
  Serial.println("Hello! This is blippygrid by hello@nevyn.dev");
  
  bool fsOk = SPIFFS.begin(true);
  Serial.print("Filesystem status ");
  Serial.println(fsOk);
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set max brightness (255=max)

  for(int c = 0; c < CategCount; c++) {
    for(int b = 0; b < 16; b++) {
      buttons[c][b].init(buttonConfigs[c], b+1, LOW, c);
    }
  }
  buttonConfigs[0]->setEventHandler(gridEvent);
  buttonConfigs[1]->setEventHandler(gridEvent);
  buttonConfigs[2]->setEventHandler(gridEvent);
  buttonConfigs[3]->setEventHandler(gridEvent);
  buttonConfigs[4]->setEventHandler(funcEvent);
  buttonConfigs[5]->setEventHandler(slotEvent);
  playAnimation(AnimBoot);
}

uint32_t applyEffect(float t, uint16_t effect, uint32_t c);

float t = 0.0;

void loop()
{
  for(int c = 0; c < CategCount; c++)
  {
    buttonConfigs[c]->checkButtons();
  }

  int cHue = analogRead(A5);
  int cSaturation = analogRead(A4);
  int cValue = analogRead(A6);
  int cEffect = analogRead(A7);
  uint32_t color = Adafruit_NeoPixel::ColorHSV(cHue * 16, cSaturation/16, cValue/16);

  if (heldGridIndex != -1)
  {
    pixels[heldGridIndex].color = color;
    pixels[heldGridIndex].effect = cEffect;
    
    Serial.print(heldGridIndex);
    Serial.print(" gets color ");
    Serial.print(color, HEX);
    Serial.print(" effect ");
    Serial.println(cEffect);
  }
  
  for(int i = 0; i < ledCount; i++)
  {
    uint32_t effectiveColor = applyEffect(t, pixels[i].effect, pixels[i].color);
    strip.setPixelColor(i, effectiveColor);
  }
  strip.setPixelColor(64, applyEffect(t, cEffect, color));
  strip.show();
  t += 0.005;
}
#endif

uint32_t interpolateColors(uint32_t colorA, float aFrac, uint32_t colorB)
{
  float bFrac = 1.0 - aFrac;
  int rA = colorA >> 16 & 0xff;
  int rB = colorB >> 16 & 0xff;
  int gA = colorA >> 8 & 0xff;
  int gB = colorB >> 8 & 0xff;
  int bA = colorA & 0xff;
  int bB = colorB & 0xff;

  int r = rA * aFrac + rB * bFrac;
  int g = gA * aFrac + gB * bFrac;
  int b = bA * aFrac + bB * bFrac;

  uint32_t ret = ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
  return ret;
}

uint32_t fade(uint32_t colorA, float frac)
{
  int rA = colorA >> 16 & 0xff;
  int gA = colorA >> 8 & 0xff;
  int bA = colorA & 0xff;

  int r = rA * frac;
  int g = gA * frac;
  int b = bA * frac;
  uint32_t ret = ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
  return ret;
}

void playAnimation(Animation anim)
{
  const char *filename = animationName[(int)anim];  
  File file = SPIFFS.open(filename, FILE_READ);
  file.read((uint8_t*)animPixels, pixelsByteSize);
  file.close();

  for(int i = 0; i < ledCount; i++)
  {
    strip.setPixelColor(i, animPixels[i].color);
  }
  strip.show();
  delay(500);
  
  for(int i = 0; i < 100; i++) {
    float pFrac = i/100.0;
    float aFrac = 1.0 + pFrac;
    
    for(int i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i, interpolateColors(pixels[i].color, pFrac, animPixels[i].color));
    }
    
    strip.show();
    delay(10);
  }
}

uint32_t applyEffect(float t, uint16_t effect, uint32_t c)
{
  if(effect == 0)
  {
    return c;
  }

  float feffect4 = (float)effect / 1024.0;
  float feffect = fmod(feffect4, 1.0);
  if (effect < 1024) {
    return fade(c, sin(t + feffect*4)/2.0 + 0.5);
  } else if(effect < 2048) {
    return fade(c, sin(t * (feffect*4+1))/2.0 + 0.5);
  } else if(effect < 3096) {
    return fmod(t/4.0, 1.0) < feffect ? c : 0;
  } else {
    return fmod(t/4.0, 1.0) > feffect ? c : 0;
  }
}



///// -----


String slotIndexToFilename(int slotIndex)
{
  String filename = "/pixels-";
  filename += slotIndex;
  filename += ".bin";

  if(slotIndex > 63) {
    filename = animationName[slotIndex-63];
  }
  return filename;
}

void save(int slotIndex)
{
  String filename = slotIndexToFilename(slotIndex);  
  File file = SPIFFS.open(filename, FILE_WRITE);
  int written = file.write((uint8_t*)pixels, pixelsByteSize);
  file.close();
  
  Serial.print("Wrote ");
  Serial.print(written);
  Serial.print(" of ");
  Serial.print(pixelsByteSize);
  Serial.print(" bytes to ");
  Serial.println(filename);
  
  playAnimation(AnimSave);
}

void load(int slotIndex)
{
  String filename = slotIndexToFilename(slotIndex);
  File file = SPIFFS.open(filename, FILE_READ);
  int readed = file.read((uint8_t*)pixels, pixelsByteSize);
  if(readed == 0) {
    memset((uint8_t*)pixels, 0, pixelsByteSize);
  }
  file.close();

  Serial.print("Read ");
  Serial.print(readed);
  Serial.print(" of ");
  Serial.print(pixelsByteSize);
  Serial.print(" bytes from ");
  Serial.println(filename);
  
  playAnimation(AnimLoad);
}

///// -----


void gridEvent(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  int indexOfGroup = button->getId();
  int indexWithinGroup = button->getPin()-1;
  int indexInGrid = indexOfGroup*16 + indexWithinGroup;

  if(currentFunc != FuncNone && buttonState == 1) {
    int slot = 4 + indexInGrid;
    
    Serial.print(funcName[currentFunc]);
    Serial.print(" to slot ");
    Serial.println(slot+1);
  
    if(currentFunc == FuncSave) {
      save(slot);
    } else if(currentFunc == FuncLoad) {
      load(slot);
    }
    return;
  }

  if(buttonState == 1)
  {
    heldGridIndex = indexInGrid;
  } else {
    heldGridIndex = -1;
  }
}

void funcEvent(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  if(buttonState == 0)
  {
    currentFunc = FuncNone;
    return;
  } else {
    currentFunc = (Func)button->getPin();
  }
}


void slotEvent(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  if(currentFunc == FuncNone || buttonState == 0) return;
  int slot = button->getPin()-1;
  
  Serial.print(funcName[currentFunc]);
  Serial.print(" to slot ");
  Serial.println(slot+1);

  if(currentFunc == FuncSave) {
    save(slot);
  } else if(currentFunc == FuncLoad) {
    load(slot);
  }
}
