

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "ListCtrlParams.h"

#ifndef DMAMEM
#define DMAMEM
#endif

char txBuffer_bulkStr[MAX_MESSAGE_SIZE];
DMAMEM char stringArray[MAX_SNAPSHOTS][MAX_COLUMNS][MAX_STRING_LENGTH];
uint8_t failed2open;
uint8_t joint_id_val;
char jointName[10];

static bool append_to_tx_buffer(const char* s) {
	if (s == nullptr) {
		return false;
	}
	size_t cur = strlen(txBuffer_bulkStr);
	size_t add = strlen(s);
	if (cur + add >= MAX_MESSAGE_SIZE) {
		return false;
	}
	memcpy(txBuffer_bulkStr + cur, s, add + 1);
	return true;
}

#if LIST_CTRL_PARAMS_SEND_MAX
static bool is_joint_side_allowed_for_config(int i_joint, uint8_t exo_side) {
	const bool is_left_joint = (i_joint % 2) == 1;
	if (exo_side == (uint8_t)config_defs::exo_side::bilateral) {
		return true;
	}
	if (exo_side == (uint8_t)config_defs::exo_side::left) {
		return is_left_joint;
	}
	if (exo_side == (uint8_t)config_defs::exo_side::right) {
		return !is_left_joint;
	}
	return false;
}
#endif

void ctrl_param_array_gen(uint8_t* config_to_send) {
	// DMAMEM is not guaranteed to be zeroed on boot; clear explicitly.
	memset(stringArray, 0, sizeof(stringArray));
	memset(txBuffer_bulkStr, 0, sizeof(txBuffer_bulkStr));

	//Begin SD card
	if (!SD.begin(SD_SELECT)) {
			while (1)
			
			if (Serial)
			{
				// logger::print("SD.begin() failed");
				// logger::print("\n");
				Serial.println("SD.begin() failed");
			}
	}
	
	// Serial.print("\nconst size_t MAX_MESSAGE_SIZE = ");
	// Serial.print(MAX_MESSAGE_SIZE);
	
	uint8_t csvCount;
	uint8_t row_idx = 0;
	failed2open = 0;
	uint16_t names_rows_added = 0;
	uint16_t values_rows_added = 0;
	
	//Loop through joints
	for (int i_joint = 1; i_joint < 13; i_joint++) {
		switch (i_joint)
		{
		case 1://left ankle
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::ankle_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_ankle_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::ankle_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 2://right ankle
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::ankle_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_ankle_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::ankle_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;	
		case 3://left hip
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::hip_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_hip_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::hip_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 4://right hip
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::hip_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_hip_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::hip_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 5://left knee
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::knee_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_knee_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::knee_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 6://right knee
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::knee_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_knee_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::knee_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 7://left elbow
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::elbow_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_elbow_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::elbow_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 8://right elbow
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::elbow_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_elbow_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::elbow_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 9://left arm 1
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::arm_1_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_arm_1_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::arm_1_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 10://right arm 1
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::arm_1_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_arm_1_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::arm_1_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 11://left arm 2
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::arm_2_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_arm_2_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::arm_2_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		case 12://right arm 2
		#if LIST_CTRL_PARAMS_SEND_MAX
		if (!is_joint_side_allowed_for_config(i_joint, config_to_send[config_defs::exo_side_idx])) { continue; }
		csvCount = (uint8_t)config_defs::arm_2_controllers::Count;
		#else
		if ((config_to_send[config_defs::exo_arm_2_default_controller_idx] > 1) && ((((uint8_t)config_defs::exo_side::bilateral == config_to_send[config_defs::exo_side_idx])) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]))))
		{
			csvCount = (uint8_t)config_defs::arm_2_controllers::Count;
		}
		else {
			continue;
		}
		#endif
			break;
		}
		
	
		//Configure
		//Serial.print("\n\n\n\nTotal number of controllers: ");
		//Serial.print(csvCount);

