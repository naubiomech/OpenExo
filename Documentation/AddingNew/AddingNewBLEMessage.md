# Adding a new BLE Message
## ble_commands.h
1. In the 'ble_names' namespace, give the character you would like to use a name. 
2. In the 'commands' array within the 'ble' namespace, input your new message name and the number of variables that the system should expect to receive or transmit when it gets your message.
3. If your message is being sent by the Exo, you're done.
4. If your message is being received by the Exo, go to the 'ble_handlers' namespace and write the function that you would to run when your message is received. 
The function must be declared like this: 'inline static void my_new_function(ExoData* data, BleMessage* msg)'

## ComsMCU.cpp
1. If your message is being received by the Exo, locate the switch statement in ComsMCU::_process_complete_gui_command(BleMessage* msg). 
2. Add a case for your message using the variable you made in the 'ble_names' namespace. The case should only call the function that your made in the 'ble_handlers'
namespace. 

## Done 
If your command is being sent by the Exo, the parser will now recognize and package your data. If your command is being received, the parser will package your
data and call the function that you made in the 'ble_handlers' namespace. If should be noted that you may have to update the GUI to interface with the newly added BLEMessage, refer to GUI documentation if needed.