OpenExo C++ Style Guide
=======================

Based on https://github.com/isocpp/CppCoreGuidelines and https://www.python.org/dev/peps/pep-0008/

General
-------

- Avoid extra whitespace
- 4 space tab
- DON’T place two statements on the same line
- DON’T use void as an argument type
- If globals exist still pass them by reference
- Place curly bracket { on the next line when opening a multiline section.
- Always run interactions through items from proximal joint to distal, and left before right.

Commenting
----------

- Use docstring (/\* \*/) for namespaces, types, methods, functions, and constants use explination before definition.
- State intent in comments, not just a restatement of what is readible in the code.
- Comment code as you go so you know what you were trying to do.

Naming
------

- Wrap globals in a namespace, e.g. board::can_rx. Pick a logical namespace name, can be after the header file it is found, but you may want multiple namespaces within one header.
- ALL_CAPS for macro names only, this avoid thinking something is a macro when it is subject to type rules.

Functions
~~~~~~~~~

- Function names should be lowercase, with words separated by underscores as necessary to improve readability.

Classes
~~~~~~~

- Class names should normally use the CapWords convention.
- Methods should use function naming convention.
- Private/protected members should use a leading underscores
- Abstract classes should use a leading underscore, ex. \_AbstractClass
- There should not be any public member variables, members should be accessed with get and set functions.

Types
~~~~~

- Type variable names should use CapWord Convention

Variables
~~~~~~~~~

- Never use the characters ‘l’ (lowercase letter el), ‘O’ (uppercase letter oh), or ‘I’ (uppercase letter eye) as single character variable names.
- Variable names should follow function naming convention
- DON’T include type in the name, e.g. int_use_this

Enum
~~~~

- use enum class rather than typedef.

Namespace
~~~~~~~~~

- Namespaces should normally use the CapWords convention.

Misc
~~~~

- Angles should be in radians if not specified.
