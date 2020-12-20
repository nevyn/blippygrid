#include <Adafruit_NeoPixel.h>

int buttonPin = A1;
int ledPin = 6;
int ledCount = 8;
int boundaries[] = {
  80,
  115,
  145,
  200,
  280,
  400,
  600,
  1200
};

Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

void setup() 
{
  Serial.begin(9600);           //Serial monitor used to determine limit values
  //strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  //strip.show();            // Turn OFF all pixels ASAP
  //strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {
  delay(100);
  int pinValue1 = analogRead(A0);
  int pinValue2 = analogRead(A1);
  int pinValue3 = analogRead(A2);
  int pinValue4 = analogRead(A3);
  Serial.print(pinValue1);
  Serial.print(" ");
  Serial.print(pinValue2);
  Serial.print(" ");
  Serial.print(pinValue3);
  Serial.print(" ");
  Serial.println(pinValue4);
return;
  int litIndex = -1;
  for(int i = 0; i < ledCount; i++)
  {
    if(pinValue1 < boundaries[i]) {
      litIndex = i;
      break;
    }
  }

  strip.clear();
  if(pinValue1 > 60) {
    strip.setPixelColor(7 - litIndex, strip.Color(255,255,255));
  }
  strip.show();
  
  delay(100);                         //Delay for stability
}