#if !LIST_CTRL_PARAMS_SEND_MAX && LIST_CTRL_PARAMS_SEND_DEFAULT_CONTROLLER_ONLY
		uint8_t default_ctrl_for_joint = 0;
		switch (i_joint)
		{
			case 1:
			case 2:
				default_ctrl_for_joint = config_to_send[config_defs::exo_ankle_default_controller_idx];
				break;
			case 3:
			case 4:
				default_ctrl_for_joint = config_to_send[config_defs::exo_hip_default_controller_idx];
				break;
			case 5:
			case 6:
				default_ctrl_for_joint = config_to_send[config_defs::exo_knee_default_controller_idx];
				break;
			case 7:
			case 8:
				default_ctrl_for_joint = config_to_send[config_defs::exo_elbow_default_controller_idx];
				break;
			case 9:
			case 10:
				default_ctrl_for_joint = config_to_send[config_defs::exo_arm_1_default_controller_idx];
				break;
			case 11:
			case 12:
				default_ctrl_for_joint = config_to_send[config_defs::exo_arm_2_default_controller_idx];
				break;
			default:
				break;
		}
#endif
		
		int start_ctrl = 2; // Skip disabled controller for all joints.
		for (int i_ctrl = start_ctrl; i_ctrl < csvCount; i_ctrl++) {
#if !LIST_CTRL_PARAMS_SEND_MAX && LIST_CTRL_PARAMS_SEND_DEFAULT_CONTROLLER_ONLY
			if ((int)default_ctrl_for_joint != i_ctrl) {
				continue;
			}
#endif
			bool csvExists;
			std::string filename;
			switch (i_joint)
			{
			case 1://left ankle
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_ankle;
				joint_id_val = (uint8_t)config_defs::joint_id::left_ankle;
				strncpy(jointName, "Ankle(L)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::ankle.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::ankle.at(i_ctrl);
				}
				break;
			case 2://right ankle
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_ankle;
				joint_id_val = (uint8_t)config_defs::joint_id::right_ankle;
				strncpy(jointName, "Ankle(R)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::ankle.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::ankle.at(i_ctrl);
				}
				break;
			case 3://left hip
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_hip;
				joint_id_val = (uint8_t)config_defs::joint_id::left_hip;
				strncpy(jointName, "Hip(L)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::hip.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::hip.at(i_ctrl);
				}
				break;
			case 4://right hip
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_hip;
				joint_id_val = (uint8_t)config_defs::joint_id::right_hip;
				strncpy(jointName, "Hip(R)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::hip.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::hip.at(i_ctrl);
				}
				break;
			case 5://left knee
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_knee;
				joint_id_val = (uint8_t)config_defs::joint_id::left_knee;
				strncpy(jointName, "Knee(L)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::knee.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::knee.at(i_ctrl);
				}
				break;
			case 6://right knee
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_knee;
				joint_id_val = (uint8_t)config_defs::joint_id::right_knee;
				strncpy(jointName, "Knee(R)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::knee.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::knee.at(i_ctrl);
				}
				break;
			case 7://left elbow
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_elbow;
				joint_id_val = (uint8_t)config_defs::joint_id::left_elbow;
				strncpy(jointName, "Elbow(L)", 10); 
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::elbow.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::elbow.at(i_ctrl);
				}
				break;
			case 8://right elbow
				//joint_id_string_l = (uint8_t)config_defs::joint_id::left_elbow;
				joint_id_val = (uint8_t)config_defs::joint_id::right_elbow;
				strncpy(jointName, "Elbow(R)", 10); 
				// Ensure the array is null-terminated at the end of its allocated space
				// to prevent printing garbage if the source string was 10 chars long.
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::elbow.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::elbow.at(i_ctrl);
				}
				break;
			case 9://left arm 1
				joint_id_val = (uint8_t)config_defs::joint_id::left_arm_1;
				strncpy(jointName, "Arm1(L)", 10);
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::arm_1.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::arm_1.at(i_ctrl);
				}
				break;
			case 10://right arm 1
				joint_id_val = (uint8_t)config_defs::joint_id::right_arm_1;
				strncpy(jointName, "Arm1(R)", 10);
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::arm_1.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::arm_1.at(i_ctrl);
				}
				break;
			case 11://left arm 2
				joint_id_val = (uint8_t)config_defs::joint_id::left_arm_2;
				strncpy(jointName, "Arm2(L)", 10);
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::arm_2.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::arm_2.at(i_ctrl);
				}
				break;
			case 12://right arm 2
				joint_id_val = (uint8_t)config_defs::joint_id::right_arm_2;
				strncpy(jointName, "Arm2(R)", 10);
				jointName[9] = '\0';
				csvExists = controller_parameter_filenames::arm_2.count(i_ctrl);
				if (csvExists) {
					filename = controller_parameter_filenames::arm_2.at(i_ctrl);
				}
				break;
			} 
			
			if (csvExists) { // condition is true if count is 1
				if (row_idx >= MAX_SNAPSHOTS) {
					break;
				}
				//Serial.print("\n\nController ");
				//Serial.print((int)i_ctrl);
				//Serial.println(" has a csv.");

				const char* filename_char = filename.c_str();
				
				// Row 1: parameter names from CSV line 5
				int columnsRead = readAndParseFifthRow(filename_char, stringArray, MAX_COLUMNS, MAX_STRING_LENGTH, row_idx, i_ctrl);
				

				// Print the results
				if (columnsRead > 0) {
					names_rows_added++;
					row_idx++;

					// Row 2: parameter values from CSV line 6
					if (row_idx < MAX_SNAPSHOTS) {
						int valuesRead = readAndParseValuesRow(filename_char, stringArray, MAX_COLUMNS, MAX_STRING_LENGTH, row_idx, i_ctrl);
						if (valuesRead > 0) {
							values_rows_added++;
							row_idx++;
						}
					}
				}
				else {
					failed2open++;
				}
			}
		}
	}
	
	create_csv_message();

	// Teensy-side checkpoint logs for bring-up.
	Serial.print("\n[ListCtrlParams] names rows added: ");
	Serial.print(names_rows_added);
	Serial.print(" values rows added: ");
	Serial.print(values_rows_added);
	Serial.print(" failed csv opens: ");
	Serial.print(failed2open);
	Serial.print(" total rows used: ");
	Serial.print(row_idx);
	Serial.print("/");
	Serial.print(MAX_SNAPSHOTS);
	Serial.print(" payload bytes: ");
	Serial.print(strlen(txBuffer_bulkStr));
	Serial.print("/");
	Serial.print(MAX_MESSAGE_SIZE);
	Serial.print(" end_marker_ok: ");
	size_t payload_len = strlen(txBuffer_bulkStr);
	bool has_end = (payload_len >= 3) &&
		(txBuffer_bulkStr[payload_len - 3] == ',') &&
		(txBuffer_bulkStr[payload_len - 2] == '?') &&
		(txBuffer_bulkStr[payload_len - 1] == '?');
	Serial.print(has_end ? "yes" : "no");
	Serial.print("\n");
}

