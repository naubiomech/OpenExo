

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "SendBulkChar.h"
/**
 * @brief Sends the contents of the txBuffer_bulkStr (the assembled CSV message) 
 * over the primary serial port and clears the buffer afterward.
 * * IMPORTANT: Serial.begin() must be called in setup() before this function runs.
 */
void send_bulk_char() {
	Serial8.begin(115200);
	pinMode(13,OUTPUT);
	digitalWrite(13,HIGH);
	long initial_time = millis();
	bool NanoReachedOut = false;
	while (millis() - initial_time < 9000)
	{
		if (Serial8.available() > 0) {
			digitalWrite(13,LOW);
            char incomingChar = Serial8.read();
			//Serial.println(incomingChar);
			if (incomingChar == 'R') {
				Serial.print("\nNano confirmed to the Teensy that it’s ready to receive the controller parameter list.");
				while (Serial8.available() > 0) {
					incomingChar = Serial8.read();//clear the buffer, as the Nano might have sent many "R"
				}
				delay(50);
				break;
			}
		}
	}
	// delay(5000);
	// digitalWrite(13,LOW);
	// delay(4000);
	// digitalWrite(13,HIGH);
    // 1. Check if the buffer is empty
    if (txBuffer_bulkStr[0] == '\0') {
        Serial.println("Warning: txBuffer_bulkStr is empty. Skipping UART send.");
        return;
    }
    
    // Calculate the exact length of the message.
    size_t message_length = strlen(txBuffer_bulkStr);
	Serial.print("\nsend_bulk_char::payload_bytes=");
	Serial.print(message_length);
	Serial.print(" end_marker_ok=");
	bool marker_ok = (message_length >= 3) &&
		(txBuffer_bulkStr[message_length - 3] == ',') &&
		(txBuffer_bulkStr[message_length - 2] == '?') &&
		(txBuffer_bulkStr[message_length - 1] == '?');
	Serial.print(marker_ok ? "yes" : "no");
    
    // 2. Transmit the entire message in one burst using Serial.write().
    // This is the most efficient method for large C-strings on Arduino.
    Serial8.write(txBuffer_bulkStr, message_length);
	//char myString[] = "f,This is a test char string.\nNew line starts here.,z";
	//Serial8.write(txBuffer_bulkStr, message_length);
    digitalWrite(13,HIGH);
	delay(50);
    // Optional: Add a small delay if the receiving end is slow to process data.
    // delay(5); 

    Serial.println("\n--- Controller parameter list has been sent. ---");
	Serial.print(txBuffer_bulkStr);

    // 3. Clear the buffer to prepare for the next assembly.
    // Crucial to prevent new, shorter messages from containing old data fragments.
    txBuffer_bulkStr[0] = '\0';
	digitalWrite(13,LOW);
	Serial8.end();
}
#endif