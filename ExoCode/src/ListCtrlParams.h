#ifndef ListCtrlParams_h
#define ListCtrlParams_h



#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "ExoData.h"
#include "ParseIni.h"
#include "Utilities.h"
#include "ParamsFromSD.h"
#include "PlottingTitles.h"

#include <SPI.h>
#include <SD.h>
#include <map>
#include <string>

#ifndef SD_SELECT
    #define SD_SELECT BUILTIN_SDCARD
#endif

// 1 = debug: for each joint side allowed by exo_side, include every controller CSV (very large Nano→GUI payload).
// 0 = production: only joints whose config.ini default controller is not "disabled" (>1); use with
// LIST_CTRL_PARAMS_SEND_DEFAULT_CONTROLLER_ONLY to shrink further.
#ifndef LIST_CTRL_PARAMS_SEND_MAX
    #define LIST_CTRL_PARAMS_SEND_MAX 0
#endif

// When 1 (and LIST_CTRL_PARAMS_SEND_MAX is 0): handshake lists only each joint's default controller CSV
// (line 5 names + line 6 values via readAndParseValuesRow).
// When 0: list all controllers for each joint that passes the gate above (still smaller than SEND_MAX).
#ifndef LIST_CTRL_PARAMS_SEND_DEFAULT_CONTROLLER_ONLY
    #define LIST_CTRL_PARAMS_SEND_DEFAULT_CONTROLLER_ONLY 1
#endif

	// Define the size of the array and the max length for each string
	const int MAX_COLUMNS = 30;
	const int MAX_STRING_LENGTH = 10;
	// Two rows are emitted per controller:
	// 1) names row from CSV line 5
	// 2) values row from CSV line 6 (prefixed with "v")
	const int MAX_SNAPSHOTS = 4 * ((uint8_t)config_defs::ankle_controllers::Count + (uint8_t)config_defs::hip_controllers::Count + (uint8_t)config_defs::knee_controllers::Count + (uint8_t)config_defs::elbow_controllers::Count + (uint8_t)config_defs::arm_1_controllers::Count + (uint8_t)config_defs::arm_2_controllers::Count);
	// Calculate the MAX size of the transmission buffer:
	// (Max Chars per Cell + 1 comma delimiter) * MAX_COLUMNS + 
	// (+ 1 newline character) * MAX_SNAPSHOTS + 
	// (+ 1 for the final null-terminator)
	const size_t MAX_MESSAGE_SIZE = 
		(MAX_STRING_LENGTH + 1) * MAX_COLUMNS * MAX_SNAPSHOTS + MAX_SNAPSHOTS + 1;
	

	
	
	
	

	// Array to hold the strings from the fifth row

void ctrl_param_array_gen(uint8_t* config_to_send);
int readAndParseFifthRow(const char* filename_char, char arr[][MAX_COLUMNS][MAX_STRING_LENGTH], int maxCols, int maxLen, uint8_t row_idx, int i_ctrl);
int readAndParseValuesRow(const char* filename_char, char arr[][MAX_COLUMNS][MAX_STRING_LENGTH], int maxCols, int maxLen, uint8_t row_idx, int i_ctrl);
void create_csv_message();
bool retrieveJointAndController(const char* filename_char, char* joint_out, char* controller_out);

// Define txBuffer_bulkStr here too, as it's also global data
    // The 1D buffer that will hold the final, flattened CSV string
	extern char txBuffer_bulkStr[MAX_MESSAGE_SIZE];
extern char stringArray[MAX_SNAPSHOTS][MAX_COLUMNS][MAX_STRING_LENGTH];
extern uint8_t failed2open;
extern uint8_t joint_id_val;
extern char jointName[10];

// Define the number of prefix columns to insert
const int PREFIX_COLS = 4;
const size_t MAX_NAME_LENGTH = 64;


#endif
#endif
