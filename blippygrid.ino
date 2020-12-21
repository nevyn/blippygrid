#include <Adafruit_NeoPixel.h>
#include <AceButton.h>
using namespace ace_button;

int ledPin = 21;
int ledCount = 64;

typedef enum {
  CategRow12,
  CategRow34,
  CategRot56,
  CategRow78,
  CategFunc,
  CategSlot,

  CategCount
} ButtonCateg;

AceButton buttons[CategCount][16];
AceButton *buttonPtrs[CategCount][16] = {
  {&buttons[0][0], &buttons[0][1], &buttons[0][2], &buttons[0][3], &buttons[0][4], &buttons[0][5], &buttons[0][6], &buttons[0][7], &buttons[0][8], &buttons[0][9], &buttons[0][10], &buttons[0][11], &buttons[0][12], &buttons[0][13], &buttons[0][14], &buttons[0][15]},
  {&buttons[1][0], &buttons[1][1], &buttons[1][2], &buttons[1][3], &buttons[1][4], &buttons[1][5], &buttons[1][6], &buttons[1][7], &buttons[1][8], &buttons[1][9], &buttons[1][10], &buttons[1][11], &buttons[1][12], &buttons[1][13], &buttons[1][14], &buttons[1][15]},
  {&buttons[2][0], &buttons[2][1], &buttons[2][2], &buttons[2][3], &buttons[2][4], &buttons[2][5], &buttons[2][6], &buttons[2][7], &buttons[2][8], &buttons[2][9], &buttons[2][10], &buttons[2][11], &buttons[2][12], &buttons[2][13], &buttons[2][14], &buttons[2][15]},
  {&buttons[3][0], &buttons[3][1], &buttons[3][2], &buttons[3][3], &buttons[3][4], &buttons[3][5], &buttons[3][6], &buttons[3][7], &buttons[3][8], &buttons[3][9], &buttons[3][10], &buttons[3][11], &buttons[3][12], &buttons[3][13], &buttons[3][14], &buttons[3][15]},
  {&buttons[4][0], &buttons[4][1], &buttons[4][2], &buttons[4][3], &buttons[4][4], &buttons[4][5], &buttons[4][6], &buttons[4][7], &buttons[4][8], &buttons[4][9], &buttons[4][10], &buttons[4][11], &buttons[4][12], &buttons[4][13], &buttons[4][14], &buttons[4][15]},
  {&buttons[5][0], &buttons[5][1], &buttons[5][2], &buttons[5][3], &buttons[5][4], &buttons[5][5], &buttons[5][6], &buttons[5][7], &buttons[5][8], &buttons[5][9], &buttons[5][10], &buttons[5][11], &buttons[5][12], &buttons[5][13], &buttons[5][14], &buttons[5][15]},
};
uint16_t buttonLevels[CategCount][17] = {
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500},
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500},
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500},
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500},
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500},
  {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500}
};

LadderButtonConfig row12(A0, 17, buttonLevels[0], 16, buttonPtrs[0], LOW);
LadderButtonConfig row34(A1, 17, buttonLevels[1], 16, buttonPtrs[1], LOW);
LadderButtonConfig row56(A2, 17, buttonLevels[2], 16, buttonPtrs[2], LOW);
LadderButtonConfig row78(A3, 17, buttonLevels[3], 16, buttonPtrs[3], LOW);
LadderButtonConfig funcs(A8, 3, buttonLevels[4], 2, buttonPtrs[4], LOW);
LadderButtonConfig slots(A9, 5, buttonLevels[5], 4, buttonPtrs[5], LOW);
LadderButtonConfig *buttonConfigs[CategCount] = {&row12, &row34, &row56, &row78, &funcs, &slots};


Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

void setup() 
{
  Serial.begin(9600);           //Serial monitor used to determine limit values
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  for(int c = 0; c < CategCount; c++) {
    for(int b = 0; b < 16; b++) {
      buttons[c][b].init(buttonConfigs[c], b);
    }
  }
}

#ifdef CALIBRATE
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
void loop() {
  for(int c = 0; c < CategCount; c++) {
    buttonConfigs[c]->checkButtons();
  }
  strip.show();
}
#endif
