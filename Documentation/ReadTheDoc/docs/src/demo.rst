Demo
====

This is used to demonstrate various features, and also for testing.

-------------------------------------------------------------------------------

Autodoc
-------

.. currentmodule:: piccolo_theme.snippets

.. autoclass:: Column

-------------------------------------------------------------------------------

Breathe
-------

.. only:: include_breathe

  .. doxygenclass:: CppClass
      :members:

  .. doxygenfunction:: cpp_function

  .. doxygenfunction:: c_function
      :project: c_demo

.. only:: exclude_breathe

  We can't demo Breathe on this version of Sphinx.

-------------------------------------------------------------------------------

Code Blocks
-----------

Basic
~~~~~

.. code-block:: python

  def say_hello():
      print("hello world!")

Emphasize
~~~~~~~~~

.. code-block:: python
  :emphasize-lines: 1,2

  def say_hello():
      print("hello world!")

Line numbers
~~~~~~~~~~~~

.. code-block:: python
  :linenos:

  def say_hello():
      print("hello world!")

Caption
~~~~~~~

.. code-block:: python
  :caption: Some example code

  def say_hello():
      print("hello world!")

Syntax highlighting
~~~~~~~~~~~~~~~~~~~

An example of how syntax highlighting looks for various bits of code:

.. code-block:: python

  a = 1
  print(a)


  def hello_world():
      print("hello world!")


  class Greeting():
    def __init__(self, message: str):
        self.message = message

    def greeting(self):
        print(self.message)


  if __name__ == '__main__':
      greeting = Greeting(message="hello world")
      greeting.greeting()

-------------------------------------------------------------------------------

Tables
------

Table 1
~~~~~~~

====== =======
Name   Drives
====== =======
Alice  True
Bob    True
Curtis False
====== =======

Table 2
~~~~~~~

+------------+------------+-----------+
| Header 1   | Header 2   | Header 3  |
+============+============+===========+
| body row 1 | column 2   | column 3  |
+------------+------------+-----------+
| body row 2 | Cells may span columns.|
|            |                        |
|            | And several paragraphs.|
+------------+------------+-----------+
| body row 3 | Cells may  | - Cells   |
+------------+ span rows. | - contain |
| body row 4 |            | - blocks. |
+------------+------------+-----------+

-------------------------------------------------------------------------------

Data definitions
----------------

Python
  A great programming language.

Sphinx
  A powerful documentation tool.

-------------------------------------------------------------------------------

Lists
-----

Unordered List
~~~~~~~~~~~~~~

* Python
* Rust
* JavaScript

Ordered List
~~~~~~~~~~~~

Explicit numbers:

1. Python
2. Rust
3. JavaScript

Auto numbers:

#. Python
#. Rust
#. JavaScript

Lower-alpha:

a. Rust
#. C++
#. C

Upper-alpha:

A. reStructuredText
#. HTML
#. Markdown

Lower-roman:

i. reStructuredText
#. HTML
#. Markdown

Upper-roman:

I. reStructuredText
#. HTML
#. Markdown

Nested List
~~~~~~~~~~~

1. Languages

   a. Python
   #. Rust
   #. JavaScript

-------------------------------------------------------------------------------

Admonitions
-----------

.. warning:: This is a warning!

.. error:: This is an error!

.. hint:: This is a hint!

.. note:: This is a note!

.. admonition:: A custom admonition

   This is my custom admonition!
