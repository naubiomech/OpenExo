Adding a New Motor Type
=======================

Motor.h
-------
1. Create a class for your new motor type that inherits its interface from the ``_Motor`` class:

   .. code-block:: c++

      class _YourMotorType : public _Motor
      {
          public:
              _YourMotorType(config_defs::joint_id id, ExoData* exo_data, int enable_pin);
              virtual ~_YourMotorType(){};
              void transaction(float torque);
              void read_data();
              void send_data(float torque);
              void on_off();
              bool enable();
              bool enable(bool overide);
              void zero();
          protected:
              ...
      };

2. Add any protected member variables or functions that will be needed to operate your motor.

3. Create a specific motor class of this type that inherits the motor type class you just created:

   .. code-block:: c++

      class ANewMotor : public _YourMotorType
      {
          public:
              ANewMotor(config_defs::joint_id id, ExoData* exo_data, int enable_pin); // constructor: type is the motor type
              ~ANewMotor(){};
      };

4. Add any additional protected member variables or functions needed to operate your motor.

Motor.cpp
---------
1. Define the behavior of the member functions of ``_YourMotorType``.
   - Ideally, these behaviors will be shared by all motors of this type.
2. Define the behavior of the member functions and any class-specific variables for ``ANewMotor``.

Connect to Everything Else
---------------------------
1. Follow the steps in :doc:`AddingNewCanMotor <AddingNewCanMotor.md>` to connect this new motor type to the rest of the codebase.
   - This step is detailed in the referenced document so that you only need to update information in one place.
