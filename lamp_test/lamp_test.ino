/*
 * Turns on all 60 pixels for alignment purposes
 */
#include <Adafruit_NeoPixel.h>
#define Count 60
#define Pin 11
Adafruit_NeoPixel strip = Adafruit_NeoPixel(Count,Pin,NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, 3,3,3);
  }
  strip.show();
}

void loop() {
}

