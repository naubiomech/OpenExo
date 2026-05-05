

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
				NanoReachedOut = true;
				while (Serial8.available() > 0) {
					incomingChar = Serial8.read();//clear the buffer, as the Nano might have sent many "R"
				}
				delay(50);
				break;
			}
		}
	}
	// CRITICAL: only transmit the bulk CSV if the Nano actually handshook.
	//
	// If the Teensy is plugged into USB AFTER the Nano has already booted,
	// the Nano is no longer in readSingleMessageBlocking() -- it's in the
	// main loop, with UARTHandler treating Serial1 RX as a binary command
	// stream.  Spewing the 1700+ byte CSV at it under those conditions
	// jams UARTHandler's parser (it interprets random bulk bytes as a
	// command + payload that never completes), the main loop stalls, and
	// BLE notifications stop -- which is exactly the "Nano stops
	// transmitting BLE when Teensy is plugged in" symptom.  Bailing out
	// here keeps the Nano running cleanly even when the boot order is
	// reversed.  Reset the Nano if you want to populate the controller
	// list after a late Teensy power-up.
	if (!NanoReachedOut) {
		Serial.println("\nsend_bulk_char: Nano did not handshake (already past boot?). Skipping bulk send.");
		Serial8.end();
		return;
	}
    // 1. Check if the buffer is empty
    if (txBuffer_bulkStr[0] == '\0') {
        Serial.println("Warning: txBuffer_bulkStr is empty. Skipping UART send.");
        return;
    }

    // Calculate the exact length of the message.
    size_t message_length = strlen(txBuffer_bulkStr);

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