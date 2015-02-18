#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 2
#define PIXEL_COUNT 40
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
void Blue(){for (int i = 0; i < strip.numPixels(); i++){strip.setPixelColor(i, strip.Color(  0,   0, 127));}strip.show();} //this should turn LEDs blue
void setup(){}
void loop(){Blue();}

