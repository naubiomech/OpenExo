#include <Adafruit_INA260.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


int delayval = 100; // delay for half a second

Adafruit_INA260 ina260 = Adafruit_INA260();

void setup() {
  while (!Serial) { delay(10); }
  Serial.begin(115200);
  Serial.println("Adafruit INA260 tuning test");

  ina260.begin();

  // set the number of samples to average
  ina260.setAveragingCount(INA260_COUNT_16);
  // set the time over which to measure the current and bus voltage
  ina260.setVoltageConversionTime(INA260_TIME_140_us);
  ina260.setCurrentConversionTime(INA260_TIME_140_us);
}

void loop() {
  // measure and print current, voltage, and power to display on the serial plotter
  Serial.print(ina260.readCurrent());
  Serial.print(" ");
  Serial.print(ina260.readBusVoltage());
  Serial.print(" ");
  Serial.print(ina260.readPower());
  Serial.println();
 
  delay(125); // Delay for a period of time (in milliseconds).
}
