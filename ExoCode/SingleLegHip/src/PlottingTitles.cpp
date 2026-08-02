#include "PlottingTitles.h"

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include <Arduino.h>
#include <cstring>  // Required for strcpy, strcat, strlen
// The Serial object is typically made available by including the .ino file or Arduino.h

/**
 * @brief Combines the 11 dynamic column header strings into a single, 
 * comma-delimited row with "t," prefix and appends it to the bulk payload.
 *
 * @param config_to_send Active firmware configuration used to select labels.
 */
void create_plotting_titles(uint8_t* config_to_send) {
	char output_buffer[MAX_COMBINED_HEADER_LENGTH];
	size_t buffer_size = sizeof(output_buffer);
    // Delimiter strings
    const char START_MARKER[] = "t,";
    const char END_MARKER[] = "\n,??";
    const size_t START_LEN = strlen(START_MARKER);
    const size_t END_LEN = strlen(END_MARKER);
    
    // This constant ensures we only process 11 columns (index 0 to 10)
    const size_t num_columns = 11; 

    // Initial check for minimum size including START and END markers
    if (buffer_size < MAX_COMBINED_HEADER_LENGTH) {
        Serial.println("Error: Header buffer too small or null."); 
        return;
    }

    // 1. Initialize the buffer with the start marker "t,"
    if (buffer_size < START_LEN + END_LEN + 1) { // Check minimum space for just markers
        Serial.println("Error: Cannot fit start and end markers.");
        return;
    }
    
    // Copy "t," to the buffer and track the current length
    strcpy(output_buffer, START_MARKER);
    size_t current_len = START_LEN; 

    // 2. Concatenate all columns using the dynamic getter
    // The loop runs exactly 11 times (i=0 to i=10)
    for (size_t i = 0; i < num_columns; ++i) { 
        
        // --- CORE CALL: Get the column name dynamically (index runs 0 to 10) ---
        const char* current_col = getColumnHeader(i,config_to_send); // Passed i directly
        size_t col_len = strlen(current_col);
        
        // Determine the length of the delimiter to append (1 for comma or 0 for none)
        size_t delimiter_len = (i < num_columns - 1) ? 1 : 0; // 1 if not the last column

        // Check space needed: current length + column length + delimiter + END marker + null terminator
        size_t required_space_for_data = current_len + col_len + delimiter_len + END_LEN + 1;

        if (required_space_for_data > buffer_size) {
            Serial.println("Error: Header buffer overflow prevented during string combination.");
            output_buffer[0] = '\0'; // Clear buffer on failure
            return;
        }

        // a. Append the column string
        strcat(output_buffer, current_col);
        current_len += col_len;

        // b. Append the comma delimiter if it's not the last column
        if (delimiter_len == 1) {
            strcat(output_buffer, ",");
            current_len += 1;
        }
    }
    
    // 3. Append the final payload end marker on its own row.
    strcat(output_buffer, END_MARKER);

    // ctrl_param_array_gen() already terminates the controller payload with ",??".
    // Remove that terminator before appending the plotting titles so the Nano
    // receives one complete frame instead of stopping before the title row.
    size_t tx_len = strlen(txBuffer_bulkStr);
    if (tx_len >= 3 &&
        txBuffer_bulkStr[tx_len - 3] == ',' &&
        txBuffer_bulkStr[tx_len - 2] == '?' &&
        txBuffer_bulkStr[tx_len - 1] == '?') {
        txBuffer_bulkStr[tx_len - 3] = '\0';
        tx_len -= 3;
    }

    if (tx_len > 0 && txBuffer_bulkStr[tx_len - 1] != '\n') {
        if (tx_len + 1 >= MAX_MESSAGE_SIZE) {
            Serial.println("Error: Header buffer overflow prevented while adding row delimiter.");
            return;
        }
        strcat(txBuffer_bulkStr, "\n");
        tx_len += 1;
    }

    if (tx_len + strlen(output_buffer) >= MAX_MESSAGE_SIZE) {
        Serial.println("Error: Header buffer overflow prevented while appending plotting titles.");
        return;
    }
	strcat(txBuffer_bulkStr, output_buffer);
    
}
#endif // COLUMN_DEFS_H
