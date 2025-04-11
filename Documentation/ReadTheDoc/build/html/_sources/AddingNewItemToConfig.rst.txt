Adding New Item to Config
=========================

config.ini
----------
### Overview
- The ini file is broken into **Sections**, which are large categories that store data such as:
  - board, battery, or types of exoskeletons.
- Within sections there are **Keys**, which are where the actual data gets stored.
  - Examples include board name, board number, and motors that are used.
- For our specific config file, the **Exo** section’s key named *name* stores the type of exoskeleton that the system will use.
  - This enables storing common configurations so that if we need to change the system, all that needs to be done from the code side is to change the name on the SD card and the system will configure itself correctly.
- A semicolon (`;`) starts a line comment (e.g., ``; this is a comment``).
- If you are just adding new values to existing keys, ensure you update all the required locations.

### Adding New Section
- To add a new section, simply enclose the section name in square brackets, for example, ``[bluetooth]``.
- You will then need to populate it with keys.
- **Note:** Section names should be less than 10 characters.

### Adding New Keys
- Under the section, add the keys in the format ``name = value`` (e.g., ``name = smart``).
- For each key, add a comment describing its purpose and possible values, e.g.,  
  ``; describes the type of battery used, values: smart (inspired energy), dumb (simple lipo)``.
- If you are adding a key to an exoskeleton section (e.g., ``[bilateralHipAnkle]``), be sure to add the key to all existing exoskeleton sections.
- **Note:** Key lengths should be less than 25 characters.

ParseIni.h
----------
- **namespace ini_config**  
  - Update ``number_of_keys`` with the total number of keys that will be parsed.  
    *Example:* If you added 3 keys to each exoskeleton, add 3 to the existing number.
- **namespace config_defs**  
  - Create an enum class to encode the key into a ``uint8_t``.
    - Your key must be encoded to a ``uint8_t`` when passed between parts of the code.
    - Create a class with all valid key values, following the existing enum classes as a template.
  - At the bottom of the list, add new index values for the keys you have added.
    - Values should be unique and consecutive.
    - As a check, confirm that the value of ``number_of_keys`` in ``namespace ini_config`` matches the number of indexes (which should be one more than the largest index, since we started at zero).
- **namespace config_map**  
  - The config file will be interpreted as text. Create a map to convert the text into the ``uint8_t`` declared above.
  - An ``IniKeyCode`` type is provided that is indexed with a ``std::string``.  
    *Example:*  
    ``config_map::hip_controllers["extensionAngle"]`` returns ``config_defs::hip_controllers::extension_angle``.
  - Create a new ``IniKeyCode`` for your new key (follow the others as a template).  
    - The string portion should include all valid values from the ini file, and the ``uint8_t`` should be defined in the ``config_defs`` namespace.
- **struct ConfigData**  
  - This structure stores the string values during parsing.
  - Create additional ``std::string`` fields for the new keys.

ParseIni.cpp
------------
- In the function ``ini_parser(char* filename, uint8_t* config_to_send)``, you can use the existing keys as a template.
- For each new key, there are three main steps:
  1. **Read the key:**  
     Use  
     ``get_section_key(filename, section_name, key_name, buffer, buffer_len)``
     - This function places the value of the key into the buffer.
     - If the key belongs to an exoskeleton type, then ``temp_exo_name`` will hold the section name (which corresponds to the appropriate exoskeleton type stored in the ini file).
     - Update the section and key names in this step according to the ini file.
  2. **Store the value from the buffer:**  
     - Use the corresponding field in the structure we created (**ConfigData**).
     - The field name (``data.<field>``) should match the one defined in ``ParseIni.h``.
  3. **Encode the value:**  
     - Convert the value to a ``uint8_t`` and place it into the configuration array using:  
       
       .. code-block:: c

          config_to_send[config_defs::<key_idx>] = config_map::<key_name>[data.<field>];
       
       - Here:
         - ``<key_idx>`` is the index defined in ``config_defs`` (in ``ParseIni.h``).
         - ``<key_name>`` is the IniKeyCode name from ``config_map`` (in ``ParseIni.h``).
         - ``<field>`` is the field that was updated in step 2.

uart_commands.h
---------------
- **get_config**:  
  - Use the existing keys as a template.
- **update_config**:  
  - Use the existing keys as a template.

Wrap up
-------
- That should be everything you need to do to parse the ini file.
- You will still need to update other parts of the code to use the new values, which depends on what you have added.
