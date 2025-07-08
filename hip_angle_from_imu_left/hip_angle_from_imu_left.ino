// include library for the IMU inside the nano 33 sense rev2
#include <Arduino_BMI270_BMM150.h>
// include Arduino library for I2C
#include <Wire.h>
// define peripheral I2C Address
#define PERIPHERAL_ADDR 8
// define peripheral answer size
#define ANSWERSIZE 5
// include kalman filter library
#include <Kalman.h>

// define variables
Kalman kalman;
float prevtime = 0;
float t = 10;
// immediately below, a union is used which allows the same value to be stored and then accessed as different
// data types. the name of the union is "data." accessing data.x returns the stored value as a float.
// accessing data.myData returns the value as a byte. This is necessary for i2c communication, which
// only accepts byte type data (no floats or ints).
union {
  float estim_angle;
  byte angle_bytes[4];
}data;  // the name used to access the data inside the union

void setup() {
  Wire.begin(PERIPHERAL_ADDR);     // initialize i2c communication as a peripheral
  Wire.onRequest(sendEvent);      // upon request of the main i2c device, initiate the sendEvent function
  //Serial.begin(9600);
  IMU.begin();
}

void loop() {
  float x, y, z;
  // using a delay (without the actual delay() function) to read the IMU and perform the Kalman filter calcs.
  // the delay prevents values from exploding due to the extremely short time interval.
  if (millis() - prevtime > t) {
    // the below if statement ensures that both sensors are actually available for the reading and subsequent
    // calculation (has been an issue before).
    if (IMU.gyroscopeAvailable() && IMU.accelerationAvailable()) {
      IMU.readGyroscope(x, y, z);
      float rate = z;                 // gets plugged into the kalman filter
      IMU.readAcceleration(x, y, z);
      float angle = (180/PI) * atan2(sqrt(x*x + z*z), y) - 90;  // gets plugged into the kalman filter. the math obtains
                                                                // the angular position about the z-axis from the 
                                                                // acceleration data.
      float dt = t/1000;              // gets plugged into the kalman filter. 
                                      // convert to s as this is what the filter expects
      // below, the inputs are fed into the filter, which returns the estimated angle value.
      data.estim_angle = kalman.getAngle(angle, rate, dt);
    }
    prevtime = millis();
  }
}

void sendEvent() {
  //Serial.println(data.estim_angle);
  Wire.write(data.angle_bytes, sizeof(data.angle_bytes)); // sends the angle reading over i2c as bytes
}
