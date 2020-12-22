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
  
  AnimCount
} Animation;

// Global state
const char *funcName[] = {"None", "Save", "Load"};
const char *saveSlotName[] = {"pixels-00.bin", "pixels-01.bin", "pixels-02.bin", "pixels-03.bin"};
const char *animationName[] = {"animation-none.bin", "animation-save.bin", "animation-load.bin"};
const int saveSlotCount = 4;
Func currentFunc;

Pixel pixels[64];
Pixel animPixels[64];
int heldGridIndex = -1;

static const int ledPin = 21;
static const int ledCount = 64;

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


Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

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
  Serial.begin(9600);           //Serial monitor used to determine limit values
  SPIFFS.begin(true)
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

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
}

void loop()
{
  for(int c = 0; c < CategCount; c++)
  {
    buttonConfigs[c]->checkButtons();
  }

  if (heldGridIndex != -1)
  {
    int cHue = analogRead(A4);
    int cSaturation = analogRead(A5);
    int cValue = analogRead(A6);
    int cEffect = analogRead(A7);

    uint32_t color = Adafruit_NeoPixel::ColorHSV(cHue * 16, cSaturation/16, cValue/16);
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
    strip.setPixelColor(i, pixels[i].color);
  }
  strip.show();
}
#endif

void playAnimation(Animation anim)
{
  const char *filename = animationName[(int)anim];  
  File file = SPIFFS.open(filename, FILE_READ);
  file.read(animPixels, sizeof(Pixel)*ledCount);
  file.close();
  
  for(int i = 0; i < 100; i++) {
    float pFrac = i/100.0;
    float aFrac = 1.0 + pFrac;
    
    for(int i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i, pixels[i].color * pFrac + animPixels[i].color * aFrac);
    }
    
    strip.show();
    delay(10);
  }
}


void gridEvent(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  int indexOfGroup = button->getId();
  int indexWithinGroup = button->getPin()-1;
  int indexInGrid = indexOfGroup*16 + indexWithinGroup;

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

  const char *filename = saveSlotName[slot];  
  if(currentFunc == FuncSave) {
    File file = SPIFFS.open(filename, FILE_WRITE);
    file.write(pixels, sizeof(Pixel)*ledCount);
    file.close();
    playAnimation(AnimSave);
  } else if(currentFunc == FuncLoad) {
    File file = SPIFFS.open(filename, FILE_READ);
    file.read(pixels, sizeof(Pixel)*ledCount);
    file.close();
    playAnimation(AnimLoad);
  }
}
