#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
//#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "Arduino.h"
#include <string.h>
#include "GetBulkChar.h"


// --- Shared Global Constants (MUST MATCH SENDER) ---
// Define the maximum size for the incoming message buffer.
//const int MAX_MESSAGE_SIZE = 25000; 

// --- Receiver Variables ---
//char rxBuffer_bulkStr[25000]; // Buffer to store the received data payload
char rxBuffer_bulkStr[MAX_MESSAGE_SIZE]; // Buffer to store the received data payload
int rxIndex = 0;                 // Current index for writing into rxBuffer_bulkStr
bool messageComplete = false;    // Flag indicating a complete message is ready

// State tracking for the serial reception
enum RxState {
    WAITING_FOR_F,      // Looking for the start character 'f'
    WAITING_FOR_COMMA,  // Found 'f', now looking for the first delimiter ','
    RECEIVING_DATA      // Receiving the entire message frame
};

RxState currentState = WAITING_FOR_F;


// --- New Function: Blocking Message Reader ---
/**
 * @brief Synchronously reads the entire message frame ("f,data,z") from UART.
 * This function blocks indefinitely until a full message is received and processed.
 */
void readSingleMessageBlocking() {
	long initialTime = millis();
    Serial.println("\n--- Entering Blocking Read Mode ---");
    Serial.println("System will halt execution until a full frame is received.");
    Serial1.begin(115200);
	//delay(3000);
	
    char txBuffer_NanoReady[2] = "R";
	size_t message_length = strlen(txBuffer_NanoReady);
    // 2. Transmit the entire message in one burst using Serial.write().
    // This is the most efficient method for large C-strings on Arduino.
    while (!Serial1.available()) {
		Serial1.write(txBuffer_NanoReady, message_length);
		Serial.print("\nCharacter R sent.");
		digitalWrite(LEDR, HIGH);
		digitalWrite(LEDG, LOW);
		digitalWrite(LEDB, HIGH);
		delay(20);
		if (millis() - initialTime > 10000) {
			break;
		}
	}
	
	digitalWrite(LEDR, HIGH);
	digitalWrite(LEDG, LOW);
	digitalWrite(LEDB, LOW);
	
    // The loop runs indefinitely until the messageComplete flag is set to true.
    while (!messageComplete) {
        // Only proceed if data is available in the UART buffer
        while (Serial1.available()) {
			Serial.print("\nSerial.available() > 0, incomingChar:");
            char incomingChar = Serial1.read();
			Serial.println(incomingChar);

            // State 1: WAITING_FOR_F
            if (currentState == WAITING_FOR_F) {
                if (incomingChar == 'f') {
                    // Store 'f' and transition
                    if (rxIndex < MAX_MESSAGE_SIZE - 1) {
                        rxBuffer_bulkStr[rxIndex++] = incomingChar;
                        currentState = WAITING_FOR_COMMA;
                    }
					else {
                        // Buffer overflow on first character
                        currentState = WAITING_FOR_F;
                        rxIndex = 0;
                    }
                }
            }
            
            // State 2: WAITING_FOR_COMMA
            else if (currentState == WAITING_FOR_COMMA) {
                if (incomingChar == ',') {
                    // Found the first delimiter: Store ',' and begin RECEIVING_DATA
                    if (rxIndex < MAX_MESSAGE_SIZE - 1) {
                        rxBuffer_bulkStr[rxIndex++] = incomingChar;
                        currentState = RECEIVING_DATA;
                        // rxIndex is now 2 (pointing to the start of the payload)
                    }
					else {
                        // Buffer overflow protection: reset and wait again for 'f'
                        Serial.println("ERROR: Receive buffer overflow during start marker.");
                        currentState = WAITING_FOR_F;
                        rxIndex = 0;
                    }
                }
				else {
                    // If 'f' was followed by something other than ',', discard 'f' and restart
                    currentState = WAITING_FOR_F;
                    rxIndex = 0; // Discard the already stored 'f'
                }
            } 
            
            // State 3: RECEIVING_DATA
            else if (currentState == RECEIVING_DATA) {
                
                // Check for buffer overflow first
                if (rxIndex >= MAX_MESSAGE_SIZE - 1) {
                    Serial.println("ERROR: Receive buffer overflow.");
                    currentState = WAITING_FOR_F;
                    rxIndex = 0;
                    continue; // Skip processing this character
                }
                
                // --- 1. CHECK FOR END MARKER (",z") ---
                // We check the incoming character against the previous one stored in the buffer.
                if (incomingChar == '?' && rxBuffer_bulkStr[rxIndex - 1] == '?' && rxBuffer_bulkStr[rxIndex - 2] == ',') {
                    
                    // Store the 'z'
                    rxBuffer_bulkStr[rxIndex++] = incomingChar;
                    
                    // Add the null terminator after 'z'
                    rxBuffer_bulkStr[rxIndex] = '\0'; 
                        
                    messageComplete = true; // Exit the blocking while loop
                        
                    // Continue to the processing block below
                }

                // --- 2. STORE DATA ---
                rxBuffer_bulkStr[rxIndex] = incomingChar;
                rxIndex++;
            }
			// 3. Immediately exit the inner 'while' loop if the message is complete
            if (messageComplete) {
                break;
            }
        }
		if (millis() - initialTime > 8000) {
			break;
		}
        // Optional: Introduce a small delay if no data is available to prevent watchdog timer resets on some boards.
        // delay(1); 
    } // End of while (!messageComplete)
	
	Serial1.end();
    // Reset the state machine to be ready for the *next* time this function is called (if ever)
    // Note: Since this is in setup(), we typically won't run again, but it's clean practice.
    // messageComplete = false; // We don't reset this, as it's the loop exit condition.
    currentState = WAITING_FOR_F;
    rxIndex = 0;
	
	digitalWrite(LEDR, HIGH);
	digitalWrite(LEDG, HIGH);
	digitalWrite(LEDB, HIGH);
}

#endif