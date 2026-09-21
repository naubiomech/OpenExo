#include <Wire.h>      // Allows I2C communication
#include <esp_now.h>   // Library for ESP-NOW wireless communication
#include <WiFi.h>      // Required for WiFi/ESP-NOW setup

#define SLAVE_ADDR 0x08  // I2C address this ESP32 will use as a slave

// Register map
#define REG_LEFT_HEEL 0x00  // 4 bytes (float)
#define REG_LEFT_TOE 0x04  // 4 bytes
#define REG_RIGHT_HEEL 0x08  // 4 bytes
#define REG_RIGHT_TOE 0x0C  // 4 bytes

#define REG_MAP_SIZE 16    // total register space (4 floats × 4 bytes)

#define FLOAT_SIZE 4 // size of float, to be used in Wire.write()

struct struct_message 
{
    int id;               // ID number of the sending board (1 or 2)
    float analog_reading_1; // First analog reading
    float analog_reading_2; // Second analog reading
};

// register array to store all registers
uint8_t registers[REG_MAP_SIZE];

// variable to store what register the Teensy is requesting from
volatile uint8_t currentRegister = 0;
volatile uint8_t lastRegister;

// variable to check if data has been received over ESPNOW
volatile bool board1Ready = false;
volatile bool board2Ready = false;

// Temporary storage for the latest received ESP-NOW packet
struct_message recvd_fsr_data;   

// These two hold readings from each individual ESP32 sender
struct_message board1_data = {0, 0.0, 0.0};      
struct_message board2_data = {0, 0.0, 0.0};      

// Flags for safe debug printing (set in ISR, printed in loop())
volatile bool i2cDebugPending = false;
volatile uint8_t i2cLastReg = 0;

void requestEvent() {

  if (!board1Ready || !board2Ready) 
  {
    float dummy = 0.0;
    Wire.write((uint8_t*)&dummy, sizeof(float));  // send 0 until data exists
    return;
  }

  // Bounds check
  if (currentRegister >= REG_MAP_SIZE)
  {
    Wire.write((uint8_t)0x00); // Return dummy byte if invalid
    return;
  }

  // check if sending twice
  // if (currentRegister == lastRegister)
  // {
  //   return; // skip sending if it's already been sent
  // }

  // snapshot current register to avoid it being changed by receiveEvent
  uint8_t reg = currentRegister;

  // send the register array
  Wire.write(&registers[reg], REG_MAP_SIZE);

  // mark for safe debug printing in main loop
  i2cLastReg = reg;
  i2cDebugPending = true;
}

// function to receive what register the teensy wants data from
void receiveEvent(int numBytes)
{
  if (numBytes >= 1)
  {
    currentRegister = Wire.read();
  }

  while (Wire.available())
  {
    Wire.read(); // clear buffer
  }

  // mark for safe debug printing in main loop
  i2cLastReg = currentRegister;
  i2cDebugPending = true;
}

// helper function to write float values into registers
void writeFloatToRegister(uint8_t reg, float value)
{
  if (reg + sizeof(float) > REG_MAP_SIZE)
    return;  // Prevent overflow

  memcpy(&registers[reg], &value, sizeof(float));
}


void onDataRecv(const uint8_t * mac, const uint8_t * incomingData, int len) {

  // Copy the received bytes into our struct
  memcpy(&recvd_fsr_data, incomingData, sizeof(recvd_fsr_data));

  // Print out what we received for debugging
  Serial.print("Received ESPNOW packet from ID ");
  Serial.print(recvd_fsr_data.id);
  Serial.print(": reading1 = ");
  Serial.print(recvd_fsr_data.analog_reading_1, 6);
  Serial.print(", reading2 = ");
  Serial.println(recvd_fsr_data.analog_reading_2, 6);


  // Check which sender the packet came from based on ID
  if (recvd_fsr_data.id == 1) {
    // Save to board 1 struct
    board1_data = recvd_fsr_data;
    board1Ready = true; 
    // Load board readings into corresponding registers    
    writeFloatToRegister(REG_LEFT_HEEL, board1_data.analog_reading_1);
    writeFloatToRegister(REG_LEFT_TOE, board1_data.analog_reading_2);

    // Print what registers we're loading values into for debugging
    // Serial.print(" Packed reading: ");
    // Serial.print(board1_data.analog_reading_1, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_LEFT_HEEL);

    // Serial.print(" Packed reading: ");
    // Serial.print(board1_data.analog_reading_2, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_LEFT_TOE);
  }

  else if (recvd_fsr_data.id == 2) {
    // Save to board 2 struct
    board2_data = recvd_fsr_data; 
    board2Ready = true;
    // Load board readings into corresponding registers
    writeFloatToRegister(REG_RIGHT_HEEL, board2_data.analog_reading_1);
    writeFloatToRegister(REG_RIGHT_TOE, board2_data.analog_reading_2);

    // Print what registers we're loading values into for debugging
    // Serial.print(" Packed reading: ");
    // Serial.print(board2_data.analog_reading_1, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_RIGHT_HEEL);

    // Serial.print(" Packed reading: ");
    // Serial.print(board2_data.analog_reading_2, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_RIGHT_TOE);
  }

  else {
    // if ID doesn’t match 1 or 2, something is wrong
    Serial.println("Error reading FSR data: Unknown board ID.");
  }

  // snippet to read the register value status for debugging
  // float regCheck[4];
  // memcpy(regCheck, registers, REG_MAP_SIZE);

  // Serial.print("Registers status: ");
  // for(int i = 0; i < 4; i++)
  // {
  //   Serial.print(regCheck[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("");

}

void setup() {

  Serial.begin(9600); // Start serial monitor for debugging messages

  WiFi.mode(WIFI_STA); // ESP-NOW requires WiFi in Station mode (not Access Point)

  // initialize ESP-NOW. If this fails, print an error message.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW.");
    return;
  }

  // register our callback function so we get notified when new data arrives
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  // configure this ESP32 as an I2C Slave using address 0x08
  Wire.begin(SLAVE_ADDR);

  // register function that sends data to the Teensy whenever it requests
  Wire.onRequest(requestEvent);

  // declare function that receives data from Teensy
  Wire.onReceive(receiveEvent);
  
  Serial.println("ESP32 Slave Ready");
}

void loop() {
  // Safe place to print I2C debug info (not in ISR)
  if (i2cDebugPending) {
    i2cDebugPending = false;
    // Serial.print("I2C register requested/sent: ");
    // Serial.println(i2cLastReg);
  }

}
