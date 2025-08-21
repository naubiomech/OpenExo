Adding a New BLE Message
========================

ble_commands.h
--------------
1. In the ``ble_names`` namespace, assign a **name** to the character you want to use.
2. In the ``commands`` array within the ``ble`` namespace, add your new message name and specify the number of variables the system should expect to receive or transmit.
3. **If your message is sent by the Exo**, you're done.
4. **If your message is received by the Exo**, navigate to the ``ble_handlers`` namespace and implement the function that should run when your message is received.

   The function must be declared like so:

   .. code-block:: c++

      inline static void my_new_function(ExoData* data, BleMessage* msg)

ComsMCU.cpp
-----------
1. **If your message is received by the Exo**, locate the switch statement in the function:

   .. code-block:: c++

      ComsMCU::_process_complete_gui_command(BleMessage* msg)

2. Add a new ``case`` for your message, using the variable you created in the ``ble_names`` namespace.  
   The case should call the corresponding function in the ``ble_handlers`` namespace.

Done
----
- **If your command is sent by the Exo**, the parser recognizes and packages your data automatically.
- **If your command is received**, the parser packages your data and calls the function defined in the ``ble_handlers`` namespace.
- **Note**: You may have to update the GUI to interface with the newly added BLEMessage, refer to GUI documentation if needed.