// --- Function to Read and Parse ---

 int readAndParseFifthRow(
    const char* filename_char, 
    char arr[][MAX_COLUMNS][MAX_STRING_LENGTH], 
    int maxCols, 
    int maxLen, 
    uint8_t row_idx,
	int i_ctrl) 
{
    // Buffers to hold extracted components
    char joint_name[MAX_NAME_LENGTH];
    char controller_name[MAX_NAME_LENGTH];

    // Attempt to extract Joint and Controller names from the path
    bool extraction_success = retrieveJointAndController(
        filename_char, 
        joint_name, 
        controller_name
    );
    
    // Use default values if extraction fails
    //uint8_t joint_id_val = 255; // Default UNKNOWN ID
    const char* controller_str = "UNKNOWN_CTRL";

    if (extraction_success) {
        //joint_id_val = get_joint_id_from_name(joint_name);
        controller_str = controller_name;
    }
    
    // Check if there is enough space in the array for the prefix
    if (maxCols < PREFIX_COLS) {
        Serial.println("\nERROR: MAX_COLUMNS too small for prefix insertion.");
        return 0;
    }

    //Serial.print("\nOpening csv: ");
    //Serial.print(filename_char);
    
    // Using SD.h types (File)
    File dataFile = SD.open(filename_char);
    
    if (!dataFile) {
        Serial.print("\nError opening ");
        Serial.println(filename_char);
        // Assuming 'failed2open' is a global variable
        // failed2open++;
        return 0; // Return 0 columns read
    }

    // 1. Find the Fifth Line
    int rowCount = 0;
    String targetLine = "";
    
    // Read byte by byte until the end of the file or the fifth row is found
    while (dataFile.available()) {
        char c = dataFile.read();
        
        if (rowCount == 4) { // Row 5 is index 4 (0-based)
            targetLine += c;
        }
        
        if (c == '\n') {
            rowCount++;
            if (rowCount > 4) {
                break; // Stop after reading the entire fifth row
            }
        }
    }

    dataFile.close(); // Always close the file!

    if (rowCount < 4) {
        return 0;
    }
    
    // 2. Parse the Line (Tokenize the String) - STARTING AT INDEX PREFIX_COLS (3)
    int colIndex = PREFIX_COLS; // Start parsing data into index 3
    int charIndex = 0;
    int dataColsRead = 0; // Tracks columns successfully parsed from the file

    for (unsigned int i = 0; i < targetLine.length(); i++) {
        char c = targetLine.charAt(i);

        if (c == ',') {
            // End of a field: terminate the current string and move to the next column
            arr[row_idx][colIndex][charIndex] = '\0'; // Null-terminate the string
            
            dataColsRead++;
            colIndex++;
            charIndex = 0; // Reset character index for the next column
            
            // Safety check: stop if max columns reached (accounting for the 3 prefixes)
            if (colIndex >= maxCols) break;

        }
        else if (c != '\r' && c != '\n') { 
            // Append character to the current string
            if (charIndex < maxLen - 1) { 
                arr[row_idx][colIndex][charIndex] = c;
                charIndex++;
            }
        }
    }
    
    // 3. Handle the Last Parsed Column
    if (colIndex < maxCols && charIndex > 0) {
        arr[row_idx][colIndex][charIndex] = '\0';
        dataColsRead++;
        colIndex++;
    } 
    // Ensure the column immediately after the last written data is also null-terminated (cleared)
    else if (colIndex < maxCols) {
        arr[row_idx][colIndex][0] = '\0';
    }


    // --- 4. INSERT PREFIX COLUMNS (Columns 0, 1, and 2) ---
	
	// New Column 0: Insert char Joint name string
	strncpy(arr[row_idx][0], jointName, maxLen - 1);
	arr[row_idx][0][maxLen - 1] = '\0';
	
    // Column 0: Insert uint8_t Joint ID value
    // Use snprintf to convert the uint8_t (%u) into a string
    snprintf(arr[row_idx][1], maxLen, "%u", joint_id_val);

    // Column 1: Insert Controller Name string
    strncpy(arr[row_idx][2], controller_str, maxLen - 1);
    arr[row_idx][2][maxLen - 1] = '\0';

    // Column 2: Insert the full File Name
    //strncpy(arr[row_idx][2], filename_char, maxLen - 1);
	snprintf(arr[row_idx][3], maxLen, "%u", i_ctrl);
    //arr[row_idx][3][maxLen - 1] = '\0';


    // The total number of columns written is the prefix columns plus the data columns read
    int totalColsWritten = PREFIX_COLS + dataColsRead;
    
    //Serial.print("\nNumber of columns read: ");
    //Serial.print(dataColsRead);
    //Serial.print(" (Total stored: ");
    //Serial.print(totalColsWritten);
    //Serial.print(")");

    return totalColsWritten; 
}

