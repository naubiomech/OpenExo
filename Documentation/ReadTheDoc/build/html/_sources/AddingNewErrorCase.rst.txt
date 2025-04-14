Adding a New Error Case
=======================

Error cases should be specific and are structured to operate on a single joint's data.

error_codes.h
-------------
1. In the ``ErrorCodes`` enum, create a name for your new error case.  
   - This name should be placed below the ``NO_ERROR`` case and above the ``ERROR_CODE_LENGTH`` case.

error_types.h
-------------
1. Create a new error type class (use the other error types as a reference).
2. The class should inherit from ``ErrorType`` and must implement both a ``check`` and a ``handle`` function.
3. The ``check`` function should:
   - Return ``True`` if your error has occurred.
   - Return ``False`` if it has not.
   - .. note::  
         The ``check`` function only has access to the ``JointData`` class and is executed for every joint.  
         If your error case requires its own data, add that data to the ``JointData`` class (in **JointData.h**).
4. The ``handle`` function decides what action to take when your error occurs, such as disabling the motors.

error_map.h
-----------
1. In ``error_map.h``, add a new key-value pair for your new error case.  
   - Use the following format:

   .. code-block:: c++

      {YOUR_ERROR_CASE_NAME_FROM_PART_ONE, new YourErrorTypeNameFromPartTwo()},

Done
----
Your new error case will now be:
- Checked by the ``run`` method in **Joint.cpp** (applied to the hip, knee, and ankle).
- Handled by your ``handle`` function when it occurs.
- Reported to the app appropriately.
