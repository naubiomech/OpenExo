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

// esp sender's MAC addresses
uint8_t broadcast_address_1[] = {0x58, 0x8c, 0x81, 0xa4, 0x82, 0xfc}; // 58:8c:81:a4:82:fc
uint8_t broadcast_address_2[] = {0x58, 0x8c, 0x81, 0xa4, 0x58, 0xf8}; // 58:8c:81:a4:58:f8

// flag to store which address to broadcast to
uint8_t requested_broadcast_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t ping = 1;

esp_now_peer_info_t peer_info_R;
esp_now_peer_info_t peer_info_L;

// register array to store all registers
uint8_t registers[REG_MAP_SIZE];

// variable to store what register the Teensy is requesting from
volatile uint8_t current_register = 0;
volatile uint8_t last_register;

// variable to check if data has been received over ESPNOW
volatile bool board1_ready = false;
volatile bool board2_ready = false;

volatile bool espnow_send_flag = false;
volatile bool espnow_send_inflight = false;
volatile uint8_t pending_broadcast_address[6] = {0};

unsigned long espnow_last_attempt_ms = 0;
const unsigned long espnow_send_interval_ms = 10;

// Temporary storage for the latest received ESP-NOW packet
struct_message received_fsr_data;   

// These two hold readings from each individual ESP32 sender
struct_message board1_data = {0, 0.0, 0.0};      
struct_message board2_data = {0, 0.0, 0.0};      

// Flags for safe debug printing (set in ISR, printed in loop())
volatile bool i2c_requested_debug_pending = false;
volatile bool i2c_sent_debug_pending = false;
volatile bool i2c_sent_ready_status = false;

volatile uint8_t i2c_last_reg = 0;

// I2C data request callback event
void request_event() 
{  
  // flag to check if the requested board is ready to send
  bool requested_side_ready = (current_register == REG_RIGHT_HEEL || current_register == REG_RIGHT_TOE) ? board1_ready : board2_ready;
  if (!requested_side_ready)
  {
    // if not ready, send 0 over the line to not flood with garbage
    float dummy = 0.0;
    Wire.write((uint8_t*)&dummy, sizeof(float));
    i2c_sent_ready_status = false;
    i2c_requested_debug_pending = true;
    return;
  }

  // set ready status flag to true if ready check is passed
  i2c_sent_ready_status = true;

  // Bounds check
  // if (current_register >= FLOAT_SIZE)
  // {
  //   Wire.write((uint8_t)0x00); // Return dummy byte if invalid
  //   return;
  // }

  // check if sending twice
  // if (current_register == last_register)
  // {
  //   return; // skip sending if it's already been sent
  // }

  // snapshot current register to avoid it being changed by receive_event
  uint8_t reg = current_register;

  // send the register from the register array
  Wire.write(&registers[reg], FLOAT_SIZE);

  // random debug print go
  // Serial.print("Sent I2C register ");
  // Serial.print(reg);
  // Serial.print(", value: ");
  // Serial.println(registers[reg]);

  // mark for safe debug printing in main loop
  i2c_last_reg = reg;
  i2c_sent_debug_pending = true;
}

// I2C data receive callback function
void receive_event(int num_bytes)
{
  if (num_bytes >= 1)
  {
    current_register = Wire.read();
  }

  while (Wire.available())
  {
    Wire.read(); // clear buffer
  }

  // check which side is being requested
    // if right, store addr 1
  if (current_register == REG_RIGHT_HEEL || current_register == REG_RIGHT_TOE)
  {
    memcpy(&requested_broadcast_address, &broadcast_address_1, sizeof(broadcast_address_1));
  }

    //if left, store addr 2
  else if (current_register == REG_LEFT_HEEL || current_register == REG_LEFT_TOE)
  {
    memcpy(&requested_broadcast_address, &broadcast_address_2, sizeof(broadcast_address_2));
  }

  else
  {
    Serial.println("Error picking side.");
  }

  // Mark one pending request and snapshot its address. The I2C callbacks run
  // separately from loop(), so ESP-NOW is only called from loop().
  espnow_send_flag = true;
  memcpy((void *)pending_broadcast_address, requested_broadcast_address, 6);

  // mark for safe debug printing in main loop
  i2c_last_reg = current_register;
  i2c_requested_debug_pending = true;
}

// helper function to write float values into registers
void write_float_to_register(uint8_t reg, float value)
{
  // if (reg + sizeof(float) > REG_MAP_SIZE)
  //   return;  // Prevent overflow

  memcpy(&registers[reg], &value, sizeof(float));
}

