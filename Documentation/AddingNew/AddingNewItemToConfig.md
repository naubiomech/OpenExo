# Adding New Item to Config
## config.ini
### Overview
- The ini file is broken into Sections, which are large categories that store data
    - examples, board, battery, or types of exoskeletons.
- Within sections there are Keys, these are where the actual data ends up getting stored.
    - examples board name, and board num; motors that are used;
- For our specific config file the Exo section, key name, stores the name of the type of exoskeleton that the system will use.
    - This allows us to store common configurations, that way if we need to change the system all we have to do from the code side is change the name here on the SD card and the system will configure itself correctly.
- ; will start a line comment, e.g. ; this is a comment
- If you are just adding new values to existing keys you should also follow this guide to make sure you updated all the required locations.

### Adding new Section
- To add a new section simply put the name in square brackets, e.g. [bluetooth]
- You will then need to populate it with keys.
- Section name should be less than 10 char

### Adding New Keys
- Under the section you will need to add the keys you would like to use with the name = value, e.g. name = smart
- for the key you should also add a comment describing the purpose and the possible values, e.g. ; describes the type of battery used, values: smart(inspired energy), dumb (simple lipo)
- If you are adding something to an exoskeleton (e.g.[bilateralHipAnkle]) Section you need to add the key to all existing exoskeleton sections. As next we will be editing the parser which will look for all the keys
- Key length should be less than 25 char

## ParseIni.h
- namespace ini_config
    - update number_of_keys with the number of keys that will be parsed, e.g. if you added 3 keys to each exoskeleton, you would just add 3 to the existing number.
- namespace config_defs
    - Here you will create an enum class to encode the key into a uint8_t.
        - Your key will need to be encoded to a uint8_t when passed between other parts of the code. 
        - Create a class with all valid key values following the existing enum classes as a template
    - At the bottom of the list of indexes where the encoded values will be placed in the configuration array.
        - For the keys that have been added create new index values.  Values should be unique an consecutive.
        - As a check go back up to namespace ini_config, and confirm that the number_of_keys matches the number of indexes (should be one more than the largest index, since we started at zero)
- namespace config_map        
    - The config file is interpreted as text.  This creates a map to convert the text to the uint8_t declared above.
    - We have created an IniKeyCode type that is "indexed" with a std:string, e.g. config_map::hip_controllers["extensionAngle"] returns config_defs::hip_controllers::extension_angle
    - Create a new IniKeyCode for your new key (follow the others as a template), the string portion should be all valid values from the ini file, and the uint8_t should be the values defined in the config_defs namespace.
- struct ConfigData
    - This structure will store the string values during the parsing.
    - create std::string(s) for the new keys
    
## ParseIni.cpp
- ini_parser(char* filename, uint8_t* config_to_send)
    - You can use the existing keys as a template.
    - For each new key there are 3 main steps
        1. Read the key using get_section_key(filename,section_name, key_name, buffer, buffer_len)
            - This will put the value of the key in the buffer.
            - If the key is within a type of exoskeleton, then temp_exo_name will be the section name as this will look into the appropriate exoskeleton type stored in the ini file.
            - update the section and key names in step 1 from the ini file
        2. Store the value from the buffer
            - This uses the field in the structure we just created.
            - data.<field> name to match the ConfigData fields from the ParseIni.h file
        3. Encode the value to a uint8_t, and place it in the appropriate place in the configuration array.
            - config_to_send[config_defs::<key_idx>] = config_map::<key_name>[data.<field>];
            - <key_idx> is the index from the config_defs in ParseIni.h
            - <key_name> is the IniKeyCode name from config_map in ParseIni.h
            - <field> is what was just updated in step 2.

## uart_commands.h
- get_config
	- Use the existing keys as a template. 
- update_config
	- Use the existing keys as a template.
            
## Wrap up
- That should be everything you need to do to parse the ini file.
- You will still need to go to the rest of the code and update where the new values are used but that will depend on what you have added so I can't help you there.
            
            
    