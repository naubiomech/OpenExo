#ifndef PLOTTINGTITLES_H
#define PLOTTINGTITLES_H

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "StatusDefs.h"
#include "Config.h"

//Specific Libraries
#include "ParseIni.h"
#include "ParamsFromSD.h"
#include "ListCtrlParams.h"


#include <cstddef> // Required for size_t
#include <cstdint> // Required for uint8_t

// Maximum buffer size estimate (must be large enough for "t," + 11 columns + 10 commas + ",z" + null terminator)
const size_t MAX_COMBINED_HEADER_LENGTH = 400; 

// --- Definition of the Mapping Function (INLINE, DYNAMIC) ---

/**
 * @brief Returns the column header string based on the configuration ID and 
 * the 0-based column index (0 to 10).
 */
inline const char* getColumnHeader(uint8_t column_index, uint8_t* config_to_send) {
    //using namespace config_defs;

    // Outer switch: Selects the set of headers based on the Exo Type ID
    switch (config_to_send[config_defs::exo_name_idx]) {
        
        case (uint8_t)config_defs::exo_name::bilateral_ankle:
        {
            // Inner switch: Selects the specific column name for this mode (0-based)
            switch (column_index) {
                case 0:  return "Desired Torque (L)";
                case 1:  return "Measured Torque (L)";
                case 2:  return "Desired Torque (R)";
                case 3:  return "Measured Torque (R)";
                case 4:  return "Toe FSR (L)";
                case 5:  return "Stance Phase (L)";
                case 6:  return "Toe FSR (R)";
                case 7:  return "Stance Phase (R)";
                case 8:  return "Channel 8";
                case 9:  return "Exoskeleton time (seconds)";
                case 10: return "Battery Level (Volts)";
                default: return "INVALID_COL";
            }
        }
		
		case (uint8_t)config_defs::exo_name::bilateral_hip:
        {
            // Inner switch: Selects the specific column name for this mode (0-based)
            switch (column_index) {
                case 0:  return "Measured Torque (R)";
                case 1:  return "Desired Torque (R)";
                case 2:  return "Measured Torque (L)";
                case 3:  return "Desired Torque (L)";
                case 4:  return "Gait/100 (R)";
                case 5:  return "Toe FSR (R)";
                case 6:  return "Gait/100 (L)";
                case 7:  return "Toe FSR (L)";
                case 8:  return "Heel FSR (R)";
                case 9:  return "Heel FSR (L)";
                case 10: return "Battery Level (Volts)";
                default: return "INVALID_COL";
            }
        }
        
        // DEFAULT CASE: Generic Data Headers
        default:
        {
            switch (column_index) {
                case 0:  return "Channel 0";
                case 1:  return "Channel 1";
                case 2:  return "Channel 2";
                case 3:  return "Channel 3";
                case 4:  return "Channel 4";
                case 5:  return "Channel 5";
                case 6:  return "Channel 6";
                case 7:  return "Channel 7";
                case 8:  return "Channel 8";
                case 9:  return "Exoskeleton time (seconds)";
                case 10: return "Battery Level (Volts)";
                default: return "INVALID_COL";
            }
        }
    }
}


/**
 * @brief Function declaration to combine the column strings into a single delimited C-string.
 */
//bool create_plotting_titles(char* output_buffer, size_t buffer_size, uint8_t* config_to_send);
void create_plotting_titles(uint8_t* config_to_send);

#endif
#endif