void create_csv_message() {
    // 1. Initialize the buffer
    txBuffer_bulkStr[0] = '\0'; // Start with an empty string
	if (!append_to_tx_buffer("f,")) {
		return;
	}
	
    // 2. Iterate through all stored rows (snapshots)
    for (int i = 0; i < MAX_SNAPSHOTS; i++) {
        
        // Safety Check: Stop if the row is empty (based on our placeholder logic)
        if (stringArray[i][0][0] == '\0') {
            break; 
        }

        // 3. Iterate through all columns in the current row
        for (int j = 0; j < MAX_COLUMNS; j++) {
            
			if (stringArray[i][j][0] == '\0') {
				break; 
			}
			
            // a. Append the string from the cell
            // Note: This relies on stringArray[i][j] being null-terminated
            if (!append_to_tx_buffer(stringArray[i][j])) {
				Serial.println("[ListCtrlParams] ERROR: tx payload overflow while appending cell");
				return;
			}

            // b. Append the comma delimiter, except after the last column
            if (j < MAX_COLUMNS - 1) {
				if (stringArray[i][j+1][0] != '\0') {
					if (!append_to_tx_buffer(",")) {
						Serial.println("[ListCtrlParams] ERROR: tx payload overflow while appending comma");
						return;
					}
				}
            }
        }
        
        // 4. Append the End-of-Line symbol
        // Using '\n' (newline) is common; use "\r\n" for Windows/BLE compatibility if needed.
        if (!append_to_tx_buffer("\n")) {
			Serial.println("[ListCtrlParams] ERROR: tx payload overflow while appending newline");
			return;
		}
    }
	if (!append_to_tx_buffer(",??")) {
		Serial.println("[ListCtrlParams] ERROR: tx payload overflow while appending end marker");
	}
}

