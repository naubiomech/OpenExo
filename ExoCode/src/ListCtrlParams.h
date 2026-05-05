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

	// Define the size of the array and the max length for each string.
	// We now store TWO rows per controller (names + values), so the row
	// budget is 4 * sum(controllers): 2x for left/right joints, 2x for
	// names+values rows.
	const int MAX_COLUMNS = 30;
	const int MAX_STRING_LENGTH = 10;
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

	// Big controller-snapshot array.  Declared extern here and defined
	// once in ListCtrlParams.cpp (placed in RAM2 via DMAMEM) so each
	// translation unit that includes this header doesn't get its own
	// multi-KB copy.  That used to blow the Teensy 4.1 RAM1 budget.
	extern char stringArray[MAX_SNAPSHOTS][MAX_COLUMNS][MAX_STRING_LENGTH];

	extern uint8_t failed2open;
	extern uint8_t joint_id_val;
	extern char jointName[10];

	// Define the number of prefix columns to insert
	const int PREFIX_COLS = 4;
	const size_t MAX_NAME_LENGTH = 64;


#endif
#endif
