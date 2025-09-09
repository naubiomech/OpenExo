#include <Adafruit_NeoPixel.h>
#include <Adafruit_INA260.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 5
#define NUMPIXELS 8
int delayval = 100; // delay for half a second

// setup the neopixel strip with it's power running through the IN+ and IN- pins of the INA260
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_INA260 ina260 = Adafruit_INA260();


void setup() {
  while (!Serial) { delay(10); }
  Serial.begin(115200);
  Serial.println("Adafruit INA260 Test");

  pixels.begin();
  pixels.setBrightness(50);
  pixels.show(); // Initialize all pixels to 'off'

  ina260.begin();

  ina260.setAveragingCount(INA260_COUNT_4);
  ina260.setVoltageConversionTime(INA260_TIME_140_us);
  ina260.setCurrentConversionTime(INA260_TIME_140_us);
}

void loop() {
  // first the default continuous mode
  ina260.setMode(INA260_MODE_CONTINUOUS);
  rampPixelColors();
  delay(500);
  
  // second trigger a reading in triggered mode
  ina260.setMode(INA260_MODE_TRIGGERED);
  // the measurements in rampPixelColors will remain the same because we haven't triggered a new reading
  rampPixelColors();
  delay(500);
  
  // trigger a new reading, with the values staying the same again
  ina260.setMode(INA260_MODE_TRIGGERED);
  rampPixelColors();
  delay(500);

  // finally shutdown the INA260. The measurements will stay from the last triggered measurement.
  ina260.setMode(INA260_MODE_SHUTDOWN);
  rampPixelColors();

  delay(1000);

}

void rampPixelColors(void){
  pixels.setBrightness(25);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(255,0, 0)); // Red
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.print(ina260.readCurrent());
    Serial.print(" ");
    Serial.print(ina260.readBusVoltage());
    Serial.print(" ");
    Serial.print(ina260.readPower());
    Serial.println();
  }
  delay(delayval); // Delay for a period of time (in milliseconds).

  pixels.setBrightness(50);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(255,255, 0)); // Yellow (Red + Green)
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.print(ina260.readCurrent());
    Serial.print(" ");
    Serial.print(ina260.readBusVoltage());
    Serial.print(" ");
    Serial.print(ina260.readPower());
    Serial.println();
  }
  delay(delayval); // Delay for a period of time (in milliseconds).

  pixels.setBrightness(100);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(255,255, 255)); // White (Red + Green + Blue)
    pixels.show(); // This sends the updated pixel color to the hardware.
    Serial.print(ina260.readCurrent());
    Serial.print(" ");
    Serial.print(ina260.readBusVoltage());
    Serial.print(" ");
    Serial.print(ina260.readPower());
    Serial.println();
  }
  delay(delayval); // Delay for a period of time (in milliseconds).
}