// Reads line 6 (first numeric data row) from the controller CSV and stores:
// [v, JointID, ControllerID, value1, value2, ...]
int readAndParseValuesRow(
   const char* filename_char,
   char arr[][MAX_COLUMNS][MAX_STRING_LENGTH],
   int maxCols,
   int maxLen,
   uint8_t row_idx,
   int i_ctrl)
{
	const int kValuePrefixCols = 3;
	if (maxCols < kValuePrefixCols) {
		return 0;
	}

	File dataFile = SD.open(filename_char);
	if (!dataFile) {
		return 0;
	}

	int rowCount = 0;
	String targetLine = "";
	while (dataFile.available()) {
		char c = dataFile.read();
		if (rowCount == 5) { // CSV line 6 (0-based index 5)
			targetLine += c;
		}
		if (c == '\n') {
			rowCount++;
			if (rowCount > 5) {
				break;
			}
		}
	}
	dataFile.close();

	if (rowCount < 5) {
		return 0;
	}

	int colIndex = kValuePrefixCols;
	int charIndex = 0;
	int dataColsRead = 0;

	for (unsigned int i = 0; i < targetLine.length(); i++) {
		char c = targetLine.charAt(i);
		if (c == ',') {
			arr[row_idx][colIndex][charIndex] = '\0';
			dataColsRead++;
			colIndex++;
			charIndex = 0;
			if (colIndex >= maxCols) break;
		}
		else if (c != '\r' && c != '\n') {
			if (charIndex < maxLen - 1) {
				arr[row_idx][colIndex][charIndex] = c;
				charIndex++;
			}
		}
	}

	if (colIndex < maxCols && charIndex > 0) {
		arr[row_idx][colIndex][charIndex] = '\0';
		dataColsRead++;
		colIndex++;
	}
	else if (colIndex < maxCols) {
		arr[row_idx][colIndex][0] = '\0';
	}

	if (dataColsRead <= 0) {
		return 0;
	}

	arr[row_idx][0][0] = 'v';
	arr[row_idx][0][1] = '\0';
	snprintf(arr[row_idx][1], maxLen, "%u", joint_id_val);
	snprintf(arr[row_idx][2], maxLen, "%u", i_ctrl);
	return kValuePrefixCols + dataColsRead;
}

/**
 * @brief Extracts the "Joint" and "Controller" names from a path string 
 * formatted as "\Joint\Controller.csv". (Logic based on extract_components.cpp)
 */
bool retrieveJointAndController(const char* filename_char, char* joint_out, char* controller_out) {
    if (!filename_char || !joint_out || !controller_out) return false;

    // 1. PREPARE START POINTER
    const char* start_ptr = filename_char;
    if (*start_ptr == '/') {
        start_ptr++; 
    }
    
    // 2. FIND JOINT END ('/')
    const char* joint_end_ptr = strchr(start_ptr, '/');
    if (!joint_end_ptr) return false;

    // 3. EXTRACT JOINT NAME
    size_t joint_len = joint_end_ptr - start_ptr;
    size_t copy_len = (joint_len < MAX_NAME_LENGTH) ? joint_len : MAX_NAME_LENGTH - 1;
    strncpy(joint_out, start_ptr, copy_len);
    joint_out[copy_len] = '\0'; 

    // 4. FIND CONTROLLER END ('.csv')
    const char* controller_start_ptr = joint_end_ptr + 1;
    const char* controller_end_ptr = strstr(controller_start_ptr, ".csv");
    if (!controller_end_ptr) return false;
    
    // 5. EXTRACT CONTROLLER NAME
    size_t controller_len = controller_end_ptr - controller_start_ptr;
    copy_len = (controller_len < MAX_NAME_LENGTH) ? controller_len : MAX_NAME_LENGTH - 1;
    strncpy(controller_out, controller_start_ptr, copy_len);
    controller_out[copy_len] = '\0'; 

    return true;
}

#endif
