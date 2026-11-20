#include <esp_now.h>
#include <WiFi.h>

// This copy exists primarily FOR TESTING PURPOSES. Make sure to update it as well with any changes made to the original!

#define FORCE_SENSOR_PIN_1 2 //ESP32 Xiao pin GPIO2 (ADC0)
#define FORCE_SENSOR_PIN_2 3 //ESP32 Xiao pin GPIO3 (ADC0)

/* 
 * Receiver MAC Address
 * ---------------------
 * Replace this with the MAC address of the receiver ESP32.
 * It identifies where the data will be sent.
 */
uint8_t broadcast_address[] = {0x80, 0xf3, 0xda, 0x5d, 0x95, 0xb8}; //80:f3:da:5d:95:b8 

/* 
 * StructMessage
 * -------------
 * Structure holds the sensor readings that each ESP32 board sends.
 *   - id: identifier for the board (1 for right, 2 for left)
 *   - analog_reading_1: the first sensor value read from the board
 *   - analog_reading_2: the second sensor value read from the board
 *
 * The sender and receiver must have the same structure definitions,
 * otherwise the data could be interpreted incorrectly.
 */
struct struct_message 
{
    int id;
    float analog_reading_1;
    float analog_reading_2;
};

// Create a struct_message called fsr_data
struct_message fsr_data;
// Holds information about the receiver, such as its MAC address
esp_now_peer_info_t peer_info;

// previous recorded time & interval to send at (in ms)
unsigned long prev_time = 0;
unsigned long current_time = 0;

// CHANGE THESE VALUES BASED ON NEEDS
  // desired frequency, in hz (probably deprecated)
  const long freq = 100; 

  // CHANGE THIS ID FOR EACH SENDER
    // Sender 1 (R) --> fsr_data.id = 1;
    // Sender 2 (L) --> fsr_data.id = 2;
  const int selected_id = 2;

// calculates ms interval based on desired frequency
const long interval = 1000 / freq; 

// true/false flag, has data been requested from receiver?
bool send_flag = false;
volatile bool send_inflight = false;
unsigned long send_last_attempt_ms = 0;
const unsigned long send_retry_interval_ms = 10;

// callback when data is sent
// It reports whether the data was successfully delivered to the receiver, and sets the "data requested" flag to false.
void on_data_sent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
  send_inflight = false;

  Serial.print("\r\nLast Packet Send Status:\t");
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Delivery Success");
    Serial.println("Sent values: ");
    Serial.println(fsr_data.analog_reading_1);
    Serial.println(fsr_data.analog_reading_2);
  }
  else
  {
    Serial.println("Delivery Fail");
  }
}

void on_data_recv(const esp_now_recv_info_t *info, const uint8_t *incoming_data, int len)
{
  send_flag = true;
}


void setup() 
{
  fsr_data.id = selected_id;  

  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  else
  {
    Serial.println("ESP-NOW initialized successfully");
  }

  // Register the callback to report send/receive status
  esp_now_register_send_cb(on_data_sent);
  esp_now_register_recv_cb(on_data_recv);

  // Register peer
  memcpy(peer_info.peer_addr, broadcast_address, 6);
  peer_info.channel = 0;  
  peer_info.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peer_info) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

}
 
/* 
 * loop
 * ----
 * Runs repeatedly after setup() finishes.
 * It reads FSR values and caches them to be sent whenever requested by the receiver
 * 
 */

void loop() 
{
  unsigned long now = millis();
  if (send_inflight &&
      (unsigned long)(now - send_last_attempt_ms) >= 100)
  {
    send_inflight = false;
  }

  current_time = millis();

  if (current_time - prev_time >= interval)
  {
    prev_time = current_time;
    
    // Read & cache values to send
    fsr_data.analog_reading_1 = analogRead(FORCE_SENSOR_PIN_1);
    fsr_data.analog_reading_2 = analogRead(FORCE_SENSOR_PIN_2);
  }

  if (send_flag && !send_inflight &&
      (unsigned long)(now - send_last_attempt_ms) >= send_retry_interval_ms)
  {
    send_last_attempt_ms = now;
    send_inflight = true;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcast_address, (uint8_t *) &fsr_data, sizeof(fsr_data));
    
    if (result == ESP_OK) {
      send_flag = false;
      // Serial.println("Sent with success");
      // Serial.println("Sent values: ");
      // Serial.println(fsr_data.analog_reading_1);
      // Serial.println(fsr_data.analog_reading_2);
    }
    else 
    {
      send_inflight = false;
      // Serial.println("Error sending the data");
    }
  }
}
