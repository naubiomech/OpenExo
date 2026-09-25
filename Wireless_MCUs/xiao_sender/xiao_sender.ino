#include <esp_now.h>
#include <WiFi.h>

#define FORCE_SENSOR_PIN_1 2 //ESP32 Xiao pin GPIO2 (ADC0)
#define FORCE_SENSOR_PIN_2 3 //ESP32 Xiao pin GPIO3 (ADC0)

/* 
 * Receiver MAC Address
 * ---------------------
 * Replace this with the MAC address of the receiver ESP32.
 * It identifies where the data will be sent.
 */
uint8_t broadcastAddress[] = {0x80, 0xf3, 0xda, 0x5d, 0x95, 0xb8}; //80:f3:da:5d:95:b8

/* 
 * StructMessage
 * -------------
 * Structure holds the sensor readings that each ESP32 board sends.
 *   - id: identifier for the board (1 for Board #1, 2 for Board #2)
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
esp_now_peer_info_t peerInfo;

// previous recorded time & interval to send at (in ms)
unsigned long prevTime = 0;
unsigned long currentTime = 0;

const long interval = 100;

// callback when data is sent
// It reports whether the data was successfully delivered to the receiver.
void on_data_sent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Delivery Success");
  }
  else
  {
    Serial.println("Delivery Fail");
  }
}
 
void setup() 
{
  // CHANGE THIS ID FOR EACH SENDER
  // Sender 1 --> fsr_data.id = 1;
  // Sender 2 --> fsr_data.id = 2;
  fsr_data.id = 2;  

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

  // Register the callback to report send status
  esp_now_register_send_cb((esp_now_send_cb_t)on_data_sent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

}
 
/* 
 * loop
 * ----
 * Runs repeatedly after setup() finishes.
 * It reads FSR values, packages them into a StructMessage,
 * and sends the data to the receiver every 2 seconds.
 */

void loop() 
{
  currentTime = millis();

  if (currentTime - prevTime >= interval)
  {
    prevTime = currentTime;
    
    // Set values to send
    fsr_data.analog_reading_1 = analogRead(FORCE_SENSOR_PIN_1);
    fsr_data.analog_reading_2 = analogRead(FORCE_SENSOR_PIN_2);
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &fsr_data, sizeof(fsr_data));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else 
    {
      Serial.println("Error sending the data");
    }
  }


}
