Adding New UART Messages
=========================

Messaging Structure
-------------------
Each message consists of a **get** and an **update** ID:
 
- **Get ID**: Used to request data from the other microcontroller.
- **Update ID**: Used to set data received from the other microcontroller.

Each message must have both a get and an update ID; however, every message ID must be unique.  
Every message ID also has an associated message handler—this is the code that executes on the microcontroller when a message is received from the other microcontroller.

Declare Message ID(s)
---------------------
1. Navigate to the ``src`` folder and open the file ``uart_commands.h``.
2. Locate the namespace ``UART_command_names``.
3. Declare your message ID(s) within this namespace.

Declare Message Handler
-----------------------
1. In the same file (``uart_commands.h``), locate the namespace ``UART_command_handlers``.
2. Create your message handler function with the following signature:

   .. code-block:: c++

      static inline void your_function_name(UARTHandler* handler, ExoData* exo_data, UART_msg_t msg)

   Replace ``your_function_name`` with the appropriate function name for your new message.

Add Your Function to the List of Callable Functions
-----------------------------------------------------
1. Again in ``uart_commands.h``, locate the namespace ``UART_command_utils``.
2. Within that namespace, find the function named ``handle_msg``.
3. Add a switch case (or cases) for your new message ID(s) in the ``handle_msg`` function, so that your handler is called when the corresponding message is received.

Done
----
You have now integrated your new UART message!  
Test your new function to ensure it handles messages as expected.