// ESPNOW received data callback function
void on_data_recv(const uint8_t * mac, const uint8_t * incoming_data, int len) {
  
  // Copy the received bytes into our struct
  memcpy(&received_fsr_data, incoming_data, sizeof(received_fsr_data));

  // Print out what we received for debugging
  // Serial.print("Received ESPNOW packet from ID ");
  // Serial.print(received_fsr_data.id);
  // Serial.print(": reading1 = ");
  // Serial.print(received_fsr_data.analog_reading_1, 6);
  // Serial.print(", reading2 = ");
  // Serial.println(received_fsr_data.analog_reading_2, 6);


  // Check which sender the packet came from based on ID
  if (received_fsr_data.id == 2) 
  {
    // Save to board 2 struct
    board2_data = received_fsr_data;
    board2_ready = true; 

    // Load board readings into corresponding registers    
    write_float_to_register(REG_LEFT_HEEL, board2_data.analog_reading_1);
    write_float_to_register(REG_LEFT_TOE, board2_data.analog_reading_2);

    // Print what registers we're loading values into for debugging
    // Serial.print(" Packed reading: ");
    // Serial.print(board2_data.analog_reading_1, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_LEFT_HEEL);

    // Serial.print(" Packed reading: ");
    // Serial.print(board2_data.analog_reading_2, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_LEFT_TOE);
  }

  else if (received_fsr_data.id == 1) {
    // Save to board 1 struct
    board1_data = received_fsr_data; 
    board1_ready = true;
    // Load board readings into corresponding registers
    write_float_to_register(REG_RIGHT_HEEL, board1_data.analog_reading_1);
    write_float_to_register(REG_RIGHT_TOE, board1_data.analog_reading_2);

    // // Print what registers we're loading values into for debugging
    // Serial.print(" Packed reading: ");
    // Serial.print(board1_data.analog_reading_1, 6);
    // Serial.print(" into register: ");
    // Serial.println(REG_RIGHT_HEEL);

    // Serial.print(" Packed reading: ");
    // Serial.print(board1_data.analog_reading_2, 6);
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

void on_data_sent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
  espnow_send_inflight = false;
}

void setup() 
{
  Serial.begin(115200); // Start serial monitor for debugging messages
  Serial.println("Setup initiated");

  WiFi.mode(WIFI_STA); // ESP-NOW requires WiFi in Station mode (not Access Point)

  // initialize ESP-NOW. If this fails, print an error message.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW.");
    return;
  }
  
  else
  {
    Serial.println("ESP-NOW intialized.");
  }

  // register our callback function so we get notified when new data is received/sent
  esp_now_register_recv_cb((esp_now_recv_cb_t)on_data_recv);
  esp_now_register_send_cb(on_data_sent);
 
  // Register ESPNOW peer
  memcpy(peer_info_R.peer_addr, broadcast_address_1, 6);
  peer_info_R.channel = 0;  
  peer_info_R.encrypt = false;

  memcpy(peer_info_L.peer_addr, broadcast_address_2, 6);
  peer_info_L.channel = 0;  
  peer_info_L.encrypt = false;
  
  // Add ESPNOW peer        
  if (esp_now_add_peer(&peer_info_R) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  if (esp_now_add_peer(&peer_info_L) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  // configure this ESP32 as an I2C Slave using address 0x08
  Wire.begin(SLAVE_ADDR);

  // register function that sends data to the Teensy whenever it requests
  Wire.onRequest(request_event);

  // declare function that receives data from Teensy
  Wire.onReceive(receive_event);
  
  Serial.println("ESP32 Slave Ready");
}

void loop() {
  if (espnow_send_inflight && (unsigned long)(millis() - espnow_last_attempt_ms) >= 100)
  {
    espnow_send_inflight = false;
  }

  // I2C debug info
    // prints what registers are being requested
  if (i2c_requested_debug_pending) {
    i2c_requested_debug_pending = false;
    Serial.print("I2C register requested: ");
    Serial.println(i2c_last_reg);
  }

  // prints what registers are being sent (and if the callback passes the board-ready check)
  if (i2c_sent_debug_pending)
  {
    i2c_sent_debug_pending = false;
    Serial.println("Request callback triggered.");

    if (i2c_sent_ready_status == true)
    {
      Serial.print("Sent I2C register ");
      Serial.print(i2c_last_reg);
      Serial.print(", value: ");
      Serial.println(registers[i2c_last_reg]);
    }

    else
    {
      Serial.println("Error: Requested board not ready.");
    }
  }
  
  // Send at most one ping at a time. Preserve the pending flag on ESP-NOW
  // allocation errors so loop() can retry after a short backoff.
  unsigned long now = millis();
  if (espnow_send_flag && !espnow_send_inflight &&
      (unsigned long)(now - espnow_last_attempt_ms) >= espnow_send_interval_ms)
  {
    uint8_t target_address[6];
    memcpy(target_address, (const void *)pending_broadcast_address, 6);

    espnow_last_attempt_ms = now;
    espnow_send_inflight = true;
    esp_err_t espnow_error_code = esp_now_send(target_address, (uint8_t *)&ping, sizeof(ping));

    if(espnow_error_code != ESP_OK)
    {
      espnow_send_inflight = false;
      Serial.print("Error sending ping: ");
      Serial.println(espnow_error_code);
    }

    else
    {
      espnow_send_flag = false;
    }
  }
}
